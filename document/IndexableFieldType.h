#ifndef LUCENE_CORE_INDEXABLE_FIELD_TYPE_H_
#define LUCENE_CORE_INDEXABLE_FIELD_TYPE_H_

namespace lucene {
  namespace core {
    namespace document {

      class IndexableFieldType {
        virtual bool stored() = 0;
        virtual bool tokenized() = 0;
        virtual bool storeTermVectors() = 0;
        virtual bool storeTermVectorOffsets() = 0;
        virtual bool storeTermVectorPositions() = 0;
        virtual bool storeTermVectorPayloads() = 0;
        virtual bool omitNorms() = 0;
        virtual IndexOptions& indexOptions() = 0;
        virtual DocValuesType& docValuesType() = 0;
        virtual int pointDimensionCount() = 0;
        virtual unsigned int pointNumBytes() = 0;
      };

    }; // End of namespace document
  }; // End of namespace core
}; // End of namespace lucene


#endif
