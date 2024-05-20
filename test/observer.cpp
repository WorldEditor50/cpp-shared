#include <string>
#include <map>
#include <iostream>

class Receiver
{
public:
    Receiver(){}
    virtual ~Receiver(){}
    virtual void update(const std::string &message) = 0;
};

class Sender
{
private:
    std::map<std::string, Receiver*> receivers;
    Sender(){}
    Sender(const Sender&) = delete;
    Sender& operator=(const Sender) = delete;
public:
    ~Sender(){}
    inline static Sender& instance()
    {
        static Sender sender;
        return sender;
    }
    void attach(const std::string &name, Receiver* receiver)
    {
        receivers.insert(std::pair<std::string, Receiver*>(name, receiver));
        return;
    }
    void dettach(const std::string &name)
    {
        receivers.erase(name);
        return;
    }
    void notify(const std::string &message)
    {
        for (auto &x : receivers) {
            x.second->update(message);
        }
        return;
    }

};

class Object1 : public Receiver
{
public:
    Object1()
    {
        Sender::instance().attach("object1", this);
    }
    virtual void update(const std::string &message) override
    {
        std::cout<<"object1:"<<message<<std::endl;
        return;
    }
};

class Object2 : public Receiver
{
public:
    Object2()
    {
        Sender::instance().attach("object2", this);
    }
    virtual void update(const std::string &message) override
    {
        std::cout<<"object2:"<<message<<std::endl;
        return;
    }
};

class Object3 : public Receiver
{
public:
    Object3()
    {
        Sender::instance().attach("object3", this);
    }
    virtual void update(const std::string &message) override
    {
        std::cout<<"object3:"<<message<<std::endl;
        return;
    }
};
int main()
{
    Object1 obj1;
    Object2 obj2;
    Object3 obj3;
    Sender::instance().notify("hello");
    return 0;
}
