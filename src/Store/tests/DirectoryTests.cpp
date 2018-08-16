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

#include <gtest/gtest.h>
#include <Store/Directory.h>
#include <iostream>
#include <memory>

using lucene::core::store::MMapDirectory;
using lucene::core::store::IndexInput;
using lucene::core::store::IndexOutput;
using lucene::core::store::IOContext;

TEST(DIRECTORY__TESTS, MMAP__DIRECTORY__BYTE__IO) {
  const size_t file_size = 11376;
  const std::string base("/tmp");
  const std::string name("mmap_out_test");
  const std::string prefix("lucene_prefix");
  const std::string suffix("lucene_suffix");

  // Write first
  {
    MMapDirectory dir(base);
    IOContext io_ctx;
    std::unique_ptr<IndexOutput> out_ptr = dir.CreateOutput(name, io_ctx);

    for (size_t i = 0 ; i < file_size ; ++i) {
      out_ptr->WriteByte(static_cast<char>(i));
    }

    out_ptr->Close();
  }

  // Read afterward
  {
    MMapDirectory dir(base);

    IOContext io_ctx;
    std::unique_ptr<IndexInput> in_ptr = dir.OpenInput(name, io_ctx);

    ASSERT_EQ(file_size, in_ptr->Length());
    for (size_t i = 0 ; i < file_size ; ++i) {
      ASSERT_EQ(static_cast<char>(i), in_ptr->ReadByte());
    }
  }
}

TEST(DIRECTORY__TESTS, MMAP__DIRECTORY__INT16__IO) {
  const size_t elem_num = 11376;
  const std::string base("/tmp");
  const std::string name("mmap_out_test");
  const std::string prefix("lucene_prefix");
  const std::string suffix("lucene_suffix");

  // Write first
  {
    MMapDirectory dir(base);
    IOContext io_ctx;
    std::unique_ptr<IndexOutput> out_ptr = dir.CreateOutput(name, io_ctx);

    for (size_t i = 0 ; i < elem_num ; ++i) {
      out_ptr->WriteInt16(static_cast<int16_t>(i));
    }

    out_ptr->Close();
  }

  // Read afterward
  {
    MMapDirectory dir(base);
    IOContext io_ctx;
    std::unique_ptr<IndexInput> in_ptr = dir.OpenInput(name, io_ctx);

    ASSERT_EQ(elem_num * sizeof(int16_t), in_ptr->Length());
    for (size_t i = 0 ; i < elem_num ; ++i) {
      ASSERT_EQ(static_cast<int16_t>(i), in_ptr->ReadInt16());
    }
  }
}

TEST(DIRECTORY__TESTS, MMAP__DIRECTORY__INT32__IO) {
  const size_t elem_num = 11376;
  const std::string base("/tmp");
  const std::string name("mmap_out_test");
  const std::string prefix("lucene_prefix");
  const std::string suffix("lucene_suffix");

  // Write first
  {
    MMapDirectory dir(base);
    IOContext io_ctx;
    std::unique_ptr<IndexOutput> out_ptr = dir.CreateOutput(name, io_ctx);

    for (size_t i = 0 ; i < elem_num ; ++i) {
      out_ptr->WriteInt32(static_cast<int32_t>(i));
    }

    out_ptr->Close();
  }

  // Read afterward
  {
    MMapDirectory dir(base);
    IOContext io_ctx;
    std::unique_ptr<IndexInput> in_ptr = dir.OpenInput(name, io_ctx);

    ASSERT_EQ(elem_num * sizeof(int32_t), in_ptr->Length());
    for (size_t i = 0 ; i < elem_num ; ++i) {
      ASSERT_EQ(static_cast<int32_t>(i), in_ptr->ReadInt32());
    }
  }
}

