#ifndef ANIMAI_IMPL_H
#define ANIMAI_IMPL_H
#include <string>
#include <iostream>

class AnimalImpl
{
protected:
    std::string text;
public:
    AnimalImpl(){}
    void setText(const std::string &text_){text = text_;}
    void speak(){std::cout<<text<<std::endl;}
};

#endif // ANIMAI_IMPL_H
