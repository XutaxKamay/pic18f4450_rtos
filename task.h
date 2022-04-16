#ifndef TASK_H
#define TASK_H

/**
 * 4 tasks maximum because mainly limited (0x300 bytes) data memory.
 * This could be bypassed by using the self-programmability of the chip;
 * by simply starting saying that the task array is starting a bit before
 * 0x4000 (as program memory is limited to 0x4000 bytes).
 * It is safe to use memory at ptr = PROGRAM_END - sizeof(struct) for
 * example; Because data memory and program memory is seperated. We must
 * use _at macro for using data memory, and raw pointer for program
 * memory.
 * Anyway we only need it for LCD and the sensors.
 * A memory manager wouldn't solve the problem because the actual
 * implementation of this would take too much space.
 */
#define MAX_TASKS 3

typedef void (*task_routine_t)(void);

enum
{
    TASK_VOID,    /* task does not exist */
    TASK_RUNNING, /* task is running */
    TASK_STOPPED, /* task is stopped */
};

typedef unsigned char task_state_t;

typedef struct registers
{
    /* Needed for Fast Register Stack for (high priority) interrupts */
    struct
    {
        /* Bank Select Register */
        unsigned char BSR : 4;
        /* Working Register */
        unsigned char WREG;
        unsigned char STATUS : 5;
    } FRS;

    /* File select registers, for indirect addressing (INDF) */
    /* Shit compiler doesn't support bit field than 8 bits */
    struct
    {
        unsigned char H : 4;
        unsigned char L;
    } FSR0;

    struct
    {
        unsigned char H : 4;
        unsigned char L;
    } FSR1;

    struct
    {
        unsigned char H : 4;
        unsigned char L;
    } FSR2;

    /**
     * Table Latch register, used for transfer between program
     * And data memory per 8 bits.
     */
    unsigned char TABLAT;

    /**
     * Table Pointer register,  the address of the program memory being
     * transfered into data memory.
     */
    struct
    {
        unsigned char U : 6;
        unsigned char H;
        unsigned char L;
    } TBLPTR;

    /**
     * Hardware multiplier register
     */
    unsigned short PROD;

    /**
     * Top Of Stack registers.
     * STKPTR doesn't go above 31.
     * It would be cool to have a memory manager,
     * but the memory manager itself would take too much space.
     * (that's a bit of irony)
     * BUGBUGBUG:
     * I don't know why but it doesn't like much to get/set values from,
     * TOSL,TOSH, TOSU with bit fields. Fuck it.
     */
    uint24_t TOS[31];

    /**
     * Stack pointer register.
     */
    unsigned char STKPTR : 5;

} registers_t;

typedef struct context
{
    registers_t registers;
} context_t;

typedef struct task
{
    context_t context;
    task_state_t state;
} task_t;

extern task_t tasks[MAX_TASKS];
/* extern task_t* tasks; */

void task_serial(void);
void task_sensor(void);
void task_lcd(void);

void _prepare_routine_registers(task_t* task,
                                task_routine_t task_routine);
unsigned char task_count(void);
unsigned char task_id(task_t* task);
task_t* create_task(task_routine_t task_routine);
void close_task(task_t* task);

#endif
