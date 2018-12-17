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

#ifndef SRC_CODEC_LUCENE50_FORUTIL_H_
#define SRC_CODEC_LUCENE50_FORUTIL_H_

#include <Store/DataInput.h>
#include <Store/DataOutput.h>
#include <Util/Pack/PackedInts.h>
#include <Codec/Lucene50/Lucene50.h>
#include <cassert>
#include <cmath>
#include <cstring>
#include <limits>

namespace lucene {
namespace core {
namespace codec {

class ForUtil {
 public:
  static const uint32_t ALL_VALUES_EQUAL = 0;
  static const uint32_t MAX_ENCODED_SIZE = (Lucene50::POSTING_BLOCK_SIZE * 4);
  static uint32_t MAX_DATA_SIZE;

 private:
  uint32_t encoded_sizes[33];
  lucene::core::util::PackedInts::Decoder* decoders[33];
  lucene::core::util::PackedInts::Encoder* encoders[33];
  uint32_t iterations[33];

 private:
  static bool IsAllEqual(const int32_t data[]) {
    int32_t v = data[0];
    for (uint32_t i = 1 ; i < Lucene50::POSTING_BLOCK_SIZE ; ++i) {
      if (data[i] != v) {
        return false;
      }
    }

    return true;
  }

  static uint32_t BitsRequired(const int32_t data[]) {
    int64_t or_data = 0; 
    for (uint32_t i = 0 ; i < Lucene50::POSTING_BLOCK_SIZE ; ++i) {
      assert(data[i] >= 0);
      or_data |= data[i];
    }

    return lucene::core::util::PackedInts::BitsRequired(or_data);
  }

  static uint32_t EncodedSize(lucene::core::util::PackedInts::Format format,
                              const uint32_t packed_ints_version,
                              const uint32_t bits_per_value) {
    const uint64_t byte_count =
      format.ByteCount(packed_ints_version,
                       Lucene50::POSTING_BLOCK_SIZE,
                       bits_per_value);
    assert(byte_count >= 0 &&
           byte_count <= std::numeric_limits<int32_t>::max());
    return static_cast<uint32_t>(byte_count);
  }

 public:
  static uint32_t
  ComputeIterations(lucene::core::util::PackedInts::Decoder* decoder) {
    return static_cast<uint32_t>(
      std::ceil(static_cast<float>(Lucene50::POSTING_BLOCK_SIZE) /
       decoder->ByteValueCount())); 
  }

  void Init(const float acceptable_overhead_ratio,
            lucene::core::store::DataOutput* out) {
    using PackedInts = lucene::core::util::PackedInts;
    out->WriteVInt32(PackedInts::VERSION_CURRENT);

    for (uint32_t bpv = 1 ; bpv <= 32 ; ++bpv) {
      PackedInts::FormatAndBits format_and_bits =
        PackedInts::FastestFormatAndBits(Lucene50::POSTING_BLOCK_SIZE,
                                         bpv,
                                         acceptable_overhead_ratio);
      assert(format_and_bits.format.IsSupported(
        format_and_bits.bits_per_value));
      assert(format_and_bits.bits_per_value <= 32);
      encoded_sizes[bpv] = EncodedSize(format_and_bits.format,
                                PackedInts::VERSION_CURRENT,
                                format_and_bits.bits_per_value);
      encoders[bpv] =
        PackedInts::GetEncoder(format_and_bits.format,
                               PackedInts::VERSION_CURRENT,
                               format_and_bits.bits_per_value);
      decoders[bpv] =
        PackedInts::GetDecoder(format_and_bits.format,
                               PackedInts::VERSION_CURRENT,
                               format_and_bits.bits_per_value);
      iterations[bpv] = ComputeIterations(decoders[bpv]);

      out->WriteVInt32(format_and_bits.format.GetId() << 5 |
                     (format_and_bits.bits_per_value - 1));
    }
  }

  void Init(lucene::core::store::DataInput* in) {
    const uint32_t packed_ints_version = in->ReadVInt32();
    lucene::core::util::PackedInts::CheckVersion(packed_ints_version);

    for (uint32_t bpv = 1 ; bpv <= 32 ; ++bpv) {
      const uint32_t code = static_cast<uint32_t>(in->ReadVInt32()); 
      const uint32_t format_id = (code >> 5);
      const uint32_t bits_per_value = ((code & 31) + 1);

      lucene::core::util::PackedInts::Format format =
        lucene::core::util::PackedInts::Format::ById(format_id);
      assert(format.IsSupported(bits_per_value));
      encoded_sizes[bpv] =
        EncodedSize(format, packed_ints_version, bits_per_value);
      encoders[bpv] =
        lucene::core::util::PackedInts::GetEncoder(format,
                                                   packed_ints_version,
                                                   bits_per_value);
      decoders[bpv] =
        lucene::core::util::PackedInts::GetDecoder(format,
                                                   packed_ints_version,
                                                   bits_per_value);
      iterations[bpv] = ComputeIterations(decoders[bpv]);
    }
  }

  void WriteBlock(int32_t data[],
                  char encoded[],
                  lucene::core::store::IndexOutput* out) {
    if (IsAllEqual(data)) {
      out->WriteByte(ALL_VALUES_EQUAL);
      out->WriteVInt32(data[0]);
    } else {
      const uint32_t num_bits = BitsRequired(data);
      assert(num_bits > 0 && num_bits <= 32);
      lucene::core::util::PackedInts::Encoder* encoder =
        encoders[num_bits];
      const uint32_t iters = iterations[num_bits];
      assert(iters * encoder->ByteValueCount() >=
             Lucene50::POSTING_BLOCK_SIZE);
      const uint32_t encoded_size = encoded_sizes[num_bits];
      assert(iters * encoder->ByteBlockCount() >= encoded_size);

      out->WriteByte(num_bits);

      encoder->Encode(data, 0, encoded, 0, iters);
      out->WriteBytes(encoded, encoded_size);
    }
  }

  void ReadBlock(lucene::core::store::IndexInput* in,
                 char encoded[],
                 uint32_t decoded[]) {
    const uint32_t num_bits = static_cast<uint32_t>(in->ReadByte());
    assert(num_bits <= 32);

    if (num_bits == ALL_VALUES_EQUAL) {
      const int32_t value = in->ReadVInt32();
      std::memset(decoded,
                  0,
                  sizeof(uint32_t) * Lucene50::POSTING_BLOCK_SIZE);
    } else {
      const uint32_t encoded_size = encoded_sizes[num_bits];
      in->ReadBytes(encoded, 0, encoded_size);

      lucene::core::util::PackedInts::Decoder* decoder = decoders[num_bits];
      const uint32_t iters = iterations[num_bits];
      assert(iters * decoder->ByteValueCount() >=
             Lucene50::POSTING_BLOCK_SIZE);
    }
  }

  void SkipBlock(lucene::core::store::IndexInput* in) {
    const uint32_t num_bits = static_cast<uint32_t>(in->ReadByte());
    if (num_bits == ALL_VALUES_EQUAL) {
      in->ReadVInt32();
    } else {
      assert(num_bits > 0 && num_bits <= 32);
      const uint32_t encoded_size = encoded_sizes[num_bits];
      in->Seek(in->GetFilePointer() + encoded_size);
    }
  }
};  // ForUtil

}  // codec
}  // core
}  // lucene


#endif  // SRC_CODEC_LUCENE50_FORUTIL_H_
