#ifndef LUCENE_CORE_DOCUMENT_INDEXABLE_FIELD_H_
#define LUCENE_CORE_DOCUMENT_INDEXABLE_FIELD_H_

#include <string>

namespace lucene {
  namespace core {
    namespace document {


      class IndexableField {
      public:
        virtual const string& name() = 0;
        virtual IndexableFieldType& fieldType() = 0;
        virtual TokenStream& tokenStream(Analyzer& analyzer, TokenStream& reuse) = 0;
        virtual BytesRef& binaryValue() = 0;
        virtual string& stringValue() = 0;
        virtual Reader& readerValue() = 0;
        virtual Number numericValue() = 0;
      };



    }; // End of namespace document
  }; // End of namespace core
}; // End of namespace lucene


#endif
