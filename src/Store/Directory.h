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

#include <Util/IO.h>
#include <Store/DataInput.h>
#include <Store/DataOutput.h>
#include <Store/Lock.h>
#include <string>
#include <vector>

namespace lucene {
namespace core {
namespace store {

class Directory {
 public:
  Directory() { }
  virtual ~Directory() { }

  virtual std::vector<std::string> ListAll(); 

  virtual void DeleteFile(const std::string& name);

  virtual uint64_t FileLength(const std::string& name); 

  virtual IndexOutput CreateOutput(const std::string& name, const IOContext context);

  virtual IndexOutput CreateTempOutput(const std::string& prefix,
                                       const std::string& suffix,
                                       const IOContext& context);

  virtual void Sync(std::vector<std::string>& names);

  virtual void Rename(const std::string& source, const std::string& dest);

  virtual void SyncMetaData();

  virtual IndexInput OpenInput(const std::string& name,
                               const IOContext& context);

  virtual ChecksumIndexInput OpenChecksumInput(const std::string& name,
                                               const IOContext& context);

  virtual Lock ObtainLock(const std::string& name);

  virtual void Close();

  void CopyFrom(Directory& from,
                const std::string& src,
                const std::string& dest,
                const IOContext& context) {
    try {
      IndexInput is = from.OpenInput(src, context);
      IndexOutput os = CreateOutput(dest, context);
      os.CopyBytes(is, is.Length());
    } catch(...) {
      lucene::core::util::IOUtils::DeleteFilesIgnoringExceptions(*this, dest);
    }
  }
};

}  // store
}  // core
}  // lucene

#endif  // SRC_STORE_DIRECTORY_H_
