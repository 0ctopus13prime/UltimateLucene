#include <Util/Numeric.h>

using lucene::core::util::numeric::FloatConsts;
using lucene::core::util::numeric::DoubleConsts;

const float FloatConsts::POSITIVE_INFINITY = 1.0F / 0.0;
const float FloatConsts::NEGATIVE_INFINITY = -1.0F / 0.0;
const float FloatConsts::NaN = 0.0F / 0.0;
const float FloatConsts::MAX_VALUE = 3.4028235E38F;
const float FloatConsts::MIN_VALUE = 1.4E-45F;
const float FloatConsts::MIN_NORMAL = 1.17549435E-38F;
const int32_t FloatConsts::SIGNIFICAND_WIDTH = 24;
const int32_t FloatConsts::MAX_EXPONENT = 127;
const int32_t FloatConsts::MIN_EXPONENT = -126;
const int32_t FloatConsts::MIN_SUB_EXPONENT = -149;
const int32_t FloatConsts::EXP_BIAS = 127;
const int32_t FloatConsts::SIGN_BIT_MASK = -2147483648;
const int32_t FloatConsts::EXP_BIT_MASK = 2139095040;
const int32_t FloatConsts::SIGNIF_BIT_MASK = 8388607;

const double DoubleConsts::POSITIVE_INFINITY = 1.0D / 0.0;
const double DoubleConsts::NEGATIVE_INFINITY = -1.0D / 0.0;
const double DoubleConsts::NaN = 0.0D / 0.0;
const double DoubleConsts::MAX_VALUE = 1.7976931348623157E308D;
const double DoubleConsts::MIN_VALUE = 4.9E-324D;
const double DoubleConsts::MIN_NORMAL = 2.2250738585072014E-308D;
const int32_t DoubleConsts::SIGNIFICAND_WIDTH = 53;
const int32_t DoubleConsts::MAX_EXPONENT = 1023;
const int32_t DoubleConsts::MIN_EXPONENT = -1022;
const int32_t DoubleConsts::MIN_SUB_EXPONENT = -1074;
const int32_t DoubleConsts::EXP_BIAS = 1023;
const int64_t DoubleConsts::SIGN_BIT_MASK = -9223372036854775808L;
const int64_t DoubleConsts::EXP_BIT_MASK = 9218868437227405312L;
const int64_t DoubleConsts::SIGNIF_BIT_MASK = 4503599627370495L;
