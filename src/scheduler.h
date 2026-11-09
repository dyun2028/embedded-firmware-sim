#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <stdint.h>

/* Tick-driven cooperative scheduler: a fixed table of periodic tasks, no heap, no preemption.
 * sched_tick() is what a SysTick/timer ISR would call; sched_run() is called from the main loop. */
#define SCHED_MAX_TASKS 8
typedef void (*task_fn)(void *ctx);

typedef struct { task_fn fn; void *ctx; uint32_t period, countdown; volatile uint8_t ready; uint32_t runs, overruns; } task_t;
typedef struct { task_t tasks[SCHED_MAX_TASKS]; int n; volatile uint32_t ticks; } scheduler_t;

void sched_init(scheduler_t *s);
int sched_add(scheduler_t *s, task_fn fn, void *ctx, uint32_t period_ticks);  /* returns task id or -1 */
void sched_tick(scheduler_t *s);   /* ISR context: only sets flags */
int sched_run(scheduler_t *s);     /* main context: runs ready tasks, returns how many ran */
#endif
