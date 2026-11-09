#include "scheduler.h"

void sched_init(scheduler_t *s) { s->n = 0; s->ticks = 0; }

int sched_add(scheduler_t *s, task_fn fn, void *ctx, uint32_t period) {
    if (s->n >= SCHED_MAX_TASKS || period == 0) return -1;
    task_t *t = &s->tasks[s->n];
    t->fn = fn; t->ctx = ctx; t->period = t->countdown = period; t->ready = 0; t->runs = t->overruns = 0;
    return s->n++;
}

void sched_tick(scheduler_t *s) {
    s->ticks++;
    for (int i = 0; i < s->n; i++) {
        task_t *t = &s->tasks[i];
        if (--t->countdown == 0) {
            t->countdown = t->period;
            if (t->ready) t->overruns++;   /* previous run hasn't happened yet: we are falling behind */
            t->ready = 1;
        }
    }
}

int sched_run(scheduler_t *s) {
    int ran = 0;
    for (int i = 0; i < s->n; i++) {
        task_t *t = &s->tasks[i];
        if (t->ready) { t->ready = 0; t->fn(t->ctx); t->runs++; ran++; }
    }
    return ran;
}
