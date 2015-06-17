#include "SPWM.h"

SPWMClass SPWM;

void spwm_isr(void *params) {
    if (params == NULL) return;
    SPWMClass *spwm = (SPWMClass *) params;
    spwm->update();
}


SPWMClass::SPWMClass() {
    _hertz = 500;
    _range = 255;
    _count = 0;
    _num = 0;
    memset(_channels, 0, sizeof(_channels));
}

void SPWMClass::begin(long hertz, uint16_t range) {
    _hertz = hertz;
    _range = range;
    _count = 0;
    _num = 0;
    memset(_channels, 0, sizeof(_channels));

    _timer.begin(spwm_isr, this, (US_PER_SEC - (hertz * (_range + 1)) / 2) / (hertz * (_range + 1)));
}

void SPWMClass::write(uint8_t pin, uint8_t value, bool invert) {
    bool out_of_range = value <= 0 || value >= _range;

    pinMode(pin, OUTPUT);

    if (out_of_range) {
        doWrite(pin, (uint8_t) (value > 0), false); // ignore invert
    }

    _timer.disable();
    int idx = find(pin);
    if (out_of_range) {
        if (idx >= 0) removeIndex(idx);
    } else {
        if (idx < 0 && _num < MAX_CHANNELS) {
            idx = _num++;
        }

        if (idx >= 0) {
            _channels[idx].pin = pin;
            _channels[idx].value = value;
            _channels[idx].invert = invert;
        }
    }
    _timer.enable();
}

void SPWMClass::update() {
    SPWMChannel *ch;

    for (int i = 0; i < _num; ++i) {
        ch = &(_channels[i]);
        if (ch->value <= 0 || ch->value >= _range) continue;
        doWrite(ch->pin, ch->value > _count, ch->invert);
    }

    if (++_count > _range) {
        _count = 0;
    }
}

void SPWMClass::printInterruptLoad(Print &printer) {
#ifdef __DEBUG_SPWM__
    if (!_timer.isActive()) return;

    unsigned long time1, time2;

    _timer.enable(); // enable interrupt
    time1 = micros();
    delayMicroseconds(5000);
    time1 = micros() - time1;

    _timer.disable(); // disable interrupt
    time2 = micros();
    delayMicroseconds(5000);
    time2 = micros() - time2;

    float load = static_cast< float >(time1 - time2) / time1;
    float interrupt_frequency = static_cast< float >(_hertz * (_range + 1));
    float cycles_per_interrupt = load * (SYSCLK / interrupt_frequency);

    printer.println(F("SPWM::printInterruptLoad():"));
    printer.print(F("  Load of interrupt: "));
    printer.println(load, 10);
    printer.print(F("  Clock cycles per interrupt: "));
    printer.println(cycles_per_interrupt);
    printer.print(F("  Interrupt frequency: "));
    printer.print(interrupt_frequency);
    printer.println(F(" Hz"));
    printer.print(F("  PWM frequency: "));
    printer.print(interrupt_frequency / (_range + 1));
    printer.println(F(" Hz"));
    printer.print(F("  Range: "));
    printer.println(_range + 1);

    _timer.enable();
#endif
}

void SPWMClass::doWrite(uint8_t pin, bool state, bool invert) {
    digitalWrite(pin, (uint8_t) (invert == !state));
}

int SPWMClass::find(uint8_t pin) {
    for (int i = 0; i < _num; ++i) {
        if (_channels[i].pin == pin) {
            return i;
        }
    }
    return -1;
}

void SPWMClass::removeIndex(int idx) {
    for (int i = idx; i < _num - 1; ++i) {
        memcpy(&(_channels[i]), &(_channels[i + 1]), sizeof(SPWMChannel));
    }
    _num--;
}
