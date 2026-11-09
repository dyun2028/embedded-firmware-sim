#ifndef PID_H
#define PID_H
#include <stdint.h>

/* Integer-only PID controller in Q16.16 fixed point, the way it is done on MCUs without an FPU.
 * Output is clamped, and the integrator uses conditional anti-windup. */
typedef int32_t q16_t;
#define Q16(x) ((q16_t)((x) * 65536.0))
static inline q16_t q16_mul(q16_t a, q16_t b) { return (q16_t)(((int64_t)a * b) >> 16); }

typedef struct { q16_t kp, ki, kd, integ, prev_err, out_min, out_max; } pid_t_;

void pid_init(pid_t_ *c, q16_t kp, q16_t ki, q16_t kd, q16_t out_min, q16_t out_max);
q16_t pid_step(pid_t_ *c, q16_t setpoint, q16_t measured);   /* fixed timestep of 1 tick */
#endif
