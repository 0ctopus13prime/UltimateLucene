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

#include <Util/Pack/BulkOperation.h>
#include <Util/Pack/Direct.h>
#include <Util/Pack/PackedInts.h>
#include <Util/Pack/Packed64.h>
#include <Util/Pack/PackedThreeBlocks.h>
#include <Util/Pack/Writer.h>
#include <Util/Pack/Reader.h>

using lucene::core::codec::CodecUtil;
using lucene::core::util::BulkOperation;
using lucene::core::util::Direct8;
using lucene::core::util::Direct16;
using lucene::core::util::Direct32;
using lucene::core::util::Direct64;
using lucene::core::util::IllegalArgumentException;
using lucene::core::util::PackedInts;
// using lucene::core::util::Packed64;
using lucene::core::util::Packed64SingleBlock;
using lucene::core::util::PackedWriter;
using lucene::core::util::Packed8ThreeBlocks;
using lucene::core::util::Packed16ThreeBlocks;

/**
 *  PackedInts
 */

const float PackedInts::FASTEST = 7;
const float PackedInts::FAST = 0.5F;
const float PackedInts::DEFAULT = 0.25F;
const float PackedInts::COMPACT = 0;
const std::string PackedInts::CODEC_NAME("PackedInts");

const PackedInts::Format PackedInts::Format::PACKED(0);
const PackedInts::Format PackedInts::Format::PACKED_SINGLE_BLOCK(1);

bool
PackedInts::Format::IsSupported(const uint32_t bits_per_value) const noexcept {
  if (id == 0) {  // PACKED
    return (bits_per_value >= 1 && bits_per_value <= 64);
  } else {  // PACKED_SINGLE_BLOCK
    return Packed64SingleBlock::IsSupported(bits_per_value);
  }
}

std::unique_ptr<PackedInts::Writer>
PackedInts::GetWriterNoHeader(lucene::core::store::DataOutput* out,
                              PackedInts::Format format,
                              const uint32_t value_count,
                              const uint32_t bits_per_value,
                              const uint32_t mem) {
  return std::make_unique<PackedWriter>(format,
                                        out,
                                        value_count,
                                        bits_per_value,
                                        mem);
}

PackedInts::FormatAndBits
PackedInts::FastestFormatAndBits(uint32_t value_count,
                                 uint32_t bits_per_value,
                                 float acceptable_overhead_ratio) {
  acceptable_overhead_ratio = std::max(COMPACT, acceptable_overhead_ratio);
  acceptable_overhead_ratio = std::min(FASTEST, acceptable_overhead_ratio);
  float acceptable_overhead_per_value =
    acceptable_overhead_ratio * bits_per_value;

  uint32_t max_bits_per_value =
    bits_per_value + static_cast<uint32_t>(acceptable_overhead_per_value);
  int32_t actual_bits_per_value = -1;
  Format format = Format::PACKED;

  if (bits_per_value <= 8 && max_bits_per_value >= 8) {
    actual_bits_per_value = 8;
  } else if (bits_per_value <= 16 && max_bits_per_value >= 16) {
    actual_bits_per_value = 16;
  } else if (bits_per_value <= 32 && max_bits_per_value >= 32) {
    actual_bits_per_value = 32;
  } else if (bits_per_value <= 64 && max_bits_per_value >= 64) {
    actual_bits_per_value = 64;
  } else if (value_count <= Packed8ThreeBlocks::MAX_SIZE &&
             bits_per_value <= 24 && max_bits_per_value >= 24) {
    actual_bits_per_value = 24; 
  } else if (value_count <= Packed16ThreeBlocks::MAX_SIZE &&
             bits_per_value <= 48 && max_bits_per_value >= 48) {
    actual_bits_per_value = 48; 
  } else {
    for (uint32_t bpv = bits_per_value ; bpv <= max_bits_per_value ; ++bpv) {
      if (PackedInts::Format::PACKED_SINGLE_BLOCK.IsSupported(bpv)) {
        float overhead =
          PackedInts::Format::PACKED_SINGLE_BLOCK.OverheadPerValue(bpv);
        float acceptable_overhead =
          acceptable_overhead_per_value + bits_per_value - bpv;
        if (overhead <= acceptable_overhead) {
          actual_bits_per_value = bpv;
          format = Format::PACKED_SINGLE_BLOCK;
          break;
        }
      }
    }

    if (actual_bits_per_value < 0) {
      actual_bits_per_value = bits_per_value;
    }
  }

  return PackedInts::FormatAndBits(format, actual_bits_per_value);
}

