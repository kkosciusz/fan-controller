/*
 * Bathroom fan controller.
 *
 * Operation:
 *
 * Digital input (active low) is tracking state of the interior bathroom light.
 *
 * External relay (active high) provides binary on/off control of a bathroom fan.
 * Whenever light has been turned on for more than FAN_TURN_ON_DELAY, the fan is
 * automatically turned on. After the light is turned off, the fan is
 * kept powered on for another FAN_TURN_OFF_DELAY, and then turned off.
 *
 * Manual control over the fan is provided by toggling the light switch.
 * When the light is toggled during a relatively short time (between
 * LIGHT_DELAY_MIN to LIGHT_DELAY_MAX), the state of fan is toggled. If
 * the light is on and the fan has been turned off this way, it won't
 * become turned on automatically after FAN_TURN_ON_DELAY.
 *
 * After manual fan toggle is registered, subsequent fast toggling
 * of the light will not cause fan toggling. This facilitates the
 * use scenario when light switch is operated three times in rapid
 * succession, ie to toggle the fan and toggle the light switch in the
 * same operation.
 */

#include "app.h"
#include "hal.h"
#include "pt.h"

static const uint32_t fan_turn_on_delay_ms  = 3 * 60 * 1000UL;
static const uint32_t fan_turn_off_delay_ms = 2 * 60 * 1000UL;
static const uint32_t light_toggle_min_time_ms = 250;
static const uint32_t light_toggle_max_time_ms = 1000;

/* sleep that uses the HAL clock (wraparound-safe) */
#define PT_SLEEP_MS(pt, ms) \
    do { static uint32_t sleep_t0_; sleep_t0_ = hal_millis(); \
         PT_WAIT_UNTIL(pt, hal_millis() - sleep_t0_ >= (uint32_t)(ms)); } while (0)

struct timer { uint32_t start, period; };
#define EXPIRED(tm)  (hal_millis() - (tm).start >= (tm).period)
#define ELAPSED(tm)  (hal_millis() - (tm).start)
#define START(tm, p) do { (tm).start = hal_millis(); (tm).period = (p); } while (0)

/* task state (file scope so app_init() can reset it) */
static bool g_light_is_on;
static bool g_fan_toggled_manually;
static uint8_t light_debounce;
static struct timer fan_timer;

static struct pt input_pt;
static struct pt control_pt;

/* helpers: polarity lives here so tests cover it */
static void set_fan(bool state) {
    hal_write_fan_pin(state);
}

static bool get_light(void) {
    return !hal_read_light_pin();
}

static bool light_toggle_valid(uint32_t elapsed) {
    return (elapsed > light_toggle_min_time_ms) && (elapsed < light_toggle_max_time_ms);
}

static PT_THREAD(task_input(struct pt *pt)) {
    PT_BEGIN(pt);

    while (true) {
        light_debounce <<= 1;
        light_debounce |= get_light();

        if (light_debounce == 0xff)
            g_light_is_on = true;
        if (light_debounce == 0)
            g_light_is_on = false;
        PT_SLEEP_MS(pt, 5);
    }

    PT_END(pt);
}

static PT_THREAD(task_control(struct pt *pt)) {
    PT_BEGIN(pt);

    /* at boot, light will be always off */
    while (true) {
        while (true) {
            PT_WAIT_UNTIL(pt, g_light_is_on);

            START(fan_timer, fan_turn_on_delay_ms);

            PT_WAIT_UNTIL(pt, !g_light_is_on || EXPIRED(fan_timer));

            if (!g_light_is_on) {
                if (!light_toggle_valid(ELAPSED(fan_timer))
                    || g_fan_toggled_manually)
                    continue;
                g_fan_toggled_manually = true;
                set_fan(true);
                break;
            }

            /* light is still on and timer expired */
            if (!g_fan_toggled_manually) {
                set_fan(true);
                break;
            }
        }

        while (true) {
            PT_WAIT_UNTIL(pt, !g_light_is_on);

            START(fan_timer, fan_turn_off_delay_ms);

            PT_WAIT_UNTIL(pt, g_light_is_on || EXPIRED(fan_timer));

            if (g_light_is_on) {
                if (!light_toggle_valid(ELAPSED(fan_timer))
                    || g_fan_toggled_manually)
                    continue;
                g_fan_toggled_manually = true;
                set_fan(false);
                break;
            }

            /* light is still off and timer expired */
            g_fan_toggled_manually = false;
            set_fan(false);
            break;
        }

        /* just in case ;) */
        PT_YIELD(pt);
    }
    PT_END(pt);
}

void app_init(void) {
    hal_init();                 /* pin directions, timer */

    g_light_is_on = false;
    g_fan_toggled_manually = false;
    light_debounce = 0;
    fan_timer.start = 0;
    fan_timer.period = 0;

    set_fan(false);

    PT_INIT(&input_pt);
    PT_INIT(&control_pt);
}

void app_poll(void) {
    (void)PT_SCHEDULE(task_input(&input_pt));
    (void)PT_SCHEDULE(task_control(&control_pt));
}
