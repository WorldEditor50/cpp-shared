#ifndef INTERFACE_HPP
#define INTERFACE_HPP
#include <functional>
#include <map>
#include <string>
#include <memory>
#include <tuple>

template<typename ...T>
class Interface : public T...{};

class Method {};

template<typename TR, typename ...TArgs>
class MethodImpl : public Method
{
public:
    explicit MethodImpl(const std::string &name_, const std::function<TR(TArgs...)>& func_):name(name_),func(func_){}
    std::string name;
    std::function<TR(TArgs...)> func;
};

class Interfaces
{
public:
    std::shared_ptr<Method> operator[](const std::string &name)
    {
        if (methods.find(name) == methods.end()) {
            return nullptr;
        }
        return methods[name];
    }

    template<typename TR, typename ...TArgs>
    void registerMethod(const std::string &name, const std::function<TR(TArgs...)>& func)
    {
        methods[name] = std::shared_ptr<Method>(new MethodImpl<TR, TArgs...>(name, func));
        return;
    }

    template<typename TR, typename ...TArgs>
    TR invoke(const std::string &name, TArgs&& ...args)
    {
        return static_cast<MethodImpl<TR, TArgs...>*>(methods[name].get())->func(std::forward<TArgs>(args)...);
    }

    void registerMethod(const std::string &name, const std::shared_ptr<Method> &method)
    {
        methods[name] = method;
        return;
    }

    template<typename Impl>
    auto invoke(const std::string &name) -> typename Impl::ReturnType
    {
        return static_cast<Impl*>(methods[name].get())->exec();
    }
private:
    std::map<std::string, std::shared_ptr<Method> > methods;
};

#endif // INTERFACE_HPP
