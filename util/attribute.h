#ifndef LUCENE_CORE_UTIL_ATTRIBUTE_H_
#define LUCENE_CORE_UTIL_ATTRIBUTE_H_

#include <string>
#include "bytes_ref.h"

namespace lucene {
  namespace core {
    namespace util {

      class Attribute {
        public:
          virtual ~Attribute() {}
      };

      class TermToBytesRefAttribute: public Attribute {
        public:
          virtual ~TermToBytesRefAttribute() { }
          virtual BytesRef& getBytesRef() = 0;
      };

      class BytesTermAttribute: public TermToBytesRefAttribute {
        public:
          virtual ~BytesTermAttribute() { }
          virtual void setBytesRef(BytesRef& bytes) = 0;
      };

      class FlagsAttribute: public Attribute {
        public:
          virtual ~FlagsAttribute() { }
          virtual int getFlags() = 0;
          virtual void setFlags(int flags) = 0;
      };

      class KeywordAttribute: public Attribute {
        public:
          virtual ~KeywordAttribute() { }
          virtual bool isKeyword() = 0;
          virtual void setKeyword(bool isKeyword) = 0;
      };

      class PayloadAttribute: public Attribute {
        public:
          virtual ~PayloadAttribute() { }
          virtual BytesRef& getPayload() = 0;
          virtual void setPayload(BytesRef& payload) = 0;
      };

      class PositionIncrementAttribute: public Attribute {
        public:
          virtual ~PositionIncrementAttribute() { }
          virtual void setPositionIncrement(int positionIncrement) = 0;
          virtual int getPositionIncrement() = 0;
      };

      class PositionLengthAttribute: public Attribute {
        public:
          virtual ~PositionLengthAttribute() { }
          virtual void setPositionLength(int positionLength) = 0;
          virtual int getPositionLength() = 0;
      };

      class TermFrequencyAttribute: public Attribute {
        public:
          virtual ~TermFrequencyAttribute() { }
          virtual void setTermFrequency(int termFrequency) = 0;
          virtual int getTermFrequency() = 0;
      };

      class TermToBytesRefAttribute: public Attribute {
        public:
          virtual ~TermToBytesRefAttribute() { }
          BytesRef& getBytesRef();
      };

      class TypeAttribute: public Attribute {
        public:
          virtual ~TypeAttribute() { }
          virtual string& type() = 0;
          virtual void setType(string& type) = 0;
      };

      class BoostAttribute: public Attribute {
        public:
          virtual ~BoostAttribute() { }
          virtual void setBoost(float boost) = 0;
          virtual float getBoost() = 0;
      };

      class MaxNonCompetitiveBoostAttribute: public Attribute {
        public:
          virtual ~MaxNonCompetitiveBoostAttribute() { }
          virtual void setMaxNonCompetitiveBoost(float maxNonCompetitiveBoost) = 0;
          virtual float getMaxNonCompetitiveBoost() = 0;
          virtual void setCompetitiveTerm(BytesRef& competitiveTerm) = 0;
          virtual BytesRef& getCompetitiveTerm() = 0;
      };

      class CharTermAttribute: public Attribute {
        public:
          virtual ~CharTermAttribute() { }
          virtual void copyBuffer(char[] buffer, int offset, int length) = 0;
          virtual char* buffer() = 0;
          virtual char* resizeBuffer(int newSize) = 0;
          virtual CharTermAttribute& setLength(int length) = 0;
          virtual CharTermAttribute& setEmpty() = 0;
          virtual CharTermAttribute& append(string& csq) = 0;
          virtual CharTermAttribute& append(string& csq, int start, int end) = 0;
          virtual CharTermAttribute& append(char c) = 0;
          virtual CharTermAttribute& append(string& s) = 0;
          virtual CharTermAttribute& append(CharTermAttribute& termAtt) = 0;
      };

    } // End of namespace util
  } // End of namespace core
} // End of namespace lucene

#endif
