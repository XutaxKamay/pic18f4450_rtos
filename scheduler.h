#ifndef SCHEDULER_H
#define SCHEDULER_H

/* Interrupt every 10 ms */
#define DEFAULT_TIMER0_INTERRUPT_TIME 0.01

task_t* next_task(void);
void ISR(void);
unsigned long timer0_max(void);
unsigned short timer0_prescale(void);
void reset_interrupt_timer0(float time);
double current_time(void);

extern task_t* current_task;
extern uint64_t timer_interrupt_count;

#endif
