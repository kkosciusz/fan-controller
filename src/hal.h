#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void     hal_init(void);                 /* pin directions, timer setup */
uint32_t hal_millis(void);
bool     hal_read_light_pin(void);       /* raw pin level */
void     hal_write_fan_pin(bool level);  /* raw pin level */

#ifdef __cplusplus
}
#endif
