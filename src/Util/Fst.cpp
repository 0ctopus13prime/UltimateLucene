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

#include <Util/Fst.h>

using lucene::core::util::FST;
using lucene::core::util::IntsRef;
using lucene::core::util::IntSequenceOutputs;

/**
 *  FST
 */
template<typename T>
const std::string FST<T>::FILE_FORMAT_NAME("FST");

/**
 *  IntSequenceOutputs 
 */
IntsRef IntSequenceOutputs::NO_OUTPUT;
