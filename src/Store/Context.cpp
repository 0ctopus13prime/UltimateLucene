#include <Store/Context.h>

using lucene::core::store::MergeInfo;
using lucene::core::store::FlushInfo;
using lucene::core::store::IOContext;

const MergeInfo MergeInfo::DEFAULT(0, 0, true, 0);
const FlushInfo FlushInfo::DEFAULT(0, 0);
const IOContext IOContext::DEFAULT(IOContext::Context::DEFAULT);
const IOContext IOContext::READONCE(true);
const IOContext IOContext::READ(false);
