#include "config.h"

#include "init.h"
#include "system.h"
#include "task.h"

#include "scheduler.h"

#include <string.h>

void init_system(void)
{
    create_task(task_serial);
    create_task(task_sensor);
    create_task(task_lcd);

    /* Enable timer interrupt for the scheduler */
    INTCONbits.GIE    = 1;
    INTCONbits.TMR0IE = 1;
    /* Timer0 interrupt has no priority */
    // INTCON2bits.TMR0IP = 1;
    RCONbits.IPEN = 1;
    /* Set Timer0 to timer mode */
    T0CONbits.T0CS = 0;
    /* 16 bit timer */
    T0CONbits.T08BIT = 0;
    /* Prescaler not assigned so we have a faster timer */
    T0CONbits.PSA = 1;

    /* Then at last, trigger timer0 interrupt as soon as possible */
    T0CONbits.TMR0ON  = 0;
    INTCONbits.TMR0IF = 1;
}