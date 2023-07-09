#include "singleton.hpp"
#include "factory.hpp"
#include "observer.hpp"
#include <string>
#include <iostream>
#include <cstring>
#include "log.hpp"
#include "LazyAlloctor.hpp"
#include "tcppipe.hpp"
#include "LazyFsm.h"
#include "interface.hpp"

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
        std::cout<<"notify: "<<name_<<std::endl;
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
template <typename T>
inline void append(T t, std::string &dst) { dst += std::to_string(t);}
inline void append(const std::string& t, std::string &dst){ dst += t;}
inline void append(const char* t, std::string &dst) { dst += t;}
template <typename ...T>
inline std::string append(const T& ...t)
{
    std::string dst;
    int argv[] = {(append(t, dst), 0)...};
    return dst;
}

int add(int x, int y, int z)
{
    std::cout<<x + y + z<<std::endl;
    return x + y + z;
}
void show(const std::string &text)
{
    std::cout<<text<<std::endl;
}
class A
{
public:
    void show(const std::string &text)
    {
        std::cout<<"A:"<<text<<std::endl;
    }
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
    std::shared_ptr<ConcreteObserver> observer1 = std::make_shared<ConcreteObserver>(subject, "ob1");
    std::shared_ptr<ConcreteObserver> observer2 = std::make_shared<ConcreteObserver>(subject, "ob2");
    std::shared_ptr<ConcreteObserver> observer3 = std::make_shared<ConcreteObserver>(subject, "ob3");
    Singleton<Subject>::instance()->attach(observer1.get());
    Singleton<Subject>::instance()->attach(observer2.get());
    Singleton<Subject>::instance()->attach(observer3.get());
    Singleton<Subject>::instance()->notify();
    /* log */
    LOG_INFO("hello");
    /* string append */
    std::string result;
    result = append(123, "hello", 256, "great");
    std::cout<<result<<std::endl;
    std::cout<<(0, 9)<<std::endl;
    /* allocator */
    LazyAllocator<char> alloc;
    char* ptr1 = alloc.pop(32);
    strcpy(ptr1, "i am invetiable.");
    std::cout<<ptr1<<std::endl;
    alloc.push(32, ptr1);
    char* ptr2 = alloc.pop(16);
    strcpy(ptr2, "hello");
    std::cout<<ptr2<<std::endl;
    alloc.push(16, ptr2);
    /* size */
    std::size_t size_ = 1025;
    if (size_ & 0x3ff) {
        size_ = ((size_ >> 10) + 1) << 10;
    }
    std::cout<<"size: "<<size_<<std::endl;
    /* invoke */
    Interfaces interfaces;
    interfaces.registerMethod("A-Show", std::function<void(A, const std::string&)>(&A::show));
    interfaces.registerMethod("show", std::function<void(const std::string&)>(show));
    interfaces.registerMethod("add", std::function<int(int, int, int)>(add));
    interfaces.invoke<void, const std::string&>("show", "listening talker makes me thirsty.");
    interfaces.invoke<int, int, int, int>("add", 1, 2, 3);
    interfaces.invoke<void, const std::string&>("A-Show", "i am batman.");
    return 0;
//    /* fsm */
//    LazyFSM fsm;
//    /* tcp-pipe */
//    TcpPipe<UnixSocket> pipe1;
//    pipe1.push("hello");
//    TcpPipe<UnixSocket> pipe2;
//    pipe2.push("i am inevitable.");
    return 0;
}
