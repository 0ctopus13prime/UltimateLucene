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

#ifndef SRC_INDEX_FILE_H_
#define SRC_INDEX_FILE_H_

#include <string>

namespace lucene {
namespace core {
namespace index {

class IndexFileNames {
 public:
  static const std::string SEGMENTS;
  static const std::string PENDING_SEGMENTS;
  static const std::string OLD_SEGMENTS_GEN;

 private:
  IndexFileNames() = default;

 public:
  static std::string FileNameFromGeneration(const std::string& base,
                                            const std::string& ext,
                                            const uint64_t gen);

  static std::string SegmentFileName(const std::string& segment_name,
                                     const std::string& segment_suffix,
                                     const std::string& ext);

  static bool MatchesExtension(const std::string& file_name,
                               const std::string& ext);

  static uint32_t IndexOfSegmentName(const std::string& filename);

  static std::string StripSegmentName(const std::string& filename);

  static uint64_t ParseGeneration(const std::string& filename);

  static std::string ParseSegmentName(const std::string& filename);

  static std::string StripExtension(const std::string& filename);

  static std::string GetExtension(const std::string& filename);
};

}  // namespace index
}  // namespace core
}  // namespace lucene

#endif  // SRC_INDEX_FILE_H_
