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

#ifndef SRC_CODEC_CODECUTIL_H_
#define SRC_CODEC_CODECUTIL_H_

#include <Index/Exception.h>
#include <Store/DataInput.h>
#include <Store/DataOutput.h>
#include <Util/Bytes.h>
#include <Util/Exception.h>
#include <string>

namespace lucene {
namespace core {
namespace codec {

class CodecUtil {
 private:
  CodecUtil() = default;

 public:
  static const uint32_t CODEC_MAGIC = 0x3fd76c17;
  static const uint32_t FOOTER_MAGIC = ~CODEC_MAGIC;

  static int64_t CheckFooter(lucene::core::store::ChecksumIndexInput* in);

/*
  TODO(0ctopus13prime): Fix this
  static int64_t
  CheckFooter(lucene::core::store::ChecksumIndexInput& in, Thrwable..????) {
  }
*/

  static uint32_t CheckHeader(lucene::core::store::DataInput* in,
                              const std::string& codec,
                              const uint32_t min_version,
                              const uint32_t max_version) {
    const uint32_t actual_header = in->ReadInt32();
    if (actual_header != CODEC_MAGIC) {
      throw lucene::core::index::CorruptIndexException("Codec header mismatch: actual header=" +
                                                       std::to_string(actual_header) +
                                                       " vs expected header = " +
                                                       std::to_string(CODEC_MAGIC));
    }

    return CheckHeaderNoMagic(in, codec, min_version, max_version);
  }

  static uint32_t CheckHeaderNoMagic(lucene::core::store::DataInput* in,
                                     const std::string& codec,
                                     const uint32_t min_version,
                                     const uint32_t max_version) {
    const std::string actual_codec = in->ReadString();
    if (actual_codec != codec) {
      throw lucene::core::index::CorruptIndexException("Codec header mismatch: actual header=" +
                                                       actual_codec +
                                                       " vs expected header = " +
                                                       std::to_string(CODEC_MAGIC));
    }

    const uint32_t actual_version = in->ReadInt32();
    if (actual_version < min_version) {
      throw lucene::core::index::IndexFormatTooOldException("Index format is too old. "
                                                            "actual version=" + 
                                                            std::to_string(actual_version) +
                                                            ", min_version=" +
                                                            std::to_string(min_version) +
                                                            ", max_version=" +
                                                            std::to_string(max_version));
    }
    if (actual_version > max_version) {
      throw lucene::core::index::IndexFormatTooNewException("Index format is too new. "
                                                            "actual version=" + 
                                                            std::to_string(actual_version) +
                                                            ", min_version=" +
                                                            std::to_string(min_version) +
                                                            ", max_version=" +
                                                            std::to_string(max_version));
    }

    return actual_version;
  }

  static uint32_t CheckIndexHeader(lucene::core::store::DataInput* in,
                                   const std::string& codec,
                                   const uint32_t min_version,
                                   const uint32_t max_version,
                                   const char expected_id[],
                                   const std::string& expected_suffix);

  static void VerifyAndCopyIndexHeader(lucene::core::store::IndexInput* in,
                                       lucene::core::store::DataOutput* out,
                                       const char expected_id[],
                                       const uint32_t expected_id_size);

  static uint32_t IndexHeaderSize(lucene::core::store::IndexInput* in);

  static void ReadIndexHeader(lucene::core::store::IndexInput* in,
                              char buffer[]);

  static uint32_t FooterLength();

  static void ReadFooter(lucene::core::store::IndexInput* in, char buffer[]);

  static void CheckIndexHeaderID(lucene::core::store::DataInput* in,
                                 char expected_id[]);

  static std::string CheckIndexHeaderSuffix(lucene::core::store::DataInput* in,
                                            const std::string& expected_suffix);

  static void WriteFooter(lucene::core::store::IndexOutput* out);

  static int64_t RetrieveChecksum(lucene::core::store::IndexInput* in);

  static int64_t ChecksumEntireFile(lucene::core::store::IndexInput* input);

  static void WriteHeader(lucene::core::store::DataOutput* out,
                          const std::string& codec,
                          const uint32_t version) {
    lucene::core::util::BytesRef bytes(codec.c_str(), 0, codec.size());
    if (bytes.length != codec.size() ||
        bytes.length >= 128) {
      throw lucene::core::util::IllegalArgumentException(
      "Codec must be simple ASCII, less than 128 characters in length [got" +
      codec + "]");
    }

    out->WriteInt32(CODEC_MAGIC);
    out->WriteString(codec);
    out->WriteInt32(version);
  }
};

}  // namespace codec
}  // namespace core
}  // namespace lucene

#endif  // SRC_CODEC_CODECUTIL_H_
