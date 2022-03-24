#include <iostream>
#include "./lib/animal.h"
#include "../pluginloader.h"

class Animal : public PluginLoader
{
protected:
    using Create = void(*)();
    using Destroy = void(*)();
    using Speak = void(*)();
    using SetText = void(*)(const char*);
private:
    Create pCreate;
    Destroy pDestroy;
    Speak pSpeak;
    SetText pSetText;
public:
    Animal()
        :pCreate(nullptr),pDestroy(nullptr),
        pSpeak(nullptr),pSetText(nullptr)
    {
        if (PluginLoader::open("./lib/libanimal.so") == false) {
            return;
        }
        pCreate = PluginLoader::parse<Create>("animal_init");
        pDestroy = PluginLoader::parse<Destroy>("animal_deinit");
        pSetText = PluginLoader::parse<SetText>("animal_setText");
        pSpeak = PluginLoader::parse<Speak>("animal_speak");
        if (pCreate != nullptr) {
            pCreate();
        }
    }
    ~Animal()
    {
        pDestroy();
        PluginLoader::close();
    }

    void setText(const std::string &text_)
    {
        pSetText(text_.c_str());
        return;
    }

    void speak()
    {
        pSpeak();
        return;
    }
};

int main()
{
    Animal animal;
    animal.setText("seeing is believing.");
    animal.speak();
    return 0;
}
