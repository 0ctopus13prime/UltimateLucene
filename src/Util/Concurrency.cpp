#include <Util/Concurrency.h>

thread_local std::unordered_map<std::type_index, void*> lucene::core::util::CloseableThreadLocal::reference{};
