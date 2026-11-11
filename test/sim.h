#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void sim_reset(uint32_t start_ms);
void sim_set_light(bool on);      /* true = light on (pin driven low) */
bool sim_fan_pin(void);           /* raw fan output level */
void sim_advance(uint32_t ms);    /* advance clock 1 ms at a time, polling each tick */

#ifdef __cplusplus
}
#endif
