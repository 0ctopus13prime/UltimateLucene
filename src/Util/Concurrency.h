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

#ifndef SRC_UTIL_CONCURRENCY_H_
#define SRC_UTIL_CONCURRENCY_H_

#include <functional>
#include <unordered_map>
#include <utility>
#include <string>
#include <stdexcept>
#include <typeindex>
#include <typeinfo>

namespace lucene {
namespace core {
namespace util {

template <typename TYPE>
class Singleton {
 private:
  Singleton() { }

 public:
  Singleton(const Singleton&) = delete;
  Singleton& operator=(const Singleton&) = delete;
  static TYPE& GetInstance() {
    static TYPE instance;
    return instance;
  }
};

class EmptyThreadLocalException: public std::exception {
 public:
  EmptyThreadLocalException()
    : std::exception() {
  }

  virtual const char* what() const throw() {
    return "Element was not found";
  }
};

class ThreadLocalVariableDestructedAlreadyException: public std::exception {
 public:
  ThreadLocalVariableDestructedAlreadyException()
    : std::exception() {
  }

  virtual const char* what() const throw() {
    return "Thread local was destructed already";
  }
};

template <typename TYPE>
class ThreadLocalItemWrapper {
 private:
  bool* thread_local_closed;

 public:
  TYPE item;

 public:
  ThreadLocalItemWrapper(ThreadLocalItemWrapper&& other)
    : thread_local_closed(other.thread_local_closed),
      item(std::move(other.item)) {
  }

  ThreadLocalItemWrapper(const TYPE& item, bool* thread_local_closed)
    : thread_local_closed(thread_local_closed),
      item(item) {
  }

  ThreadLocalItemWrapper(TYPE&& item, bool* thread_local_closed)
    : thread_local_closed(thread_local_closed),
      item(std::forward<TYPE>(item)) {
  }

  void SetClosed() {
    *thread_local_closed = true;
  }
};

template <typename CLASS, typename TYPE>
class CloseableThreadLocal;

template <typename CLASS, typename TYPE>
class CloseableThreadLocalReference {
 private:
  std::unordered_map<size_t, ThreadLocalItemWrapper<TYPE>> reference;

 public:
  TYPE& Get(const size_t instance_id) {
    auto it = reference.find(instance_id);
    if (it == reference.end()) {
      throw EmptyThreadLocalException();
    } else {
      return it->second.item;
    }
  }

  void Set(CloseableThreadLocal<CLASS, TYPE>* ctl, const TYPE& object) {
    // reference[addr_this] = object; TODO. How to branch using <type_traits>??
    reference.insert(
      std::make_pair(
        ctl->addr_this,
        ThreadLocalItemWrapper<TYPE>(object, &(ctl->thread_local_closed))));
  }

  void Set(CloseableThreadLocal<CLASS, TYPE>* ctl, TYPE&& object) {
    reference.insert(
      std::make_pair(
        ctl->addr_this,
        ThreadLocalItemWrapper<TYPE>(
          std::forward<TYPE>(object), &(ctl->thread_local_closed))));
  }

  void Erase(const size_t instance_id) {
    reference.erase(instance_id);
  }

  ~CloseableThreadLocalReference() {
    for (auto& entry : reference) {
      entry.second.SetClosed();
    }
  }
};

/**
 * This thread local "MUST" not shared between different threads
 */
template <typename CLASS, typename TYPE>
class CloseableThreadLocal {
  friend class CloseableThreadLocalReference<CLASS, TYPE>;

 private:
  thread_local static CloseableThreadLocalReference<CLASS, TYPE> reference;

 private:
  size_t addr_this;
  bool thread_local_closed;

 private:
  void CheckThreadLocalDestructed() {
    if (thread_local_closed) {
      throw ThreadLocalVariableDestructedAlreadyException();
    }
  }

 public:
  CloseableThreadLocal()
    : addr_this(reinterpret_cast<size_t>(this)),
      thread_local_closed(false) {
  }

  ~CloseableThreadLocal() {
    Close();
  }

  TYPE& Get() {
    CheckThreadLocalDestructed();
    return reference.Get(addr_this);
  }

  void Set(const TYPE& object) {
    CheckThreadLocalDestructed();
    reference.Set(this, object);
  }

  void Set(TYPE&& object) {
    CheckThreadLocalDestructed();
    reference.Set(this, std::forward<TYPE>(object));
  }

  void Close() {
    if (!thread_local_closed) {
      bool* kkk = &thread_local_closed;
      reference.Erase(addr_this);
    }
  }
};

template <typename CLASS, typename TYPE>
thread_local CloseableThreadLocalReference<CLASS, TYPE>
CloseableThreadLocal<CLASS, TYPE>::reference;

}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_CONCURRENCY_H_