TEST(DIRECTORY__TESTS, MMAP__DIRECTORY__INT64__IO) {
  const size_t elem_num = 11376;
  const std::string base("/tmp");
  const std::string name("mmap_out_test");
  const std::string prefix("lucene_prefix");
  const std::string suffix("lucene_suffix");

  // Write first
  {
    MMapDirectory dir(base);
    IOContext io_ctx;
    std::unique_ptr<IndexOutput> out_ptr = dir.CreateOutput(name, io_ctx);

    for (size_t i = 0 ; i < elem_num ; ++i) {
      out_ptr->WriteInt64(static_cast<int64_t>(i));
    }

    out_ptr->Close();
  }

  // Read afterward
  {
    MMapDirectory dir(base);
    IOContext io_ctx;
    std::unique_ptr<IndexInput> in_ptr = dir.OpenInput(name, io_ctx);

    ASSERT_EQ(elem_num * sizeof(int64_t), in_ptr->Length());
    for (size_t i = 0 ; i < elem_num ; ++i) {
      ASSERT_EQ(static_cast<int64_t>(i), in_ptr->ReadInt64());
    }
  }
}

TEST(DIRECTORY__TESTS, MMAP__DIRECTORY__VINT32__IO) {
  const size_t elem_num = 11376;
  const std::string base("/tmp");
  const std::string name("mmap_out_test");
  const std::string prefix("lucene_prefix");
  const std::string suffix("lucene_suffix");

  // Write first
  {
    MMapDirectory dir(base);
    IOContext io_ctx;
    std::unique_ptr<IndexOutput> out_ptr = dir.CreateOutput(name, io_ctx);

    for (size_t i = 0 ; i < elem_num ; ++i) {
      out_ptr->WriteVInt32(static_cast<int32_t>(i));
    }

    out_ptr->Close();
  }

  // Read afterward
  {
    MMapDirectory dir(base);
    IOContext io_ctx;
    std::unique_ptr<IndexInput> in_ptr = dir.OpenInput(name, io_ctx);

    for (size_t i = 0 ; i < elem_num ; ++i) {
      ASSERT_EQ(static_cast<int32_t>(i), in_ptr->ReadVInt32());
    }
  }
}

TEST(DIRECTORY__TESTS, MMAP__DIRECTORY__ZINT32__IO) {
  const size_t elem_num = 11376;
  const std::string base("/tmp");
  const std::string name("mmap_out_test");
  const std::string prefix("lucene_prefix");
  const std::string suffix("lucene_suffix");

  // Write first
  {
    MMapDirectory dir(base);
    IOContext io_ctx;
    std::unique_ptr<IndexOutput> out_ptr = dir.CreateOutput(name, io_ctx);

    for (size_t i = 0 ; i < elem_num ; ++i) {
      out_ptr->WriteZInt32(static_cast<int32_t>(i));
    }

    out_ptr->Close();
  }

  // Read afterward
  {
    MMapDirectory dir(base);
    IOContext io_ctx;
    std::unique_ptr<IndexInput> in_ptr = dir.OpenInput(name, io_ctx);

    for (size_t i = 0 ; i < elem_num ; ++i) {
      ASSERT_EQ(static_cast<int32_t>(i), in_ptr->ReadZInt32());
    }
  }
}

TEST(DIRECTORY__TESTS, MMAP__DIRECTORY__VINT64__IO) {
  const size_t elem_num = 11376;
  const std::string base("/tmp");
  const std::string name("mmap_out_test");
  const std::string prefix("lucene_prefix");
  const std::string suffix("lucene_suffix");

  // Write first
  {
    MMapDirectory dir(base);
    IOContext io_ctx;
    std::unique_ptr<IndexOutput> out_ptr = dir.CreateOutput(name, io_ctx);

    for (size_t i = 0 ; i < elem_num ; ++i) {
      out_ptr->WriteVInt64(static_cast<int64_t>(i));
    }

    out_ptr->Close();
  }

  // Read afterward
  {
    MMapDirectory dir(base);
    IOContext io_ctx;
    std::unique_ptr<IndexInput> in_ptr = dir.OpenInput(name, io_ctx);

    for (size_t i = 0 ; i < elem_num ; ++i) {
      ASSERT_EQ(static_cast<int64_t>(i), in_ptr->ReadVInt64());
    }
  }
}

