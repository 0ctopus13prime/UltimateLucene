#ifndef LUCENE_CORE_UTIL_ATTRIBUTE_H_
#define LUCENE_CORE_UTIL_ATTRIBUTE_H_

#include <unordered_map>
#include <vector>
#include <unordered_set>
#include <functional>
#include <memory>
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
    virtual std::vector<size_t> Attributes() = 0;
    virtual void ShallowCopyTo(AttributeImpl& attr_impl) = 0;
    std::string ReflectAsString(const bool prepend_att_class);
};

class AttributeFactory {
  private:
    static std::unordered_map<size_t, std::function<AttributeImpl*(void)>> ATTR_IMPL_TABLE;

  public:
    AttributeFactory();
    virtual ~AttributeFactory() {}

    virtual AttributeImpl* CreateAttributeInstance(size_t attr_hash_code) = 0;

    static std::function<AttributeImpl*(void)> FindAttributeImplCtor(size_t attr_hash_code);

    template<typename ATTR_FACTORY, typename ATTR_IMPL>
    static AttributeFactory* GetStaticImplementation() {
      return new StaticImplementationAttributeFactory<ATTR_FACTORY, ATTR_IMPL>();
    }

    template<typename ATTR, typename ATTR_IMPL_CLASS>
    static void RegisterAttributeImpl() {
      ATTR_IMPL_TABLE[typeid(ATTR).hash_code()] = [](){
        return new ATTR_IMPL_CLASS();
      };
    }

  public:
    class DefaultAttributeFactory;

    template<typename ATTR_FACTORY, typename ATTR_IMPL>
    class StaticImplementationAttributeFactory;

  public:
    static DefaultAttributeFactory* DEFAULT_ATTRIBUTE_FACTORY;
};

class AttributeFactory::DefaultAttributeFactory: public AttributeFactory {
  public:
    DefaultAttributeFactory();
    virtual ~DefaultAttributeFactory();
    AttributeImpl* CreateAttributeInstance(size_t attr_hash_code) override;
};

template<typename ATTR_FACTORY, typename ATTR_IMPL>
class AttributeFactory::StaticImplementationAttributeFactory: public AttributeFactory {
  private:
    ATTR_FACTORY delegate;
    static std::unordered_set<size_t> DEFAULT_ATTR_NAMES;

  public:
    StaticImplementationAttributeFactory() {
    }
    ~StaticImplementationAttributeFactory() {
    }
    AttributeImpl* CreateAttributeInstance(size_t attr_hash_code) override {
      if(DEFAULT_ATTR_NAMES.find(attr_hash_code) != DEFAULT_ATTR_NAMES.end()) {
        return new ATTR_IMPL();
      } else {
        return delegate.CreateAttributeInstance(attr_hash_code);
      }
    }
};

template<typename ATTR_FACTORY, typename ATTR_IMPL>
std::unordered_set<size_t> AttributeFactory::StaticImplementationAttributeFactory<ATTR_FACTORY, ATTR_IMPL>::DEFAULT_ATTR_NAMES;

class AttributeSource {
  public:
    class State final {
      public:
        std::shared_ptr<AttributeImpl> attribute;
        State* next;

      public:
        State();
        State(const State& other);
        ~State();
        State& operator=(const State& other);
    };

  private:
    std::shared_ptr<State> current_state;
    bool current_state_dirty;
    std::unordered_map<size_t, std::shared_ptr<AttributeImpl>> attributes;
    std::unordered_map<size_t, std::shared_ptr<AttributeImpl>> attribute_impls;
    std::shared_ptr<AttributeFactory> factory;

  private:
    State* GetCurrentState();

  public:
    AttributeSource();
    AttributeSource(const AttributeSource& other);
    AttributeSource(AttributeFactory* factory);
    AttributeFactory& GetAttributeFactory() const;

    void AddAttributeImpl(AttributeImpl* attr_impl);

    template <typename ATTR>
    std::shared_ptr<ATTR> AddAttribute() {
      auto attr_it = attributes.find(typeid(ATTR).hash_code());
      if(attr_it == attribute_impls.end()) {
        AddAttributeImpl(factory->CreateAttributeInstance(typeid(ATTR).hash_code()));
        attr_it = attributes.find(typeid(ATTR).hash_code());
      }

      return std::dynamic_pointer_cast<ATTR>(attr_it->second);
    }

    bool HasAttributes();

    template<typename ATTR>
    bool HasAttribute() {
      return (attributes.find(typeid(ATTR).hash_code()) != attributes.end());
    }

    void ClearAttributes();
    void EndAttributes();
    void RemoveAllAttributes();
    State CaptureState();
    void RestoreState(State state);
    std::string ReflectAsString(const bool prepend_att);
    void ReflectWith(AttributeReflector& reflector);
    AttributeSource& operator=(const AttributeSource& other);
};

}}} // End of namespace

#endif
