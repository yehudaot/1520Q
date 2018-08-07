#ifndef INCLUDE_TIMER_H_
#define INCLUDE_TIMER_H_

#include <stdint.h>
#include <stdbool.h>

typedef void (*timer_func)(void* arg);

typedef struct {
  //bool triggred;
  uint32_t expTime;
  void *arg;
  timer_func func;
} timer_task_t;

#INLINE
uint32_t timer_get_us_count();
bool timer_registerTask(uint32_t expTime, timer_func func, void *arg);
void timer_yield();

#endif /* INCLUDE_TIMER_H_ */
