#ifndef ANIMAL_H
#define ANIMAL_H

extern "C" {

void animal_init();
void animal_deinit();
void animal_setText(const char* text);
void animal_speak();

}
#endif // ANIMAL_H
