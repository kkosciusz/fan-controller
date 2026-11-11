#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void app_init(void);
void app_poll(void);   /* call as often as possible from the main loop */

#ifdef __cplusplus
}
#endif
