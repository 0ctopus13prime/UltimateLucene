#ifndef LUCENE_CORE_UTIL_ATTRIBUTE_IMPL_H
#define LUCENE_CORE_UTIL_ATTRIBUTE_IMPL_H

#include "attribute.h"

namespace lucene {
  namespace core {
    namespace util {

      class AttributeImpl: public Attribute {
        public:
          virtual ~AttributeImpl() {}
      };

    } // End of namespace util
  } // End of namespace core
} // End of namespace lucene

#endif
