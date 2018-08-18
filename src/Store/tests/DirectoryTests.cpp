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
#include <Util/File.h>
#include <iostream>
#include <memory>

using lucene::core::store::MMapDirectory;
using lucene::core::store::IndexInput;
using lucene::core::store::RandomAccessInput;
using lucene::core::store::FileIndexOutput;
using lucene::core::store::IndexOutput;
using lucene::core::store::IOContext;
using lucene::core::util::FileUtil;

/*
TEST(DIRECTORY__TESTS, MMAP__DIRECTORY__BYTE__IO) {
  const size_t file_size = 11376;
  const std::string base("/tmp");
  const std::string name("mmap_out_test");
  FileUtil::Delete(base + '/' + name);

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

TEST(DIRECTORY__TESTS, MMAP__DIRECTORY__RANDOM__BYTE__IO) {
  const size_t file_size = 11376;
  const std::string base("/tmp");
  const std::string name("mmap_out_test");
  FileUtil::Delete(base + '/' + name);

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
    RandomAccessInput* rin_ptr = dynamic_cast<RandomAccessInput*>(in_ptr.get());
    ASSERT_NE(rin_ptr, nullptr);
    ASSERT_EQ(file_size, in_ptr->Length());

    for (size_t i = 0 ; i < file_size ; ++i) {
      ASSERT_EQ(static_cast<char>(i), rin_ptr->ReadByte(i));
    }
  }
}

TEST(DIRECTORY__TESTS, MMAP__DIRECTORY__INT16__IO) {
  const size_t elem_num = 11376;
  const std::string base("/tmp");
  const std::string name("mmap_out_test");
  FileUtil::Delete(base + '/' + name);

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

TEST(DIRECTORY__TESTS, MMAP__DIRECTORY__RANDOM__INT16__IO) {
  const size_t elem_num = 11376;
  const std::string base("/tmp");
  const std::string name("mmap_out_test");
  FileUtil::Delete(base + '/' + name);

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
    RandomAccessInput* rin_ptr = dynamic_cast<RandomAccessInput*>(in_ptr.get());
    ASSERT_NE(rin_ptr, nullptr);
    ASSERT_EQ(elem_num * sizeof(int16_t), in_ptr->Length());

    for (size_t i = 0 ; i < elem_num ; ++i) {
      ASSERT_EQ(static_cast<int16_t>(i),
                rin_ptr->ReadInt16(i * sizeof(int16_t)));
    }
  }
}

TEST(DIRECTORY__TESTS, MMAP__DIRECTORY__INT32__IO) {
  const size_t elem_num = 11376;
  const std::string base("/tmp");
  const std::string name("mmap_out_test");
  FileUtil::Delete(base + '/' + name);

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

TEST(DIRECTORY__TESTS, MMAP__DIRECTORY__RANDOM__INT32__IO) {
  const size_t elem_num = 11376;
  const std::string base("/tmp");
  const std::string name("mmap_out_test");
  FileUtil::Delete(base + '/' + name);

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
    RandomAccessInput* rin_ptr = dynamic_cast<RandomAccessInput*>(in_ptr.get());
    ASSERT_NE(rin_ptr, nullptr);
    ASSERT_EQ(elem_num * sizeof(int32_t), in_ptr->Length());

    for (size_t i = 0 ; i < elem_num ; ++i) {
      ASSERT_EQ(static_cast<int32_t>(i),
                rin_ptr->ReadInt32(i * sizeof(int32_t)));
    }
  }
}

TEST(DIRECTORY__TESTS, MMAP__DIRECTORY__INT64__IO) {
  const size_t elem_num = 11376;
  const std::string base("/tmp");
  const std::string name("mmap_out_test");
  FileUtil::Delete(base + '/' + name);

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

TEST(DIRECTORY__TESTS, MMAP__DIRECTORY__RANDOM__INT64__IO) {
  const size_t elem_num = 11376;
  const std::string base("/tmp");
  const std::string name("mmap_out_test");
  FileUtil::Delete(base + '/' + name);

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
    RandomAccessInput* rin_ptr = dynamic_cast<RandomAccessInput*>(in_ptr.get());
    ASSERT_NE(rin_ptr, nullptr);

    ASSERT_EQ(elem_num * sizeof(int64_t), in_ptr->Length());
    for (size_t i = 0 ; i < elem_num ; ++i) {
      ASSERT_EQ(static_cast<int64_t>(i),
                rin_ptr->ReadInt64(i * sizeof(int64_t)));
    }
  }
}

TEST(DIRECTORY__TESTS, MMAP__DIRECTORY__VINT32__IO) {
  const size_t elem_num = 11376;
  const std::string base("/tmp");
  const std::string name("mmap_out_test");
  FileUtil::Delete(base + '/' + name);
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
  FileUtil::Delete(base + '/' + name);
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
  FileUtil::Delete(base + '/' + name);
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
  FileUtil::Delete(base + '/' + name);
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
  FileUtil::Delete(base + '/' + name);
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
  FileUtil::Delete(base + '/' + name);
  std::string dup_name(name + "_dup");
  FileUtil::Delete(base + '/' + dup_name);
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

TEST(DIRECTORY__TESTS, WRITE__BYTES) {
  const std::string base("/tmp");
  const std::string name("mmap_out_test");
  FileUtil::Delete(base + '/' + name);
  const uint32_t buf_len = FileIndexOutput::BUF_SIZE + 101;
  char buffer[buf_len];

  for (int i = 0 ; i < buf_len ; ++i) {
    buffer[i] = static_cast<char>(i);
  }

  // Write first
  {
    MMapDirectory dir(base);
    IOContext io_ctx;
    std::unique_ptr<IndexOutput> out_ptr = dir.CreateOutput(name, io_ctx);
    out_ptr->WriteBytes(buffer, buf_len);
    out_ptr->Close();
  }

  // Read afterward and write what I have read again
  {
    MMapDirectory dir(base);
    IOContext io_ctx;
    std::unique_ptr<IndexInput> in_ptr = dir.OpenInput(name, io_ctx);
    ASSERT_EQ(buf_len, in_ptr->Length());
  }
}
*/

