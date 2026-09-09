#ifndef SCHEDULER
#define SCHEDULER

#include "stm32f1xx.h"

typedef void (*task_func_t)(void);

typedef struct{
    task_func_t function;
    uint32_t period_ms;
    uint32_t last_run_tick;
} Task;




#endif