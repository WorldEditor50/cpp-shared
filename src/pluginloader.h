#ifndef PLUGIN_LOADER_H
#define PLUGIN_LOADER_H
#ifdef WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif
#include <string>
#include <iostream>

#ifdef WIN32

class PluginLoader
{
private:
    HMODULE hModule;
public:
    PluginLoader():hModule(nullptr){}
    ~PluginLoader()
    {
        if (!hModule) {
            close();
        }
    }
    bool open(const std::string &fileName)
    {
        if (fileName.empty()) {
            std::cout<<"empty filename."<<std::endl;
            return false;
        }
        if (hModule != nullptr) {
            std::cout<<"library has been opened."<<std::endl;
            return true;
        }
        hModule = LoadLibraryA(fileName.c_str());
        if (!hModule) {
            std::cout<<"failed to open library."<<std::endl;
            return false;
        }
        return true;
    }

    void close()
    {
        int ret = FreeLibrary(hModule);
        if (ret != 0) {
            std::cout<<"failed to close library."<<std::endl;
            return;
        }
        hModule = nullptr;
        return;
    }

    template<typename TFunc>
    TFunc parse(const std::string &funcName)
    {
        TFunc func;
        if (funcName.empty()) {
            return func;
        }
        func = TFunc(GetProcAddress(hModule, funcName.c_str()));
        if (!func) {
            std::cout<<"failed to load function."<<std::endl;
            return func;
        }
        return func;
    }
};


#else
class PluginLoader
{
private:
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

#endif
#endif // PLUGIN_LOADER_H
