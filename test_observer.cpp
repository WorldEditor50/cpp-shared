#include "observer.hpp"
#include <string>

class ConcreteObserver : public Observer
{
public:
    ConcreteObserver(Subject *sub)
    {
        subject = sub;
    }
    void updtae() override
    {

    }
private:
    Subject *subject;
};

class ConcreteSubject : public Subject
{
public:
    ConcreteSubject(){}
private:
    std::string name;
};
