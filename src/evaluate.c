#include <math.h>
#include <stdlib.h>
#include <string.h>

#define EVA_CV_P2                 0b00001111 // 변동계수 (Coefficient of Variation, CV)
#define EVA_DEVIATION_MAXIMUM_P2  0b00010001 // 최대차이 (Maximum Deviation)
#define EVA_DEVIATION_STANDARD_P2 0b00010101 // 표준편차 (Standard Deviation)
#define EVA_ENTROPY_BINARY_P1     0b00000001 // 이진 엔트로피 (Binary Entropy)
#define EVA_EXPECTED_P1           0b00000010 // 기대값 (Expected Value)
#define EVA_IQR_P1                0b00000101 // 사분위범위 (Interquartile Range, IQR)
#define EVA_KURTOSIS_P3           0b00011001 // 첨도 (Kurtosis)
#define EVA_MAD_P2                0b00010000 // 평균 절대 편차 (Mean Absolute Deviation, MAD)
#define EVA_MAE_P1                0b00000110 // 평균 절대 편차 (Mean Absolute Error, MAE)
#define EVA_MEAD_P1               0b00000111 // 절대 편차 중앙값 (Median Absolute Deviation, MeAD)
#define EVA_MEAN_GEOMETRIC_P1     0b00000011 // 기하평균 (Geometric Mean)
#define EVA_MEAN_HARMONIC_P1      0b00000100 // 조화평균 (Harmonic Mean)
#define EVA_MEAN_P1               0b00001000 // 평균값 (Mean)
#define EVA_MEAN_TRIMMED_P2       0b00010110 // 절사평균 (Trimmed Mean)
#define EVA_MEDIAN_P1             0b00001001 // 중위값 (Median)
#define EVA_MODE_P1               0b00001010 // 최빈값 (Mode)
#define EVA_MSE_P2                0b00010010 // 평균 제곱 편차 (Mean Squared Error, MSE)
#define EVA_PERCENTILE_P2         0b00010100 // 백분위수 (Percentiles)
#define EVA_QUARTILES_P4R3        0b00011010 // 사분위수 (Quartiles)
#define EVA_RANGE_P1              0b00001100 // 범위 (Range)
#define EVA_RANGE_PERCENT_P1      0b00001011 // 퍼센트 범위 (PercentRange)
#define EVA_RMS_P1                0b00001101 // 제곱평균 (Root Mean Square, RMS)
#define EVA_RMSLE_P1              0b00001110 // 제곱평균 로그 (RMSLE)
#define EVA_SKEWNESS_M_P2         0b00010011 // 중위값 기준 왜도 (Median Skewness)
#define EVA_SKEWNESS_Z_P2         0b00011000 // 왜도(Skewness)
#define EVA_VARIANCE_P2           0b00010111 // 분산 (Variance)

static inline void eva_swap ( double *a, double *b )
{
  double temp = *a;

  *a = *b;
  *b = temp;
}

static void eva_sort ( double *data, int left, int right )
{
  if ( left < right )
  {
    double pv = data[right];
    int    i  = left - 1;

    for ( int j = left; j < right; j++ )
    {
      if ( data[j] < pv )
      {
        i++;
        eva_swap ( &data[i], &data[j] );
      }
    }

    eva_swap ( &data[i + 1], &data[right] );

    int idx = i + 1;

    quicksort ( data, left, idx - 1 );
    quicksort ( data, idx + 1, right );
  }
}

