/* HAL for ATtiny85 (Digispark-style board with micronucleus bootloader). */
#include <avr/io.h>
#include <avr/interrupt.h>
#include "hal.h"

#define LIGHT_PIN PB0   /* Arduino pin 0 */
#define FAN_PIN   PB1   /* Arduino pin 1 */

/* Timer0, CTC, clk/256 -> ~1 kHz. At F_CPU=16.5 MHz OCR0A=63 gives a 0.993 ms
 * tick (~0.7% fast), which is fine for fan timing. */
#define TIMER_PRESCALER 256UL

static volatile uint32_t ms_ticks;

ISR(TIMER0_COMPA_vect) {
    ms_ticks++;
}

void hal_init(void) {
    DDRB |=  _BV(FAN_PIN);      /* output */
    DDRB &= ~_BV(LIGHT_PIN);    /* input, no pull-up (same as Arduino INPUT) */

    TCCR0A = _BV(WGM01);                            /* CTC mode */
    TCCR0B = _BV(CS02);                             /* clk/256 */
    OCR0A  = (uint8_t)(F_CPU / TIMER_PRESCALER / 1000UL - 1UL);
    TIMSK |= _BV(OCIE0A);
    sei();
}

uint32_t hal_millis(void) {
    uint8_t sreg = SREG;
    cli();
    uint32_t m = ms_ticks;
    SREG = sreg;
    return m;
}

bool hal_read_light_pin(void) {
    return (PINB & _BV(LIGHT_PIN)) != 0;
}

void hal_write_fan_pin(bool level) {
    if (level) PORTB |=  _BV(FAN_PIN);
    else       PORTB &= ~_BV(FAN_PIN);
}
