#include "BA1474.h"
#include <stdint.h>
#include <stdbool.h>
#include "timer.h"
#include "message_handlers.h"

#INLINE
uint32_t timer_get_us_count() {
    return get_ticks();
}

#define TIMER_TASKS_NUM 30
timer_task_t timerTasks[TIMER_TASKS_NUM];
uint32_t timerTriggered = 0xffffffff;

bool timer_registerTask(uint32_t expTime, timer_func func, void *arg) {
    uint32_t i;
    for (i=0;i<TIMER_TASKS_NUM;i++) {
          if((timerTriggered & (uint32_t)(1 << i)) != 0) {
            /* use this pos */
            timerTriggered &= ~((uint32_t)1 << i);
            timerTasks[i].expTime = expTime;
            timerTasks[i].func = func;
            timerTasks[i].arg = arg;
            return true;
        }
    }
    return false;
}

void timer_checkRunTask() {
    int i;
    uint32_t t = timer_get_us_count();
    for (i=0;i<TIMER_TASKS_NUM;i++) {
        if((timerTriggered & (((uint32_t)1) << i)) == 0) {
            if(t >= timerTasks[i].expTime) {
                timerTriggered |= ((uint32_t)1 << i);
                timerTasks[i].func(timerTasks[i].arg);
            }
        }
    }
}

/* in case a function can spare some CPU time */
void timer_yield() {
#ifndef _bootloader
    timer_checkRunTask();
#endif
}

