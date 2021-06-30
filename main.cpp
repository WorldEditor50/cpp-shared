#include "singleton.hpp"
#include "factory.hpp"
#include "observer.hpp"
#include <string>
#include <iostream>

class Animal
{
public:
    Animal(){}
    virtual ~Animal(){};
    virtual void speak() = 0;
};

class Bird : public Animal
{
public:
    Bird(){}
    ~Bird(){}
    void speak() override {std::cout<<"bird speak."<<std::endl;}
};

class Fish : public Animal
{
public:
    Fish(){}
    ~Fish(){}
    void speak() override {std::cout<<"fish speak."<<std::endl;}
};

class Cat : public Animal
{
public:
    Cat(){}
    ~Cat(){}
    void speak() override {std::cout<<"cat speak."<<std::endl;}
};

class Dragon : public Animal
{
public:
    Dragon(){}
    ~Dragon(){}
    void speak() override {std::cout<<"dragon speak."<<std::endl;}
};

class Counter
{
public:
    int num;
public:
    Counter():num(0){}
    ~Counter(){}
    void tiktok(){num++;std::cout<<num<<std::endl;}
};

class ConcreteObserver : public Observer
{
public:
    ConcreteObserver(Subject *sub, const std::string & name)
    {
        subject = sub;
        name_ = name;
    }
    void updtae() override
    {
        std::cout<<name_<<std::endl;
        return;
    }
private:
    Subject *subject;
    std::string name_;
};

class ConcreteSubject : public Subject
{
public:
    ConcreteSubject(){}
private:
    std::string name;
};

int main(int argc, char *argv[])
{
    /* singleton */
    Singleton<Counter>::instance()->tiktok();
    Singleton<Counter>::instance()->tiktok();
    Singleton<Counter>::instance()->tiktok();
    /* abstract factory */
    FactoryRegister<Animal, Bird> birdRegister("bird");
    FactoryRegister<Animal, Fish> fishRegister("fish");
    FactoryRegister<Animal, Cat> catRegister("cat");
    FactoryRegister<Animal, Dragon> dragonRegister("dragon");
    Factory<Animal>::instance().get("cat")->speak();
    Factory<Animal>::instance().get("bird")->speak();
    Factory<Animal>::instance().get("fish")->speak();
    Factory<Animal>::instance().get("dragon")->speak();
    /* observer */
    Subject *subject = Singleton<Subject>::instance().get();
    std::shared_ptr<ConcreteObserver> observer1;
    std::shared_ptr<ConcreteObserver> observer2;
    std::shared_ptr<ConcreteObserver> observer3;
    observer1.reset(new ConcreteObserver(subject, "ob1"));
    observer2.reset(new ConcreteObserver(subject, "ob2"));
    observer3.reset(new ConcreteObserver(subject, "ob3"));
    Singleton<Subject>::instance().get()->attach(observer1.get());
    Singleton<Subject>::instance().get()->attach(observer2.get());
    Singleton<Subject>::instance().get()->attach(observer3.get());
    Singleton<Subject>::instance().get()->notify();
    return 0;
}
