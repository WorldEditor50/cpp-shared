#include <string>
#include <map>
#include <memory>
#include <iostream>

template <typename Base>
class Factory
{
private:
    std::map<std::string, std::shared_ptr<Base> > typeContainer;
private:
    Factory(){}
public:
    template<typename T>
    void registerType(const std::string &name, std::shared_ptr<T> typeInstance)
    {
        static_assert(std::is_base_of<Base, T>::value, "Type is not based on BasicType");
        typeContainer.insert(std::pair<std::string, std::shared_ptr<Base> >(name, typeInstance));
        return;
    }
    inline std::shared_ptr<Base> get(const std::string &name)
    {
        return typeContainer[name];
    }
    inline static Factory& instance()
    {
        static Factory factory;
        return factory;
    }
};


class Gun
{
public:
    Gun(){}
    virtual ~Gun(){}
    virtual void load() = 0;
    virtual void aim() = 0;
    virtual void shoot() = 0;
};


class AK47 : public Gun
{
public:
    AK47(){}
    virtual void load() override
    {
        std::cout<<"ak47 load"<<std::endl;
        return;
    }
    virtual void aim() override
    {
        std::cout<<"ak47 aim"<<std::endl;
        return;
    }
    virtual void shoot() override
    {
        std::cout<<"ak47 shoot"<<std::endl;
        return;
    }
};


class M16 : public Gun
{
public:
    M16(){}
    virtual void load() override
    {
        std::cout<<"M16 load"<<std::endl;
        return;
    }
    virtual void aim() override
    {
        std::cout<<"M16 aim"<<std::endl;
        return;
    }
    virtual void shoot() override
    {
        std::cout<<"M16 shoot"<<std::endl;
        return;
    }
};

class AUG : public Gun
{
public:
    AUG(){}
    virtual void load() override
    {
        std::cout<<"AUG load"<<std::endl;
        return;
    }
    virtual void aim() override
    {
        std::cout<<"AUG aim"<<std::endl;
        return;
    }
    virtual void shoot() override
    {
        std::cout<<"AUG shoot"<<std::endl;
        return;
    }
};

using GunFactory = Factory<Gun>;

int main()
{
    GunFactory::instance().registerType("ak47", std::make_shared<AK47>());
    GunFactory::instance().registerType("m16", std::make_shared<M16>());
    GunFactory::instance().registerType("aug", std::make_shared<AUG>());
    GunFactory::instance().get("ak47")->shoot();
    GunFactory::instance().get("m16")->shoot();
    GunFactory::instance().get("aug")->shoot();
    return 0;
}
