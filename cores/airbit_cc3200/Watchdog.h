#ifndef WATCHDOG_H
#define WATCHDOG_H

typedef void (*WatchdogCallback)();

class WatchdogClass {
public:
    void enable(WatchdogCallback callback, unsigned long timeout);
    void enable(unsigned long timeout);
    void disable();
    void reset();
};

extern WatchdogClass Watchdog;

#endif