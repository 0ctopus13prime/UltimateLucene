#ifndef LUCENE_CORE_UTIL_ATTRIBUTE_H_
#define LUCENE_CORE_UTIL_ATTRIBUTE_H_

#include <unordered_map>
#include <unordered_set>
#include <functional>
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
    virtual void Clear() = 0;
    virtual void End();
    std::string ReflectAsString(const bool prepend_att_class);
};

class AttributeFactory {
  private:
    static std::unordered_map<std::string, std::function<AttributeImpl*(void)>> ATTR_IMPL_TABLE;

  public:
    AttributeFactory();
    virtual ~AttributeFactory() {}
    virtual AttributeImpl* CreateAttributeInstance(const std::string& attr_name) = 0;
    static std::function<AttributeImpl*(void)> FindAttributeImplCtor(const std::string& attr_name);

    template<typename ATTR_FACTORY, typename ATTR_IMPL>
    static AttributeFactory* GetStaticImplementation() {
      return new StaticImplementationAttributeFactory<ATTR_FACTORY, ATTR_IMPL>();
    }

    template<typename ATTR_IMPL_CLASS>
    static void RegisterAttributeImpl(const char* name) {
      ATTR_IMPL_TABLE[name] = [](){
        return new ATTR_IMPL_CLASS();
      };
    }

  public:
    class DefaultAttributeImpl;

    template<typename ATTR_FACTORY, typename ATTR_IMPL>
    class StaticImplementationAttributeFactory;
};

class AttributeFactory::DefaultAttributeImpl: public AttributeFactory {
  public:
    DefaultAttributeImpl();
    virtual ~DefaultAttributeImpl();
    AttributeImpl* CreateAttributeInstance(const std::string& attr_name) override;
};

template<typename ATTR_FACTORY, typename ATTR_IMPL>
class AttributeFactory::StaticImplementationAttributeFactory: public AttributeFactory {
  private:
    ATTR_FACTORY delegate;
    static std::unordered_set<std::string> DEFAULT_ATTR_NAMES;

  public:
    StaticImplementationAttributeFactory() {
    }
    ~StaticImplementationAttributeFactory() {
    }
    AttributeImpl* CreateAttributeInstance(const std::string& attr_name) override {
      if(DEFAULT_ATTR_NAMES.find(attr_name) != DEFAULT_ATTR_NAMES.end()) {
        return new ATTR_IMPL();
      } else {
        return delegate.CreateAttributeInstance(attr_name);
      }
    }
};

class AttributeSource {
  public:
    virtual AttributeImpl* CreateAttributeInstance(std::string class_name) = 0;

  public:
    class State final {
      public:
        AttributeImpl* attribute;
        State* next;

      public:
        State();
        State(const State& other);
        ~State();
        State& operator=(const State& other);
    };
};

}}} // End of namespace

#endif
