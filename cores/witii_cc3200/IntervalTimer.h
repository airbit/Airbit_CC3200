//
// Created by 陶源 on 15/5/3.
//

#ifndef INTERVALTIMER_H
#define INTERVALTIMER_H

#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

class IntervalTimer {
private:
    typedef void (*ISR)(void *params);
    enum {TIMER_OFF, TIMER_PIT};
    struct PITISR {
        ISR isr;
        void *params;
        void call() { isr(params); }
    };
#if defined(__CC3200R1M1RGC__)
    static const uint8_t NUM_PIT = 4;
#endif
    static const uint32_t MAX_PERIOD = (uint32_t) (UINT32_MAX / (F_CPU / 1000000.0));
    static bool Enabled;
    static bool Used[NUM_PIT];
    static PITISR ISRS[NUM_PIT];

    bool _status;
    uint8_t _id;
    uint8_t _priority;
    ISR _isr;
    void *_params;

    void _enable();
    void _disable();

    bool _allocate(uint32_t newValue);
    void _start(uint32_t newValue);
    void _stop();

    bool beginPeriod(ISR newISR, void *params, uint32_t period);
public:
    IntervalTimer() { _status = TIMER_OFF; _priority = 1; }
    ~IntervalTimer() { end(); }

    bool begin(ISR newISR, unsigned int newPeriod) {
        return begin(newISR, NULL, newPeriod);
    }
    bool begin(ISR newISR, int newPeriod) {
        return begin(newISR, NULL, newPeriod);
    }
    bool begin(ISR newISR, unsigned long newPeriod) {
        return begin(newISR, NULL, newPeriod);
    }
    bool begin(ISR newISR, long newPeriod) {
        return begin(newISR, NULL, newPeriod);
    }

    bool begin(ISR newISR, void *params, unsigned int newPeriod) {
        if (newPeriod == 0 || newPeriod > MAX_PERIOD) return false;
        return begin(newISR, params, (unsigned long)newPeriod);
    }
    bool begin(ISR newISR, void *params, int newPeriod) {
        if (newPeriod < 0) return false;
        return begin(newISR, params, (unsigned int)newPeriod);
    }
    bool begin(ISR newISR, void *params, unsigned long newPeriod) {
        if (newPeriod == 0 || newPeriod > MAX_PERIOD) return false;
        return beginPeriod(newISR, params, newPeriod);
    }
    bool begin(ISR newISR, void *params, long newPeriod) {
        return begin(newISR, params, (int)newPeriod);
    }

    void end();
    void enable();
    void disable();
    void period(unsigned long newPeriod);
    void priority(uint8_t n);

    bool isActive() { return _status == TIMER_PIT; }

#if defined(__CC3200R1M1RGC__)
    friend void pit0_isr();
    friend void pit1_isr();
    friend void pit2_isr();
    friend void pit3_isr();
#endif
};

#ifdef __cplusplus
}
#endif


#endif //INTERVALTIMER_H
