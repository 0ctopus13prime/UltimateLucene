#ifndef LUCENE_CORE_UTIL_CONCURRENCY_H_
#define LUCENE_CORE_UTIL_CONCURRENCY_H_

#include <functional>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

namespace lucene { namespace core { namespace util {

template <typename CLASS>
class CloseableThreadLocal {
  private:
    thread_local static std::unordered_map<size_t, void*> reference;

  private:
    size_t addr_this;
    std::function<void(void*)> cleanup;

  public:
    CloseableThreadLocal()
      : addr_this(reinterpret_cast<size_t>(this)),
        cleanup([](void*){}) {
    }

    CloseableThreadLocal(std::function<void(void*)> cleanup)
      : addr_this(reinterpret_cast<size_t>(this)),
        cleanup(cleanup) {
    }

    ~CloseableThreadLocal() {
      Close();
    }

    template <typename T>
    T* Get() const {
      auto it = reference.find(addr_this);
      if(it == reference.end()) {
        return nullptr;
      } else {
        return static_cast<T*>(it->second);
      }
    }

    template <typename T>
    void Set(T* object) {
      reference[addr_this] = object;
    }

    void Close() {
      auto it = reference.find(addr_this);
      if(it != reference.end()) {
        cleanup(it->second);
        reference.erase(it);
      }
    }
};

template <typename CLASS>
thread_local std::unordered_map<size_t, void*> lucene::core::util::CloseableThreadLocal<CLASS>::reference{};

}}} // End of namespace

#endif
