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

#include <stdlib.h>
#include <libgen.h>
#include <limits.h>
#include <Util/Exception.h>
#include <Util/File.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <string>
#include <vector>

using lucene::core::util::FileUtil;
using lucene::core::util::IOException;

TEST(FILE__UTIL__TESTS, EXSTS) {
  // For a directory
  {
    char tmpl[] = "/tmp/lucene_core_util_flieutil_tempdir.XXXXXX";
    const char* temp_dir = mkdtemp(tmpl);
    if (temp_dir == NULL) {
      std::cerr << "Failed to create a temporary directory, Error -> "
      << strerror(errno) << std::endl;
      FAIL();
    } else {
      ASSERT_TRUE(FileUtil::Exists(temp_dir));
      ASSERT_TRUE(FileUtil::IsDirectory(temp_dir));
    }

    FileUtil::Delete(temp_dir);
  }

  // For a file
  {
    char buffer[] = "/tmp/lucene_core_util_flieutil_tempfile.XXXXXX";
    const int fd = mkstemp(buffer);

    if (fd == -1) {
      std::cerr << "Failed to create a temporary file, Error -> "
      << strerror(errno) << std::endl;
      FAIL();
    } else {
      ASSERT_TRUE(FileUtil::Exists(buffer));
    }

    close(fd);
    FileUtil::Delete(buffer);
  }
}

TEST(FILE__UTIL__TESTS, LIST__FILES) {
  std::vector<std::string> file_names;
  char tmpl[] = "/tmp/lucene_core_util_flieutil_tempdir.XXXXXX";
  const char* temp_dir = mkdtemp(tmpl);
  if (temp_dir == NULL) {
    std::cerr << "Failed to create a temporary directory, Error -> "
    << strerror(errno) << std::endl;
    FAIL();
  }

  char file_tmpl[PATH_MAX];
  strcpy(file_tmpl, temp_dir);
  strcat(file_tmpl, "/lucene_core_util_flieutil_tempfile.XXXXXX");

  char tmp_file_name_buf[PATH_MAX];
  strcpy(tmp_file_name_buf, file_tmpl);

  // Create a temp file1
  int fd = mkstemp(tmp_file_name_buf);
  if (fd == -1) {
    std::cerr << "Failed to create a temporary file, Error -> "
    << strerror(errno) << std::endl;
    FAIL();
  }
  close(fd);
  file_names.push_back(std::string(tmp_file_name_buf));

  // Create a temp file2
  strcpy(tmp_file_name_buf, file_tmpl);

  fd = mkstemp(tmp_file_name_buf);
  if (fd == -1) {
    std::cerr << "Failed to create a temporary file. Error -> "
    << strerror(errno) << std::endl;
    FAIL();
  }
  close(fd);
  file_names.push_back(std::string(tmp_file_name_buf));


  std::vector<std::string> got_files =
  FileUtil::ListFiles(std::string(temp_dir));

  for (const std::string& tmp_file_name : file_names) {
    strcpy(tmp_file_name_buf, tmp_file_name.c_str());
    const char* base_name = basename(tmp_file_name_buf);
    if (std::find(got_files.begin(), got_files.end(), base_name) ==
        got_files.end()) {
      std::cerr << base_name << " was not found" << std::endl;
      FAIL();
    }
  }

  // Clean up
  for (const std::string& tmp_file_name : file_names) {
    FileUtil::Delete(tmp_file_name);
  }
  FileUtil::Delete(temp_dir);
}

TEST(FILE__UTIL__TESTS, CREATE__DIRECTORY) {
  char tmpl[] = "/tmp/lucene_core_util_flieutil_tempdir.XXXXXX";
  const char* temp_dir = mkdtemp(tmpl);
  if (temp_dir == NULL) {
    std::cerr << "Failed to create a temporary directory, Error -> "
    << strerror(errno) << std::endl;
    FAIL();
  }

  try {
    FileUtil::CreateDirectory(temp_dir);
    FAIL();
  } catch(IOException&) {
    // Ignore
  }

  FileUtil::Delete(temp_dir);
  try {
    FileUtil::CreateDirectory(temp_dir);
  } catch(IOException&) {
    FAIL();
  }

  FileUtil::Delete(temp_dir);
}

TEST(FILE__UTIL__TESTS, CREATE__DIRECTORIES) {
  std::string path("/tmp/x0jfBCA1/AB09JfxZ");
  try {
    FileUtil::CreateDirectories(path);
  } catch(...) {
    FAIL();
  }

  FileUtil::Delete(path);
  path.resize(path.rfind('/'));
  FileUtil::Delete(path);
}

TEST(FILE__UTIL__TESTS, MOVE) {
  char file_path_buf[PATH_MAX];
  strcpy(file_path_buf, "/tmp/lucene_core_util_flieutil_tempdir.XXXXXX");
  int fd = mkstemp(file_path_buf);
  if (fd == -1) {
    std::cerr << "Failed to create a temporary file. Error -> "
    << strerror(errno) << std::endl;
    FAIL();
  }

  close(fd);
  const char* file_name = basename(file_path_buf);

  char file_path_dest_buf[PATH_MAX];
  strcpy(file_path_dest_buf, file_path_buf);
  strcat(file_path_dest_buf, file_name);

  FileUtil::Move(file_path_buf, file_path_dest_buf);

  ASSERT_FALSE(FileUtil::Exists(file_path_buf));
  ASSERT_TRUE(FileUtil::Exists(file_path_dest_buf));

  FileUtil::Delete(file_path_dest_buf);
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
