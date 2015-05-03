#include "Arduino.h"

#include "Watchdog.h"

WatchdogClass Watchdog;

#ifdef __CC3200R1M1RGC__

#include "inc/hw_ints.h"
#include "inc/hw_types.h"

#include "driverlib/prcm.h"
#include "driverlib/wdt.h"

#ifndef SYSCLK
#ifdef F_CPU
#define SYSCLK           F_CPU
#else
#define SYSCLK           80000000
#endif // F_CPU
#endif // SYSCLK

#ifndef MILLISECONDS_TO_TICKS
#define MILLISECONDS_TO_TICKS(ms)    ((SYSCLK / 1000) * (ms))
#endif

void watchdog_enable(WatchdogCallback callback, unsigned long timeout) {
   //
    // Enable the peripherals used by this example.
    //
    MAP_PRCMPeripheralClkEnable(PRCM_WDT, PRCM_RUN_MODE_CLK);

    //
    // Unlock to be able to configure the registers
    //
    MAP_WatchdogUnlock(WDT_BASE);

    if (callback != NULL) {
        MAP_IntPrioritySet(INT_WDT, INT_PRIORITY_LVL_1);
        MAP_WatchdogIntRegister(WDT_BASE, callback);
    }

    //
    // Set the watchdog timer reload value
    //
    MAP_WatchdogReloadSet(WDT_BASE, MILLISECONDS_TO_TICKS(timeout));

    //
    // Start the timer. Once the timer is started, it cannot be disable.
    //
    MAP_WatchdogEnable(WDT_BASE);
}

void watchdog_disable() {
    //
    // Unlock to be able to configure the registers
    //
    MAP_WatchdogUnlock(WDT_BASE);

    //
    // Disable stalling of the watchdog timer during debug events
    //
    MAP_WatchdogStallDisable(WDT_BASE);

    //
    // Clear the interrupt
    //
    MAP_WatchdogIntClear(WDT_BASE);

    //
    // Unregister the interrupt
    //
    MAP_WatchdogIntUnregister(WDT_BASE);
}

void watchdog_reset() {
	MAP_WatchdogIntClear(WDT_BASE);

}

#else

void watchdog_enable(WatchdogCallback callback, unsigned long timeout) {}

void watchdog_disable() {}

void watchdog_reset() {}

#endif

void WatchdogClass::enable(WatchdogCallback callback, unsigned long timeout) {
    watchdog_enable(callback, timeout);
}

void WatchdogClass::enable(unsigned long timeout) {
    watchdog_enable(NULL, timeout);
}

void WatchdogClass::disable() {
    watchdog_disable();
}

void WatchdogClass::reset() {
    watchdog_reset();
}
