#include "config.h"

#include "init.h"
#include "task.h"

#include "scheduler.h"

/* Shit compiler */
asm("GLOBAL _task_serial");
asm("GLOBAL _task_sensor");
asm("GLOBAL _task_lcd");

task_t tasks[MAX_TASKS] = { [0 ... MAX_TASKS - 1] = {
                              .context = { .registers = { .FRS = { .BSR = 0,
                                                                   .STATUS = 0,
                                                                   .WREG = 0 },
                                                          .FSR0.L = 0,
                                                          .FSR0.H = 0,
                                                          .FSR1.L = 0,
                                                          .FSR1.H = 0,
                                                          .FSR2.L = 0,
                                                          .FSR2.H = 0,
                                                          .PROD   = 0,
                                                          .STKPTR = 0,
                                                          .TABLAT = 0,
                                                          .TBLPTR = 0,
                                                          .TOS = { [0 ... 30] = 0 } } },
                              .state   = TASK_VOID } };

void task_serial(void)
{
    while (1)
    {
    }
}

void task_sensor(void)
{
    while (1)
    {
        SEND_PORT = 1;
    }
}

void task_lcd(void)
{
    while (1)
    {
        SEND_PORT = 0;
    }
}

void prepare_routine_registers(task_t* task, task_routine_t task_routine)
{
    task->state                      = TASK_RUNNING;
    task->context.registers.FRS.BSR  = BSR;
    task->context.registers.PROD     = PROD;
    task->context.registers.TABLAT   = TABLAT;
    task->context.registers.TBLPTR.H = TBLPTRH;
    task->context.registers.TBLPTR.L = TBLPTRL;
    task->context.registers.TBLPTR.U = TBLPTRU;
    task->context.registers.TOS[0]   = (uint24_t)task_routine;
    task->context.registers.STKPTR   = 1;
    task->context.registers.FSR0.L   = FSR0L;
    task->context.registers.FSR0.H   = FSR0H;
    task->context.registers.FSR1.L   = FSR1L;
    task->context.registers.FSR1.H   = FSR1H;
    task->context.registers.FSR2.L   = FSR2L;
    task->context.registers.FSR2.H   = FSR2H;
}

unsigned char task_count(void)
{
    unsigned char count = 0;

    for (unsigned char i = 0; i < MAX_TASKS; i++)
    {
        if (tasks[i].state != TASK_VOID)
        {
            count++;
        }
    }

    return count;
}

unsigned char task_id(task_t* task)
{
    return (unsigned char)(task - tasks);
}

task_t* create_task(task_routine_t task_routine)
{
    /**
     * No need for mutex since PIC is single-threaded.
     * However, if we have MPPA/Multiple MUC;
     * that might need some changes in the future... But unlikely.
     */

    task_t* task = NULL;

    for (int i = 0; i < MAX_TASKS; i++)
    {
        if (tasks[i].state == TASK_VOID)
        {
            task = &tasks[i];
            prepare_routine_registers(task, task_routine);
            break;
        }
    }

    return task;
}

void close_task(task_t* task)
{
    if (task)
    {
        task->state = TASK_VOID;
    }
}