#include "pid.h"

void pid_init(pid_t_ *c, q16_t kp, q16_t ki, q16_t kd, q16_t lo, q16_t hi) {
    c->kp = kp; c->ki = ki; c->kd = kd; c->integ = 0; c->prev_err = 0; c->out_min = lo; c->out_max = hi;
}

q16_t pid_step(pid_t_ *c, q16_t sp, q16_t meas) {
    q16_t err = sp - meas;
    q16_t d = err - c->prev_err;
    c->prev_err = err;
    q16_t trial_i = c->integ + err;
    q16_t out = q16_mul(c->kp, err) + q16_mul(c->ki, trial_i) + q16_mul(c->kd, d);
    if (out > c->out_max) { out = c->out_max; if (err < 0) c->integ = trial_i; }       /* anti-windup: only integrate toward the limit's inside */
    else if (out < c->out_min) { out = c->out_min; if (err > 0) c->integ = trial_i; }
    else c->integ = trial_i;
    return out;
}
