#include "config.h"

// clang-format off
#include "task.h"
#include "scheduler.h"
// clang-format on

task_t* current_task           = NULL;
uint64_t timer_interrupt_count = 0;

task_t* next_task(void)
{
    task_t* next_task;
    unsigned short current_task_id = task_id(current_task);

    for (unsigned char i = 0; i < MAX_TASKS; i++)
    {
        next_task = &tasks[(i + current_task_id + 1) % MAX_TASKS];

        if (next_task->state == TASK_RUNNING)
        {
            return next_task;
        }
    }

    return &tasks[0];
}

unsigned long timer0_max(void)
{
    if (T0CONbits.T08BIT)
    {
        return 0x100;
    }

    return 0x10000;
}

unsigned short timer0_prescale(void)
{
    unsigned short prescale = 1;

    if (!T0CONbits.PSA)
    {
        prescale += 1;

        int prescale_bits = (T0CONbits.T0PS0 << 1)
                            + (T0CONbits.T0PS0 << 2)
                            + (T0CONbits.T0PS0 << 3);

        prescale += (1 << prescale_bits);
    }

    return prescale;
}

void reset_interrupt_timer0(float time)
{
    TMR0 = (unsigned short)(timer0_max()
                            - (unsigned long)(time * (float)_XTAL_FREQ)
                                / (4 * timer0_prescale()));
}

/* Interrupt Service Routine */
void __interrupt() ISR(void)
{
    static unsigned char WREG_TMP, STATUS_TMP;
    static unsigned short FSR_TMP[3], PROD_TMP;
    /**
     * Safe operation for context switching,
     * we do not loose our wanted registers values
     */
    if (INTCONbits.TMR0IF)
    {
        /* Same here, disable timer during context switch */
        T0CONbits.TMR0ON = 0;
        /* Clear flag */
        INTCONbits.TMR0IF = 0;

        /**
         * Save current task context, here is the tricky part:
         * The WREG, STATUS, FSRx, PROD Registers gets modified when
         * accessing the members of the current_task structure.
         * Solution is to create two temporary variables for them so
         * they do not get modified. Generally we would do this in
         * pure ASM anyway, but since I know how the compiler is
         * creating the instructions, it is safe to do this way.
         */

        STATUS_TMP = STATUS;
        WREG_TMP   = WREG;
        FSR_TMP[0] = FSR0;
        FSR_TMP[1] = FSR1;
        FSR_TMP[2] = FSR2;
        PROD_TMP   = PROD;

        /* If no previous tasks were running, do nothing */
        if (current_task)
        {
            current_task->context.registers.FRS.WREG   = WREG_TMP;
            current_task->context.registers.FRS.STATUS = STATUS_TMP;
            current_task->context.registers.FSR0.L = FSR_TMP[0] & 0xFF;
            current_task->context.registers.FSR0.H = (FSR_TMP[0] >> 8);
            current_task->context.registers.FSR1.L = FSR_TMP[1] & 0xFF;
            current_task->context.registers.FSR1.H = (FSR_TMP[1] >> 8);
            current_task->context.registers.FSR2.L = FSR_TMP[2] & 0xFF;
            current_task->context.registers.FSR2.H = (FSR_TMP[2] >> 8);
            current_task->context.registers.PROD   = PROD_TMP;

            /* The rest is not touched */
            current_task->context.registers.FRS.BSR  = BSR;
            current_task->context.registers.TABLAT   = TABLAT;
            current_task->context.registers.TBLPTR.H = TBLPTRL;
            current_task->context.registers.TBLPTR.L = TBLPTRH;
            current_task->context.registers.TBLPTR.U = TBLPTRU;
            current_task->context.registers.STKPTR   = STKPTR;

            /* Save call stack */
            while (STKPTR > 0)
            {
                current_task->context.registers.TOS[STKPTR - 1] = TOS;
                STKPTR--;
            }
        }

        timer_interrupt_count++;
        reset_interrupt_timer0(DEFAULT_TIMER0_INTERRUPT_TIME);

        /* Update current task */
        current_task = next_task();

        /* No tasks to run anymore ? */
        if (current_task)
        {
            /* Restore its context */
            STKPTR = 0;

            /* Restore call stack */
            while (STKPTR < current_task->context.registers.STKPTR)
            {
                STKPTR++;
                TOSL = current_task->context.registers.TOS[STKPTR - 1]
                       & 0xFF;
                TOSH = (current_task->context.registers.TOS[STKPTR - 1]
                        >> 8)
                       & 0xFF;
                TOSU = (current_task->context.registers.TOS[STKPTR - 1]
                        >> 16)
                       & 0x1F;
            }

            TBLPTRL = current_task->context.registers.TBLPTR.L;
            TBLPTRH = current_task->context.registers.TBLPTR.H;
            TBLPTRU = current_task->context.registers.TBLPTR.U;
            TABLAT  = current_task->context.registers.TABLAT;
            BSR     = current_task->context.registers.FRS.BSR;

            /* Same tricky part as the saving context */
            WREG_TMP   = current_task->context.registers.FRS.WREG;
            STATUS_TMP = current_task->context.registers.FRS.STATUS;

            *(unsigned char*)((uintptr_t)&FSR_TMP[0]) = current_task
                                                          ->context
                                                          .registers.FSR0
                                                          .L;
            *(unsigned char*)((uintptr_t)&FSR_TMP[0] + 1) = current_task
                                                              ->context
                                                              .registers
                                                              .FSR0.H;

            *(unsigned char*)((uintptr_t)&FSR_TMP[1]) = current_task
                                                          ->context
                                                          .registers.FSR1
                                                          .L;
            *(unsigned char*)((uintptr_t)&FSR_TMP[1] + 1) = current_task
                                                              ->context
                                                              .registers
                                                              .FSR1.H;

            *(unsigned char*)((uintptr_t)&FSR_TMP[2]) = current_task
                                                          ->context
                                                          .registers.FSR2
                                                          .L;
            *(unsigned char*)((uintptr_t)&FSR_TMP[2] + 1) = current_task
                                                              ->context
                                                              .registers
                                                              .FSR2.H;
            PROD_TMP = current_task->context.registers.PROD;

            PROD   = PROD_TMP;
            FSR0   = FSR_TMP[0];
            FSR1   = FSR_TMP[1];
            FSR2   = FSR_TMP[2];
            WREG   = WREG_TMP;
            STATUS = STATUS_TMP;
        }

        /* Enable timer */
        T0CONbits.TMR0ON = 1;
    }
}

double current_time(void)
{
    return timer_interrupt_count * DEFAULT_TIMER0_INTERRUPT_TIME;
}
