#include <stdbool.h>

#ifndef TIMER_H 
#define TIMER_H
void setup_timer();
bool get_interrupt_call();
void set_interrupt_call(bool state);
#endif