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

#include <Index/File.h>

using lucene::core::index::IndexFileNames;

/**
 *  IndexFileNames
 */

std::string IndexFileNames::FileNameFromGeneration(const std::string& base,
                                                   const std::string& ext,
                                                   const uint64_t gen) {
  // TODO(0ctopus13prime): Implement this
}

std::string IndexFileNames::SegmentFileName(const std::string& segment_name,
                                            const std::string& segment_suffix,
                                            const std::string& ext) {
  if (!ext.empty() || !segment_name.empty()) {
    std::string ret;
    ret.reserve(segment_name.length() + 2 +
                segment_suffix.length() +
                ext.length());

    ret += segment_name;
    if (!segment_suffix.empty()) {
      ret += '_';
      ret += segment_suffix;
    }

    if (!ext.empty()) {
      ret += '.';
      ret += ext;
    }

    return ret;
  } else {
    return segment_name;
  }
}

bool IndexFileNames::MatchesExtension(const std::string& file_name,
                                      const std::string& ext) {
  // TODO(0ctopus13prime): Implement this
  return true;
}

uint32_t IndexFileNames::IndexOfSegmentName(const std::string& filename) {
  // TODO(0ctopus13prime): Implement this
  return 0;
}

std::string IndexFileNames::StripSegmentName(const std::string& filename) {
  // TODO(0ctopus13prime): Implement this
  return "";
}

uint64_t IndexFileNames::ParseGeneration(const std::string& filename) {
  // TODO(0ctopus13prime): Implement this
  return 0L;
}

std::string IndexFileNames::ParseSegmentName(const std::string& filename) {
  // TODO(0ctopus13prime): Implement this
  return "";
}

std::string IndexFileNames::StripExtension(const std::string& filename) {
  // TODO(0ctopus13prime): Implement this
  return "";
}

std::string IndexFileNames::GetExtension(const std::string& filename) {
  // TODO(0ctopus13prime): Implement this
  return "";
}