double eva_p1 ( int operation, size_t len, const double *data )
{
  double result = 0.0;

  switch ( operation )
  {
    case EVA_MEAN_P1:
    case EVA_EXPECTED_P1:
    {
      for ( size_t i = 0; i < len; i++ )
      {
        result += data[i];
      }

      result /= len;
    }
    break;

    case EVA_MEAN_GEOMETRIC_P1:
    {
      double product     = 1.0;
      size_t valid_count = 0;

      for ( size_t i = 0; i < len; i++ )
      {
        if ( data[i] > 0 )
        {
          product *= data[i];
          valid_count++;
        }
      }

      result = valid_count > 0 ? pow ( product, 1.0 / valid_count ) : 0.0;
    }
    break;

    case EVA_MEAN_HARMONIC_P1:
    {
      double sum_reciprocal = 0.0;
      size_t valid_count    = 0;

      for ( size_t i = 0; i < len; i++ )
      {
        if ( data[i] != 0 )
        {
          sum_reciprocal += 1.0 / data[i];
          valid_count++;
        }
      }

      result = valid_count > 0 ? valid_count / sum_reciprocal : 0.0;
    }
    break;

    case EVA_MEDIAN_P1:
    {
      double *sorted_data = malloc ( len * sizeof ( double ) );

      memcpy ( sorted_data, data, len * sizeof ( double ) );
      quicksort ( sorted_data, 0, len - 1 );

      if ( len % 2 == 0 )
      {
        result = ( sorted_data[len / 2 - 1] + sorted_data[len / 2] ) / 2.0;
      }
      else
      {
        result = sorted_data[len / 2];
      }

      free ( sorted_data );
    }
    break;

    case EVA_MODE_P1:
    {
      double mode       = data[0];
      int    mode_count = 1, max_count = 1;

      for ( size_t i = 1; i < len; i++ )
      {
        if ( data[i] == data[i - 1] )
        {
          mode_count++;
        }
        else
        {
          if ( mode_count > max_count )
          {
            max_count = mode_count;
            mode      = data[i - 1];
          }

          mode_count = 1;
        }
      }
      result = mode;
    }
    break;

    case EVA_RANGE_P1:
    {
      double min_val = data[0];
      double max_val = data[0];

      for ( size_t i = 1; i < len; i++ )
      {
        if ( data[i] < min_val )
        {
          min_val = data[i];
        }

        if ( data[i] > max_val )
        {
          max_val = data[i];
        }
      }

      result = max_val - min_val;
    }
    break;

    case EVA_IQR_P1:
    {
      double q1 = eva_p2 ( EVA_PERCENTILE_P2, len, data, 25.0 );
      double q3 = eva_p2 ( EVA_PERCENTILE_P2, len, data, 75.0 );

      result = q3 - q1;
    }
    break;

    case EVA_RMS_P1:
    {
      double sum_of_squares = 0.0;

      for ( size_t i = 0; i < len; i++ )
      {
        sum_of_squares += data[i] * data[i];
      }

      result = sqrt ( sum_of_squares / len );
    }
    break;

    case EVA_RMSLE_P1:
    {
      double sum_of_squares = 0.0;

      for ( size_t i = 0; i < len; i++ )
      {
        sum_of_squares += pow ( log ( data[i] + 1 ), 2 );
      }

      result = sqrt ( sum_of_squares / len );
    }
    break;

    case EVA_MAE_P1:
    {
      double sum_of_absolute_errors = 0.0;

      for ( size_t i = 0; i < len; i++ )
      {
        sum_of_absolute_errors += fabs ( data[i] );
      }

      result = sum_of_absolute_errors / len;
    }
    break;

    case EVA_MEAD_P1:
    {
      double  median             = eva_p1 ( EVA_MEDIAN_P1, len, data );
      double *absolute_deviation = malloc ( len * sizeof ( double ) );

      for ( size_t i = 0; i < len; i++ )
      {
        absolute_deviation[i] = fabs ( data[i] - median );
      }

      result = eva_p1 ( EVA_MEDIAN_P1, len, absolute_deviation );
      free ( absolute_deviation );
    }
    break;

    case EVA_RANGE_PERCENT_P1:
    {
      double min_val = data[0];
      double max_val = data[0];

      for ( size_t i = 1; i < len; i++ )
      {
        if ( data[i] < min_val )
        {
          min_val = data[i];
        }

        if ( data[i] > max_val )
        {
          max_val = data[i];
        }
      }

      result = ( ( max_val - min_val ) / max_val ) * 100.0;
    }
    break;

    case EVA_ENTROPY_BINARY_P1:
    {
      double entropy = 0.0;

      for ( size_t i = 0; i < len; i++ )
      {
        if ( data[i] > 0 )
        {
          entropy -= data[i] * log2 ( data[i] );
        }
      }

      result = entropy / len;
    }
    break;
  }

  return result;
}

