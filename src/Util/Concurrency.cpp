#include <Util/Concurrency.h>

thread_local std::unordered_map<size_t, void*> lucene::core::util::CloseableThreadLocal::reference{};
