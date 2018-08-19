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

#ifndef SRC_UTIL_FILE_H_
#define SRC_UTIL_FILE_H_

#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <Util/Exception.h>
#include <cstring>
#include <string>
#include <vector>

namespace lucene {
namespace core {
namespace util {

class FileUtil {
 public:
  static bool Exists(const std::string& path) {
    return (access(path.c_str(), F_OK) == 0);
  }

  static bool IsDirectory(const std::string& path) {
    struct stat statbuf;
    stat(path.c_str(), &statbuf);
    return S_ISDIR(statbuf.st_mode);
  }

  static std::vector<std::string> ListFiles(const std::string& path) {
    const int fd = open(path.c_str(), O_DIRECTORY); 
    if (fd == -1) {
      throw lucene::core::util::IOException(strerror(errno));
    }

    std::vector<std::string> ret;
    DIR* dirp = fdopendir(fd);
    struct dirent* dp;

    while ((dp = readdir(dirp)) != NULL) {
      if (strcmp(dp->d_name, ".") == 0 ||
          strcmp(dp->d_name, "..") == 0) {
        continue;
      }

      ret.push_back(std::string(dp->d_name));
    }

    closedir(dirp);
    close(fd);

    return ret;
  }

  static void CreateDirectory(const std::string& path) {
    const int result =
    mkdir(path.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);  
    if (result != 0) {
      throw lucene::core::util::IOException(std::string(strerror(errno)));
    }
  }

  static void CreateDirectories(const std::string& path) {
    struct stat statbuf;
    char path_buf[PATH_MAX + 1];
    std::memcpy(path_buf, path.c_str(), path.length());
    path_buf[path.length()] = '\0';
    char* ptr = path_buf;
    bool done = false;

    while (!done) {
      ptr += strspn(ptr, "/");  
      ptr += strcspn(ptr, "/");
      done = (*ptr == '\0');
      *ptr = '\0';

      if (stat(path_buf, &statbuf) == -1) {
        if (errno == ENOENT) {
          FileUtil::CreateDirectory(std::string(path_buf));
        }
      } else if (!S_ISDIR(statbuf.st_mode)) {
        throw lucene::core::util::IOException(std::string(path_buf) +
                                              " is not a directory");
      }

      *ptr = '/';
    }
  }

  static void Delete(const std::string& path) {
    const int result = remove(path.c_str());

    if (result != 0) {
      if (errno != ENOENT) {
        throw lucene::core::util::IOException(std::string(strerror(errno)));
      }
    }
  }

  static std::string ToRealPath(const std::string& path) {
    char path_buf[PATH_MAX + 1];
    return std::string(realpath(path.c_str(), path_buf));
  }

  static void Fsync(const std::string& path) {
    const int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_APPEND);
    if (fd != 0) {
        throw lucene::core::util::IOException(std::string(strerror(errno)));
    }

    const int result = fsync(fd);
    if (result != 0) {
        throw lucene::core::util::IOException(std::string(strerror(errno)));
    }

    const int close_result = close(fd);
    if (close_result != 0) {
        throw lucene::core::util::IOException(std::string(strerror(errno)));
    }
  }

  static uint64_t Size(const std::string& path) {
    const int fd = open(path.c_str(), O_RDONLY);
    if (fd != 0) {
        throw lucene::core::util::IOException(std::string(strerror(errno)));
    }
    struct stat statbuf;
    stat(path.c_str(), &statbuf);

    const uint64_t size = statbuf.st_size;
    const int close_result = close(fd);
    if (close_result != 0) {
        throw lucene::core::util::IOException(std::string(strerror(errno)));
    }

    return size;
  }

  static void Move(const std::string& source, const std::string& dest) {
    const int result =
    rename(source.c_str(), dest.c_str());

    if (result != 0) {
      throw lucene::core::util::IOException(std::string(strerror(errno)));
    }
  }
};

}  // util
}  // core
}  // lucene

#endif  // SRC_UTIL_FILE_H_
