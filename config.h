#ifndef CONFIG_H
#define CONFIG_H

/* Special Function Register Limit */
/* 0xF60 to 0xF65 unimplemented for PIC18F4450 */
#define SFR_START 0xF60

#define PROGRAM_START 0x0
#define PROGRAM_END   0x4000

#define GOTO_ASM(address) asm volatile("goto %0" ::"r"(address) :)

#define _XTAL_FREQ 32000

#define RECEIVE_PORT LATE1
#define SEND_PORT    LATE0

typedef void* ptr_t;

#include <stdint.h>
#include <xc.h>

#endif
