#include <string.h>

#include "config.h"

#include "init.h"
#include "system.h"
#include "task.h"

void main(void)
{
    init_ports();
    init_system();
}
