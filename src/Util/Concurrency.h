#ifndef LUCENE_CORE_UTIL_CONCURRENCY_H_
#define LUCENE_CORE_UTIL_CONCURRENCY_H_

#include <typeinfo>
#include <unordered_map>

namespace lucene { namespace core { namespace util {

class CloseableThreadLocal {
  private:
    thread_local static std::unordered_map<size_t, void*> reference;

  public:
    template <typename T>
    T* Get() const {
      auto it = reference.find(typeid(T).hash_code());
      if(it == reference.end()) {
        return nullptr;
      } else {
        return static_cast<T*>(it->second);
      }
    }

    template <typename T>
    void Set(T* object) {
      reference[typeid(T).hash_code()] = object;
    }
};

}}} // End of namespace

#endif
