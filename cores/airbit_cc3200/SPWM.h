//
// Created by 陶源 on 15/5/1.
//

#ifndef SPWM_H
#define SPWM_H

#include <IntervalTimer.h>
#include "Arduino.h"

#define __DEBUG_SPWM__

#define MAX_CHANNELS    10

#ifndef SYSCLK
#ifdef F_CPU
#define SYSCLK F_CPU
#else
#define SYSCLK 80000000UL;
#endif // F_CPU
#endif // SYSCLK

#define US_PER_SEC  (1000000UL)

//#define SPWM_DEFINE_OBJECT(MAX_CHANNELS) SPWM_DEFINE_OBJECT_FULL(SPWM, MAX_CHANNELS, Timer1)
//
//#define SPWM_DEFINE_OBJECT_FULL(NAME, MAX_CHANNELS, TIMER) \
//    void _##TIMER##_ISR_(void *params); \
//    SPWMClass<MAX_CHANNELS> NAME(TIMER, _##TIMER##_ISR_); \
//    void _##TIMER##_ISR_(void *params) { NAME.update(); } \


//typedef void (*SPWM_ISR)(void *);

struct SPWMChannel {
    uint8_t pin;
    uint8_t value;
    bool invert;
};

class SPWMClass {
public:
    SPWMClass();

    void begin() {
        begin(500);
    }

    void begin(long hertz, uint16_t range = 255);

    void write(uint8_t pin, uint8_t value, bool invert = false);

    size_t size() { return _num; }

    void update() __attribute__((always_inline));

    void printInterruptLoad(Print &printer = Serial);

protected:
    SPWMChannel _channels[MAX_CHANNELS];
    uint8_t _num;
    uint16_t _count;
    long _hertz;
    uint16_t _range;
    IntervalTimer _timer;

    void doWrite(uint8_t pin, bool state, bool invert);

    int find(uint8_t pin);

    void removeIndex(int idx);
};

extern SPWMClass SPWM;

#endif //SPWM_H
