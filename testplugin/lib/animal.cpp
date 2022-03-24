#include "animal_impl.h"
#include "animal.h"
#include <iostream>

static AnimalImpl* impl = nullptr;

void animal_init()
{
    if (impl != nullptr) {
        return;
    }
    impl = new AnimalImpl;
    return;
}
void animal_deinit()
{
    if (impl == nullptr) {
        return;
    }
    delete impl;
    impl = nullptr;
    return;
}
void animal_setText(const char* text)
{
    if (impl == nullptr) {
        return;
    }
    impl->setText(text);
    return;
}
void animal_speak()
{
    if (impl == nullptr) {
        return;
    }
    impl->speak();
    return;
}