PackedInts::Decoder* PackedInts::GetDecoder(PackedInts::Format format,
                                            const uint32_t version,
                                            const uint32_t bits_per_value) {
  CheckVersion(version);   
  return BulkOperation::Of(format, bits_per_value);
}

std::unique_ptr<PackedInts::Reader>
PackedInts::GetDirectReader(lucene::core::store::IndexInput* in) {
  const uint32_t version =
    CodecUtil::CheckHeader(in,
                           PackedInts::CODEC_NAME,
                           PackedInts::VERSION_START,
                           PackedInts::VERSION_CURRENT);
  const uint32_t bits_per_value = in->ReadVInt32();
  assert(bits_per_value > 0 && bits_per_value <= 64);
  const uint32_t value_count = in->ReadVInt32();
  const PackedInts::Format format = Format::ById(in->ReadVInt32());

  return GetDirectReaderNoHeader(in,
                                 format,
                                 version,
                                 value_count,
                                 bits_per_value);
}

std::unique_ptr<PackedInts::Reader>
PackedInts::GetDirectReaderNoHeader(lucene::core::store::IndexInput* in,
                                    PackedInts::Format format,
                                    const uint32_t version,
                                    const uint32_t value_count,
                                    const uint32_t bits_per_value) {
  CheckVersion(version); 
  if (format == PackedInts::Format::PACKED) {
    return std::make_unique<DirectPackedReader>(bits_per_value,
                                                value_count,
                                                in);
  } else if (format == PackedInts::Format::PACKED_SINGLE_BLOCK) {
    return std::make_unique<DirectPacked64SingleBlockReader>(bits_per_value,
                                                             value_count,
                                                             in);
  } else {
    throw IllegalArgumentException("Unknown format");
  }
}

PackedInts::Encoder*
PackedInts::GetEncoder(PackedInts::Format format,
                       const uint32_t version,
                       const uint32_t bits_per_value) {
  CheckVersion(version); 
  return BulkOperation::Of(format, bits_per_value);
}

std::unique_ptr<PackedInts::Reader>
PackedInts::GetReader(lucene::core::store::DataInput* in) {
  const uint32_t version =
    CodecUtil::CheckHeader(in,
                           PackedInts::CODEC_NAME,
                           PackedInts::VERSION_START,
                           PackedInts::VERSION_CURRENT);
  const uint32_t bits_per_value = in->ReadVInt32();  
  assert(bits_per_value > 0 && bits_per_value <= 64);
  const uint32_t value_count = in->ReadVInt32();
  PackedInts::Format format(PackedInts::Format::ById(in->ReadVInt32()));

  return GetReaderNoHeader(in, format, version, value_count, bits_per_value);
}

std::unique_ptr<PackedInts::Reader>
PackedInts::GetReaderNoHeader(lucene::core::store::DataInput* in,
                              PackedInts::Format format,
                              const uint32_t version,
                              const uint32_t value_count,
                              const uint32_t bits_per_value) {
  CheckVersion(version);  
  if (format == PackedInts::Format::PACKED) {
    switch (bits_per_value) {
      case 8:
        return std::make_unique<Direct8>(version, in, value_count);
      case 16:
        return std::make_unique<Direct16>(version, in, value_count);
      case 32:
        return std::make_unique<Direct32>(version, in, value_count);
      case 64:
        return std::make_unique<Direct64>(version, in, value_count);
      case 24:
        if (value_count <= Packed8ThreeBlocks::MAX_SIZE) {
          return std::make_unique<Packed8ThreeBlocks>(version, in, value_count);
        }
      case 48:
        if (value_count <= Packed16ThreeBlocks::MAX_SIZE) {
          return
          std::make_unique<Packed16ThreeBlocks>(version, in, value_count);
        }
    }

    // return std::make_unique<Packed64>(version, in, value_count, bits_per_value);
    return std::unique_ptr<PackedInts::Reader>();
  } else if (format == PackedInts::Format::PACKED_SINGLE_BLOCK) {
    return Packed64SingleBlock::Create(in, value_count, bits_per_value);
  } else {
    throw IllegalArgumentException("Unknown writer format, format's id=" +
                                   std::to_string(format.GetId()));
  }
}
