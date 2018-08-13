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

#ifndef SRC_STORE_DIRECTORY_H_
#define SRC_STORE_DIRECTORY_H_

#include <Store/DataOutput.h>
#include <Store/DataInput.h>
#include <Store/Exception.h>
#include <atomic>
#include <string>
#include <set>
#include <vector>

namespace lucene {
namespace core {
namespace store {

class Directory;

class Lock {
 public:
  Lock() = default;

  virtual ~Lock() = default;

  virtual void Close() = 0;

  virtual void EnsureValid() = 0;
};

class LockFactory {
 public:
  LockFactory() = default;

  virtual ~LockFactory() = default;

  virtual std::unique_ptr<Lock>
  ObtainLock(Directory& dir, const std::string& lock_name) = 0;
};

class IOUtils {
 private:
  IOUtils() = default;
  ~IOUtils() = default;

 public:
  static void
  DeleteFilesIgnoringExceptions(Directory& dir,
                                const std::vector<std::string>& files);
};

class Directory {
 protected:
  virtual void EnsureOpen() { }

 public:
  Directory() = default;

  virtual ~Directory() = default;

  virtual std::vector<std::string> ListAll() = 0;

  virtual void DeleteFile(const std::string& name) = 0;

  virtual uint64_t FileLength(const std::string& name) = 0; 

  virtual std::unique_ptr<IndexOutput>
  CreateOutput(const std::string& name, const IOContext& context) = 0;

  virtual std::unique_ptr<IndexOutput>
  CreateTempOutput(const std::string& prefix,
                   const std::string& suffix,
                   const IOContext& context) = 0;

  virtual void Sync(const std::vector<std::string>& names) = 0;

  virtual void Rename(const std::string& source, const std::string& dest) = 0;

  virtual void SyncMetaData() = 0;

  virtual std::unique_ptr<IndexInput>
  OpenInput(const std::string& name, const IOContext& context) = 0;

  std::unique_ptr<ChecksumIndexInput>
  OpenChecksumInput(const std::string& name, const IOContext& context);

  virtual std::unique_ptr<Lock> ObtainLock(const std::string& name) = 0;

  virtual void Close() = 0;

  void CopyFrom(Directory& from,
                const std::string& src,
                const std::string& dest,
                const IOContext& context);
};

class BaseDirectory: public Directory {
 protected:
  volatile bool is_open;
  std::shared_ptr<LockFactory> lock_factory;

 protected:
  BaseDirectory(const std::shared_ptr<LockFactory>& lock_factory);

  void EnsureOpen();

 public:
  ~BaseDirectory() = default;

  std::unique_ptr<Lock> ObtainLock(const std::string& name);
};

class FSDirectory: public BaseDirectory {
 protected:
  std::string directory;
 
 private:
  // TODO(0ctopus13prime): It's not thread safe. Make it thread safe
  std::set<std::string> pending_deletes; 
  std::atomic<std::uint32_t> ops_since_last_delete;
  std::atomic<std::uint32_t> next_temp_file_counter;

 private:
  void MaybeDeletePendingFiles() {
    // TODO(0ctopus13prime): Fix this
  }

  void PrivateDeleterFile(const std::string& name, bool is_pending_delete) {
    // TODO(0ctopus13prime): Fix this
  }

 protected:
  FSDirectory(const std::string path,
              const std::shared_ptr<LockFactory>& lock_factory);

  void EnsureCanRead(const std::string& name) {
    // TODO(0ctopus13prime): Fix this
  }

 public:
  static std::vector<std::string>
  ListAllWithSkipNames(const std::string& dir,
                       const std::set<std::string>& skip_names) {
    // TODO(0ctopus13prime): Fix this
    return {};
  }

  void Fsync(const std::string& name) {
    // TODO(0ctopus13prime): Fix this
  }

 public:
  std::vector<std::string> ListAll() {
    // TODO(0ctopus13prime): Fix this
    return {};
  }
 
  uint64_t FileLength(const std::string& name) {
    // TODO(0ctopus13prime): Fix this
    return 0;
  }

  std::unique_ptr<IndexOutput>
  CreateOutput(const std::string& name, const IOContext& context) {
    // TODO(0ctopus13prime): Fix this
    return std::unique_ptr<IndexOutput>(); 
  }

  std::unique_ptr<IndexOutput> CreateTempOutput(const std::string& prefix,
                                                const std::string& suffix,
                                                const IOContext& context) {
    // TODO(0ctopus13prime): Fix this
    return std::unique_ptr<IndexOutput>(); 
  }

  void Sync(const std::vector<std::string>& names) {
    // TODO(0ctopus13prime): Fix this
  }

  void Rename(const std::string& source, const std::string& dest) {
    // TODO(0ctopus13prime): Fix this
  }

  void SyncMetaData() {
    // TODO(0ctopus13prime): Fix this
  }

  void Close() {
    // TODO(0ctopus13prime): Not synchronized. Make it thread safe
    // TODO(0ctopus13prime): Fix this
  }

  const std::string& GetDirectory();

  void DeleteFile(const std::string& name) {
    // TODO(0ctopus13prime): Fix this
  }

  bool CheckPendingDeletions() {
    // TODO(0ctopus13prime): Fix this
  }

  // TODO(0ctopus13prime): Not synchronized. Make it thread safe
  void DeletePendingFiles() {
    // TODO(0ctopus13prime): Fix this
  }
};

class MMapDirectory: public FSDirectory {
 public:
  static const uint32_t CHUNK_SIZE_POWER = 30;
  static const uint32_t DEFAULT_MAX_CHUNK_SIZE = (1 << CHUNK_SIZE_POWER);

 private:
  bool preload;

 public:
  MMapDirectory(const std::string& path);

  MMapDirectory(const std::string& path,
                const std::shared_ptr<LockFactory>& lock_factory);

  void SetPreLoad(const bool new_preload) noexcept {
    preload = new_preload;
  }

  bool IsPreLoad() const noexcept {
    return preload;
  }

  std::unique_ptr<IndexInput> OpenInput(const std::string& name,
                                        const IOContext& context);
};

}  // store
}  // core
}  // lucene

#endif  // SRC_STORE_DIRECTORY_H_
