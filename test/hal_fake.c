/* Fake HAL for host tests: simulated clock and pins. */
#include "hal.h"
#include "app.h"
#include "sim.h"

static uint32_t now;
static bool light_pin = true;   /* idle high: light off (active low) */
static bool fan_pin;

void hal_init(void) {}
uint32_t hal_millis(void) { return now; }
bool hal_read_light_pin(void) { return light_pin; }
void hal_write_fan_pin(bool level) { fan_pin = level; }

void sim_reset(uint32_t start_ms) {
    now = start_ms;
    light_pin = true;
    fan_pin = false;
    app_init();
}

void sim_set_light(bool on) { light_pin = !on; }
bool sim_fan_pin(void) { return fan_pin; }

void sim_advance(uint32_t ms) {
    while (ms--) {
        now++;
        app_poll();
    }
}
