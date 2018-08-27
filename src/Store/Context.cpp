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
#include <Store/Context.h>

using lucene::core::store::MergeInfo;
using lucene::core::store::FlushInfo;
using lucene::core::store::IOContext;

const MergeInfo MergeInfo::DEFAULT(0, 0, true, 0);
const FlushInfo FlushInfo::DEFAULT(0, 0);
const IOContext IOContext::DEFAULT(IOContext::Context::DEFAULT);
const IOContext IOContext::READONCE(true);
const IOContext IOContext::READ(false);