double eva_p2 ( int operation, size_t len, const double *data, const double p1 )
{
  double result = 0.0;

  switch ( operation )
  {
    case EVA_VARIANCE_P2:
    case EVA_MSE_P2:
    {
      for ( size_t i = 0; i < len; i++ )
      {
        result += ( data[i] - p1 ) * ( data[i] - p1 );
      }

      result /= len;
    }
    break;

    case EVA_DEVIATION_STANDARD_P2:
    {
      result = sqrt ( eva_p2 ( EVA_VARIANCE_P2, len, data, p1 ) );
    }
    break;

    case EVA_MAD_P2:
    {
      double *absolute_deviation = malloc ( len * sizeof ( double ) );
      for ( size_t i = 0; i < len; i++ )
      {
        absolute_deviation[i] = fabs ( data[i] - p1 );
      }
      result = eva_p1 ( EVA_MEDIAN_P1, len, absolute_deviation );
      free ( absolute_deviation );
    }
    break;

    case EVA_PERCENTILE_P2:
    {
      double k = ( p1 / 100.0 ) * ( len - 1 );
      size_t f = ( size_t )floor ( k ), c = ( size_t )ceil ( k );
      result = data[f] + ( k - f ) * ( data[c] - data[f] );
    }
    break;

    case EVA_DEVIATION_MAXIMUM_P2:
    {
      for ( size_t i = 0; i < len; i++ )
      {
        double deviation = fabs ( data[i] - p1 );
        if ( deviation > result )
        {
          result = deviation;
        }
      }
    }
    break;

    case EVA_MEAN_TRIMMED_P2:
    {
      size_t trim_amount = ( size_t )( len * p1 );

      result = eva_p1 ( EVA_MEAN_P1, len - 2 * trim_amount, data + trim_amount );
    }
    break;

    case EVA_SKEWNESS_M_P2:
    case EVA_SKEWNESS_Z_P2:
    {
      double sum_cubed_deviation = 0.0;
      double stddev              = eva_p2 ( EVA_DEVIATION_STANDARD_P2, len, data, p1 );

      for ( size_t i = 0; i < len; i++ )
      {
        sum_cubed_deviation += pow ( data[i] - p1, 3 );
      }

      result = ( sum_cubed_deviation / len ) / pow ( stddev, 3 );
    }
    break;

    case EVA_CV_P2:
    {
      result = p1 / eva_p2 ( EVA_DEVIATION_STANDARD_P2, len, data, p1 );
    }
    break;
  }

  return result;
}

double eva_p3 ( int operation, size_t len, const double *data, const double p1, const double p2 )
{
  double result = 0.0;

  switch ( operation )
  {
    case EVA_KURTOSIS_P3:
    {
      double sum = 0.0;

      for ( size_t i = 0; i < len; i++ )
      {
        sum += pow ( data[i] - p1, 4 );
      }

      result = ( sum / len ) / pow ( p2, 4 ) - 3.0;
    }
    break;
  }

  return result;
}

void eva_p4r3 ( int operation, size_t len, const double *data, double *p1, double *p2, double *p3 )
{
  switch ( operation )
  {
    case EVA_QUARTILES_P4R3:
    {
      double *sorted_data = malloc ( len * sizeof ( double ) );

      memcpy ( sorted_data, data, len * sizeof ( double ) );
      quicksort ( sorted_data, 0, len - 1 );

      *p2 = eva_p1 ( EVA_MEDIAN_P1, len, sorted_data );

      if ( len % 2 == 0 )
      {
        *p1 = eva_p1 ( EVA_MEDIAN_P1, len / 2, sorted_data );
        *p3 = eva_p1 ( EVA_MEDIAN_P1, len / 2, sorted_data + len / 2 );
      }
      else
      {
        *p1 = eva_p1 ( EVA_MEDIAN_P1, len / 2, sorted_data );
        *p3 = eva_p1 ( EVA_MEDIAN_P1, len / 2, sorted_data + len / 2 + 1 );
      }

      free ( sorted_data );
    }
    break;
  }
}
