#ifndef PLUGIN_LOADER_H
#define PLUGIN_LOADER_H
#include <dlfcn.h>
#include <string>
#include <iostream>

class PluginLoader
{
protected:
    void* handle;
public:
    PluginLoader():handle(nullptr){}
    bool open(const std::string &fileName)
    {
        if (fileName.empty()) {
            std::cout<<"empty filename."<<std::endl;
            return false;
        }
        if (handle != nullptr) {
            std::cout<<"library has been opened."<<std::endl;
            return false;
        }
        handle = dlopen(fileName.c_str(), RTLD_LAZY);
        if (handle == nullptr) {
            std::cout<<"failed to open library."<<std::endl;
            return false;
        }
        return true;
    }

    void close()
    {
        if (handle == nullptr) {
            return;
        }
        int ret = dlclose(handle);
        if (ret != 0) {
            std::cout<<"failed to close library."<<std::endl;
            return;
        }
        handle = nullptr;
        return;
    }

    template<typename TFunc>
    TFunc parse(const std::string &funcName)
    {
        TFunc func;
        if (funcName.empty()) {
            return func;
        }
        func = TFunc(dlsym(handle, funcName.c_str()));
        if (func == nullptr) {
            std::cout<<"failed to load function."<<std::endl;
            return func;
        }
        return func;
    }
};

#endif // PLUGIN_LOADER_H
