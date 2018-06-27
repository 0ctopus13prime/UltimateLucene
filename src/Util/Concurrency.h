#ifndef LUCENE_CORE_UTIL_CONCURRENCY_H_
#define LUCENE_CORE_UTIL_CONCURRENCY_H_

#include <stdexcept>
#include <functional>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

namespace lucene { namespace core { namespace util {

class EmptyThreadLocalException: public std::exception{
  public:
    EmptyThreadLocalException()
      : std::exception() {
    };

    virtual const char* what() const throw() {
      return "Element was not found";
    }
};

template <typename CLASS, typename TYPE>
class CloseableThreadLocal {
  private:
    thread_local static std::unordered_map<size_t, TYPE> reference;

  private:
    size_t addr_this;
    std::function<void(void*)> cleanup;

  public:
    CloseableThreadLocal()
      : addr_this(reinterpret_cast<size_t>(this)) {
    }

    ~CloseableThreadLocal() {
      Close();
    }

    TYPE& Get() {
      auto it = reference.find(addr_this);
      if(it == reference.end()) {
        throw EmptyThreadLocalException();
      } else {
        return it->second;
      }
    }

    void Set(TYPE object) {
      reference[addr_this] = object;
    }

    void Close() {
      reference.erase(addr_this);
    }
};

template <typename CLASS, typename TYPE>
thread_local std::unordered_map<size_t, TYPE> lucene::core::util::CloseableThreadLocal<CLASS, TYPE>::reference{};

}}} // End of namespace

#endif
