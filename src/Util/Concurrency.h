#ifndef LUCENE_CORE_UTIL_CONCURRENCY_H_
#define LUCENE_CORE_UTIL_CONCURRENCY_H_

#include <typeindex>
#include <typeinfo>
#include <unordered_map>

namespace lucene { namespace core { namespace util {

class CloseableThreadLocal {
  private:
    thread_local static std::unordered_map<std::type_index, void*> reference;

  public:
    template <typename T>
    T* Get() const {
      std::type_index idx(typeid(T));
      auto it = reference.find(idx);
      if(it == reference.end()) {
        return nullptr;
      } else {
        return static_cast<T*>(it->second);
      }
    }

    template <typename T>
    void Set(T* object) {
      reference[std::type_index(typeid(*object))] = object;
    }
};

}}} // End of namespace

#endif
