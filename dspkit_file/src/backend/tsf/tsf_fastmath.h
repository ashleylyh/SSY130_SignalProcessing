/** @file Contains fast approximate versions of math functions used by TSF */
#ifndef TSF_FASTMATH_H_
#define TSF_FASTMATH_H_

#define TSF_POWF    TSF_POW
#define TSF_POW     tsf_fastmath_pow
#define TSF_EXPF    tsf_fastmath_exp
#define TSF_LOG     tsf_fastmath_log
#define TSF_TAN     tsf_fastmath_tan
#define TSF_LOG10   tsf_fastmath_log10
#define TSF_SQRTF   TSF_SQRT
#define TSF_SQRT    tsf_fastmath_sqrt

/** @brief Precision of power/exponential functions
 * See powfast-c for details */
#define TST_FASTMATH_POW_PREC   8

/** @brief Initialize fastmath subsystem */
void tsf_fastmath_init(void);

/** @brief Returns fast approximation to a^b
 * See powfast-c for details */
float tsf_fastmath_pow(float a, float b);

/** @brief Returns fast approximation to 2^b
 * See powfast-c for details */
float tsf_fastmath_pow2(float x);

/** @brief Returns fast approximation to exp(b)
 * See powfast-c for details */
float tsf_fastmath_exp(float x);

/** @brief Returns fast approximation to 10^b
 * See powfast-c for details */
float tsf_fastmath_pow10(float x);

/** @brief Fast approximation to natural logarithm
 * See https://community.arm.com/tools/f/discussions/4292/cmsis-dsp-new-functionality-proposal/22621 */
float tsf_fastmath_log(float x);

/** @brief Fast approximation to base 10 logarithm
 * See https://community.arm.com/tools/f/discussions/4292/cmsis-dsp-new-functionality-proposal/22621 */
float tsf_fastmath_log10(float x);

/** @brief Fast approximation to sqaure root function
 * See http://bits.stephan-brumme.com/squareRoot.html */
float tsf_fastmath_sqrt(float x);

/** @brief Fast approximation to tan function
 * Trivial implementation using arm_sin/cos_f32 */
float tsf_fastmath_tan(float x);

#endif /* TSF_FASTMATH_H_ */