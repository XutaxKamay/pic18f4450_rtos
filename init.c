#include "config.h"

#include "init.h"

void pic_running(void)
{
    /* say that this pic is running */
    TRISE2 = 0;
    LATE2  = 1;
}

void init_ports(void)
{
    pic_running();

    /* receive data */
    TRISE1 = 1;

    /* send data */
    TRISE0 = 0;

    /* no usb */
    UCONbits.SUSPND = 0;

    while (UIRbits.ACTVIF)
    {
        UIRbits.ACTVIF = 0;
    }

    UCONbits.USBEN = 0;
}