TEST(DIRECTORY__TESTS, MMAP__DIRECTORY__ZINT64__IO) {
  const size_t elem_num = 11376;
  const std::string base("/tmp");
  const std::string name("mmap_out_test");
  const std::string prefix("lucene_prefix");
  const std::string suffix("lucene_suffix");

  // Write first
  {
    MMapDirectory dir(base);
    IOContext io_ctx;
    std::unique_ptr<IndexOutput> out_ptr = dir.CreateOutput(name, io_ctx);

    for (size_t i = 0 ; i < elem_num ; ++i) {
      out_ptr->WriteZInt64(static_cast<int64_t>(i));
    }

    out_ptr->Close();
  }

  // Read afterward
  {
    MMapDirectory dir(base);
    IOContext io_ctx;
    std::unique_ptr<IndexInput> in_ptr = dir.OpenInput(name, io_ctx);

    for (size_t i = 0 ; i < elem_num ; ++i) {
      ASSERT_EQ(static_cast<int64_t>(i), in_ptr->ReadZInt64());
    }
  }
}

TEST(DIRECTORY__TESTS, MMAP__DIRECTORY__STRING__IO) {
  const size_t elem_num = 11376;
  const std::string base("/tmp");
  const std::string name("mmap_out_test");
  const std::string prefix("lucene_prefix");
  const std::string suffix("lucene_suffix");
  std::string str("content-");
  const uint32_t str_len = str.length();

  // Write first
  {
    MMapDirectory dir(base);
    IOContext io_ctx;
    std::unique_ptr<IndexOutput> out_ptr = dir.CreateOutput(name, io_ctx);

    for (size_t i = 0 ; i < elem_num ; ++i) {
      str.resize(str_len);
      str += std::to_string(i);
      out_ptr->WriteString(str);
    }

    out_ptr->Close();
  }

  // Read afterward
  {
    MMapDirectory dir(base);
    IOContext io_ctx;
    std::unique_ptr<IndexInput> in_ptr = dir.OpenInput(name, io_ctx);

    for (size_t i = 0 ; i < elem_num ; ++i) {
      str.resize(str_len);
      str += std::to_string(i);
      ASSERT_EQ(str, in_ptr->ReadString());
    }
  }
}

TEST(DIRECTORY__TESTS, MMAP__DIRECTORY__ETC) {
  const size_t elem_num = 11376;
  const std::string base("/tmp");
  const std::string name("mmap_out_test");
  std::string dup_name(name + "_dup");
  const std::string prefix("lucene_prefix");
  const std::string suffix("lucene_suffix");
  std::string str("content-");
  const uint32_t str_len = str.length();

  // Write first
  {
    MMapDirectory dir(base);
    IOContext io_ctx;
    std::unique_ptr<IndexOutput> out_ptr = dir.CreateOutput(name, io_ctx);

    for (size_t i = 0 ; i < elem_num ; ++i) {
      str.resize(str_len);
      str += std::to_string(i);
      out_ptr->WriteString(str);
    }

    out_ptr->Close();
  }

  // Read afterward and write what I have read again
  {
    MMapDirectory dir(base);
    IOContext io_ctx;
    std::unique_ptr<IndexInput> in_ptr = dir.OpenInput(name, io_ctx);

    std::unique_ptr<IndexOutput> out_ptr = dir.CreateOutput(dup_name, io_ctx);
    out_ptr->CopyBytes(*(in_ptr.get()), in_ptr->Length());
    out_ptr->Close();
  }

  // Read duplicated file and validate it
  {
    MMapDirectory dir(base);
    IOContext io_ctx;
    std::unique_ptr<IndexInput> in_ptr = dir.OpenInput(dup_name, io_ctx);

    for (size_t i = 0 ; i < elem_num ; ++i) {
      str.resize(str_len);
      str += std::to_string(i);
      ASSERT_EQ(str, in_ptr->ReadString());
    }
  }
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
