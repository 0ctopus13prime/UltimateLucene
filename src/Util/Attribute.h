#ifndef LUCENE_CORE_UTIL_ATTRIBUTE_H_
#define LUCENE_CORE_UTIL_ATTRIBUTE_H_

#include <string>

namespace lucene { namespace core { namespace util {

class Attribute {
  public:
    virtual ~Attribute() {}
};

class AttributeReflector {
  public:
    virtual ~AttributeReflector() {}
    virtual void Reflect() = 0;
};

class AttributeImpl: public Attribute {
  public:
    virtual ~AttributeImpl() { }
    virtual void ReflectWith(AttributeReflector& reflector) = 0;
    virtual void CopyTo(AttributeImpl& target) = 0;
    virtual void Clear() = 0;
    void End();
    std::string ReflectAsString(const bool prepend_att_class);
};

class AttributeFactory {
  public:
    virtual ~AttributeFactory() {}
};

class AttributeSource {

};

}}} // End of namespace

#endif
