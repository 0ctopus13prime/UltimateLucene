#ifndef LUCENE_CORE_UTIL_CONCURRENCY_H_
#define LUCENE_CORE_UTIL_CONCURRENCY_H_

template <typename T>
class CloseableThreadLocal {
  private:
    thread_local static T* reference;

  public:
    T* Get() const noexcept {
      return reference;
    }

    void Set(T* object) noexcept {
      reference = object;
    }

    static void Close(CloseableThreadLocal<T>*& closeable_thread_local) noexcept {
      if(closeable_thread_local != nullptr) {
        delete closeable_thread_local;
        closeable_thread_local = nullptr;
      }
    }
};

template <typename T>
thread_local T* CloseableThreadLocal<T>::reference(nullptr);

#endif
