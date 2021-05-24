#ifndef OBSERVER_HPP
#define OBSERVER_HPP

#include <vector>

class Observer
{
public:
    Observer(){}
    virtual ~Observer(){}
    virtual void updtae() = 0;
};

class Subject
{
private:
    std::vector<Observer*> observerList;
public:
    void attach(Observer* observer)
    {
        if (observer != nullptr) {
            observerList.push_back(observer);
        }
        return;
    }
    void detach(Observer* observer)
    {
        for (auto it = observerList.begin(); it != observerList.end(); it++) {
            if (*it == observer) {
                it = observerList.erase(it);
                break;
            }
        }
        return;
    }
    void notify()
    {
        for (auto x : observerList) {
            x->updtae();
        }
        return;
    }
};
#endif // OBSERVER_HPP
