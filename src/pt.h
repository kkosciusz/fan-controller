/*
 * Minimal protothreads (switch/__LINE__ based, in the style of Adam Dunkels'
 * pt.h). Plain C, no dependencies, works with avr-gcc and host gcc.
 *
 * Rules: local variables do not survive a wait, so use static or file-scope
 * variables for anything that must persist. Do not use `switch` inside a
 * thread. Declare statics BEFORE PT_BEGIN (a declaration cannot directly
 * follow a case label in C).
 */
#pragma once

#include <stdint.h>

struct pt { uint16_t lc; };

#define PT_WAITING 0
#define PT_YIELDED 1
#define PT_EXITED  2
#define PT_ENDED   3

#define PT_INIT(pt)         ((pt)->lc = 0)
#define PT_THREAD(name_args) char name_args

#define PT_BEGIN(pt) { char PT_YIELD_FLAG = 1; (void)PT_YIELD_FLAG; \
                       switch ((pt)->lc) { case 0:

#define PT_END(pt)   } PT_YIELD_FLAG = 0; (pt)->lc = 0; return PT_ENDED; }

#define PT_WAIT_UNTIL(pt, cond) \
    do { (pt)->lc = __LINE__; case __LINE__: \
         if (!(cond)) return PT_WAITING; } while (0)

#define PT_YIELD(pt) \
    do { PT_YIELD_FLAG = 0; (pt)->lc = __LINE__; case __LINE__: \
         if (PT_YIELD_FLAG == 0) return PT_YIELDED; } while (0)

#define PT_SCHEDULE(f) ((f) < PT_EXITED)
