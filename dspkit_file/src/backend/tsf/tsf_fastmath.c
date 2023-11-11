#include <math.h>
#include "tsf_fastmath.h"
#include "powfast-c/PowFast.h"
#include "macro.h"
#include "arm_math.h"

void* powFastAdj;

// This is a fast approximation to log2()
// Y = C[0]*F*F*F + C[1]*F*F + C[2]*F + C[3] + E;
static float log2f_approx(float X) {
  float Y, F;
  int E;
  F = frexpf(ABS(X), &E);
  Y = 1.23149591368684f;
  Y *= F;
  Y += -4.11852516267426f;
  Y *= F;
  Y += 6.02197014179219f;
  Y *= F;
  Y += -3.13396450166353f;
  Y += E;
  return(Y);
}


void tsf_fastmath_init(void){
    powFastAdj = powFastCreate(TST_FASTMATH_POW_PREC);
}

float tsf_fastmath_pow(float a, float b){
    return powFast(powFastAdj, tsf_fastmath_log(a), b);
}

float tsf_fastmath_pow2(float x){
    return powFast2(powFastAdj, x);
}

float tsf_fastmath_pow10(float x){
    return powFast10(powFastAdj, x);
}

float tsf_fastmath_exp(float x){
    return powFastE(powFastAdj, x);
}

float tsf_fastmath_log(float x){
    return log2f_approx(x) * 0.693147180559945;
}

float tsf_fastmath_log10(float x){
    return log2f_approx(x) * 0.301029995663981f;
}

float tsf_fastmath_sqrt(float x){
    int32_t i = *(int32_t*) &x;
    // adjust bias
    i  += 127 << 23;
    // approximation of square root
    i >>= 1;
    return *(float*) &i;
}

float tsf_fastmath_tan(float x){
    return arm_sin_f32(x)/arm_cos_f32(x);
}