// This is for generating binary to compare with Java version.
TEST(DIRECTORY__TESTS, JAVA__CMP__ETC) {
  const size_t elem_num = 100000000;
  const std::string base("/tmp");
  const std::string name("mmap_out_test");
  FileUtil::Delete(base + '/' + name);
  std::string str("content-");
  const uint32_t str_len = str.length();

  MMapDirectory dir(base);
  IOContext io_ctx;
  std::unique_ptr<IndexOutput> out_ptr = dir.CreateOutput(name, io_ctx);

  for (size_t i = 0 ; i < elem_num ; ++i) {
    out_ptr->WriteByte(static_cast<char>(i));
    out_ptr->WriteInt32(static_cast<int32_t>(i));
    out_ptr->WriteInt64(static_cast<int64_t>(i));
    out_ptr->WriteVInt32(static_cast<int32_t>(i));
    out_ptr->WriteVInt64(static_cast<int64_t>(i));
    out_ptr->WriteInt16(static_cast<int16_t>(i));
    out_ptr->WriteZInt64(static_cast<int64_t>(i));
    str.resize(str_len);
    str += std::to_string(i);
    out_ptr->WriteString(str);
  }

  out_ptr->Close();

  // Java Version
  // Linux compare two binaries command : cmp
  //
  // import org.apache.lucene.store.IOContext;
  // import org.apache.lucene.store.IndexOutput;
  // import org.apache.lucene.store.MMapDirectory;
  // import java.io.File;
  // import java.io.IOException;
  // import java.nio.file.Paths;
  //
  // public class TempMain {
  //   public static void main(String[] args) throws IOException {
  //     new File("/tmp/mmap_test_out").delete();
  //
  //     long s = System.currentTimeMillis();
  //     MMapDirectory mMapDirectory = new MMapDirectory(Paths.get("/tmp"));
  //     IOContext ioContext = new IOContext();
  //     IndexOutput out =
  //     mMapDirectory.createOutput("mmap_test_out", ioContext);
  //     int N = 100_000_000;
  //
  //     for (int i = 0 ; i < N; ++i) {
  //       out.writeByte((byte) i);
  //       out.writeInt(i);
  //       out.writeLong(i);
  //       out.writeVInt(i);
  //       out.writeVLong(i);
  //       out.writeShort((short) i);
  //       out.writeZLong(i);
  //       out.writeString("content-" + i);
  //     }
  //
  //     out.close();
  //   }
  // }
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
