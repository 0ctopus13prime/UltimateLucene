/*
 *
 * Copyright (c) 2018-2019 Doo Yong Kim. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <Util/Numeric.h>

using lucene::core::util::numeric::FloatConsts;
using lucene::core::util::numeric::DoubleConsts;

const float FloatConsts::POSITIVE_INFINITY =
std::numeric_limits<float>::infinity();
const float FloatConsts::NEGATIVE_INFINITY =
-std::numeric_limits<float>::infinity();
const float FloatConsts::NaN = std::numeric_limits<float>::quiet_NaN();
const float FloatConsts::MAX_VALUE = std::numeric_limits<float>::max();
const float FloatConsts::MIN_VALUE = std::numeric_limits<float>::min();
const float FloatConsts::MIN_NORMAL = 1.17549435E-38F;
const int32_t FloatConsts::SIGNIFICAND_WIDTH = 24;
const int32_t FloatConsts::MAX_EXPONENT = 127;
const int32_t FloatConsts::MIN_EXPONENT = -126;
const int32_t FloatConsts::MIN_SUB_EXPONENT = -149;
const int32_t FloatConsts::EXP_BIAS = 127;
const int32_t FloatConsts::SIGN_BIT_MASK = -2147483648;
const int32_t FloatConsts::EXP_BIT_MASK = 2139095040;
const int32_t FloatConsts::SIGNIF_BIT_MASK = 8388607;

const double DoubleConsts::POSITIVE_INFINITY =
std::numeric_limits<float>::infinity();
const double DoubleConsts::NEGATIVE_INFINITY =
-std::numeric_limits<float>::infinity();
const double DoubleConsts::NaN = std::numeric_limits<double>::quiet_NaN();
const double DoubleConsts::MAX_VALUE = std::numeric_limits<double>::max();
const double DoubleConsts::MIN_VALUE = std::numeric_limits<double>::min();
const double DoubleConsts::MIN_NORMAL = 2.2250738585072014E-308D;
const int32_t DoubleConsts::SIGNIFICAND_WIDTH = 53;
const int32_t DoubleConsts::MAX_EXPONENT = 1023;
const int32_t DoubleConsts::MIN_EXPONENT = -1022;
const int32_t DoubleConsts::MIN_SUB_EXPONENT = -1074;
const int32_t DoubleConsts::EXP_BIAS = 1023;
const int64_t DoubleConsts::SIGN_BIT_MASK = -9223372036854775808L;
const int64_t DoubleConsts::EXP_BIT_MASK = 9218868437227405312L;
const int64_t DoubleConsts::SIGNIF_BIT_MASK = 4503599627370495L;
