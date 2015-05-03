//
// Created by 陶源 on 15/5/3.
//

#include <inc/hw_types.h>
#include <inc/hw_ints.h>
#include <driverlib/prcm.h>
#include <driverlib/timer.h>
#include "timer_if.h"
#include "IntervalTimer.h"


// ------------------------------------------------------------
// static class variables need to be reiterated here before use
// ------------------------------------------------------------
bool IntervalTimer::Enabled;
bool IntervalTimer::Used[];
IntervalTimer::PITISR IntervalTimer::ISRS[];

typedef void (*TimerIntHandler)(void);

#ifdef __CC3200R1M1RGC__

void pit0_isr() {
    Timer_IF_InterruptClear(TIMERA0_BASE);
    IntervalTimer::ISRS[0].call();
}

void pit1_isr() {
    Timer_IF_InterruptClear(TIMERA1_BASE);
    IntervalTimer::ISRS[1].call();
}

void pit2_isr() {
    Timer_IF_InterruptClear(TIMERA2_BASE);
    IntervalTimer::ISRS[2].call();
}

void pit3_isr() {
    Timer_IF_InterruptClear(TIMERA3_BASE);
    IntervalTimer::ISRS[3].call();
}

#endif

static uint32_t PRCM_TIMER[] = {PRCM_TIMERA0, PRCM_TIMERA1, PRCM_TIMERA2, PRCM_TIMERA3};
static uint32_t TIMER_BASE[] = {TIMERA0_BASE, TIMERA1_BASE, TIMERA2_BASE, TIMERA3_BASE};
static uint8_t PERIPHERAL_INT[] = {INT_TIMERA0A, INT_TIMERA1A, INT_TIMERA2A, INT_TIMERA3A};
static uint8_t PRIORITY_LVL[] = {INT_PRIORITY_LVL_0, INT_PRIORITY_LVL_1, INT_PRIORITY_LVL_2, INT_PRIORITY_LVL_3,
                                 INT_PRIORITY_LVL_4, INT_PRIORITY_LVL_5, INT_PRIORITY_LVL_6, INT_PRIORITY_LVL_7};
TimerIntHandler ISR_HANDLER[] = {pit0_isr, pit1_isr, pit2_isr, pit3_isr};

// ------------------------------------------------------------
// this function init and start the timer, using the specified
// function as a callback and the period provided. must be passed
// the name of a function taking no arguments and returning void.
// make sure this function can complete within the time allowed.
// attempts to allocate a timer using available resources,
// returning true on success or false in case of failure.
// period is micro seconds
// ------------------------------------------------------------
bool IntervalTimer::beginPeriod(IntervalTimer::ISR newISR, void *params, uint32_t period) {
    // if this interval timer is already running, stop it
    if (_status == TIMER_PIT) {
        _stop();
        _status = TIMER_OFF;
    }
    // store callback pointer
    _isr = newISR;
    _params = params;

    // attempt to allocate this timer
    if (_allocate(period)) _status = TIMER_PIT;
    else _status = TIMER_OFF;

    // check for success and return
    return _status != TIMER_OFF;
}

// ------------------------------------------------------------
// stop the timer if it's currently running, using its status
// to determine what hardware resources the timer may be using
// ------------------------------------------------------------
void IntervalTimer::end() {
    if (_status == TIMER_PIT) _stop();
    _status = TIMER_OFF;
}

void IntervalTimer::enable() {
    if (TIMER_PIT != _status) return;
    _enable();
}

void IntervalTimer::disable() {
    if (TIMER_PIT != _status) return;
    _disable();
}

void IntervalTimer::period(unsigned long newPeriod) {
    if (TIMER_PIT != _status) return;

    Timer_IF_ReLoad(TIMER_BASE[_id], TIMER_BOTH, newPeriod);
}

void IntervalTimer::priority(uint8_t n) {
    _priority = n;
    if (TIMER_PIT == _status) {
        MAP_IntPrioritySet(PERIPHERAL_INT[_id], PRIORITY_LVL[_priority]);
    }
}

void IntervalTimer::_enable() {
    Timer_IF_IntSetup(TIMER_BASE[_id], TIMER_BOTH, ISR_HANDLER[_id]);
    MAP_IntPrioritySet(PERIPHERAL_INT[_id], PRIORITY_LVL[_priority]);
}

void IntervalTimer::_disable() {
    // clear interrupt
    Timer_IF_DeInit(TIMER_BASE[_id], TIMER_BOTH);
}

// ------------------------------------------------------------
// enables the PIT clock if not already enabled, then checks to
// see if any PITs are available for use. if one is available,
// it's initialized and started with the specified value, and
// the function returns true, otherwise it returns false
// ------------------------------------------------------------
bool IntervalTimer::_allocate(uint32_t newValue) {
    // check for an available PIT, and if so, start it
    for (uint8_t id = 0; id < NUM_PIT; id++) {
        if (!Used[id]) {
            _id = id;
            _start(newValue);
            Used[id] = true;
            return true;
        }
    }

    // no PIT available
    return false;
}

void IntervalTimer::_start(uint32_t newValue) {
    // point to the correct PIT ISR
    ISRS[_id].isr = _isr;
    ISRS[_id].params = _params;

    // init timer and enable interrupt
    Timer_IF_Init(PRCM_TIMER[_id], TIMER_BASE[_id], TIMER_CFG_PERIODIC, TIMER_BOTH, 0);
    _enable();
    Timer_IF_Start(TIMER_BASE[_id], TIMER_BOTH, newValue);
}

void IntervalTimer::_stop() {

    // free PIT for future use
    Used[_id] = false;

    _disable();
    // none used, disable PIT clock
    Timer_IF_Stop(TIMER_BASE[_id], TIMER_BOTH);

}
