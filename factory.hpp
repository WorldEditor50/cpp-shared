#ifndef FACTORY_HPP
#define FACTORY_HPP

#include <map>
#include <string>

template <typename BasicType>
class AbstractFactory
{
public:
    virtual BasicType* instance() = 0;
protected:
    AbstractFactory(){}
    virtual ~AbstractFactory(){}
private:
    AbstractFactory(const AbstractFactory&) = delete;
    const AbstractFactory& operator=(const AbstractFactory&) = delete;
};

template <typename BasicType>
class Factory
{
private:
    std::map<std::string, AbstractFactory<BasicType>*> instanceMap;
public:
    static Factory<BasicType>& instance()
    {
        static Factory<BasicType> factory;
        return factory;
    }
    void registerType(AbstractFactory<BasicType>* typeInstance, const std::string& name)
    {
        instanceMap[name] = typeInstance;
        return;
    }
    BasicType* get(const std::string& name)
    {
        return instanceMap[name]->instance();
    }
private:
    Factory(const Factory&) = delete;
    const Factory& operator=(const Factory&) = delete;
};

template <typename BasicType, typename Type>
class FactoryRegister : public AbstractFactory<BasicType>
{
public:
    explicit FactoryRegister(const std::string& name)
    {
        Factory<BasicType>::instance().registerType(this, name);
    }
    BasicType* instance()
    {
        static_assert(std::is_base_of<BasicType, Type>::value, "Type is not based on BasicType");
        return new Type;
    }
};
#endif // FACTORY_HPP
