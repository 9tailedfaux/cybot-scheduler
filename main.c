#include "Scheduler.h"
#include <stdio.h>
#include "lcd.h" //NOTE: LCD code is from CPRE 288 and was not developed by me

/**
 * main.c
 */

void handleError(runFlag_t flags) {
    char error_string[42]; //just large enough for largest error message + null byte
    if (flags & FLAG_YIELD_ERROR) {
        sprintf(error_string, "Task %d did not yield frequently enough\n", (flags & FLAG_TASKINDEX) >> 8);
    } 
    else if (flags & FLAG_INSUFFICIENT_COMPTIME) {
        sprintf(error_string, "Task %d was not given enough compTime\n", (flags & FLAG_TASKINDEX) >> 8);
    }
    else {
        //catch-all "other" state
        sprintf(error_string, "Unknown error\n");
    }

    lcd_puts(error_string);
}

int main(void)
{
    sched_init();
    lcd_init();
    SchedParams params;

    //declare
    PeriodicTaskSet ts;
    runFlag_t flags = 0;

    //assemble taskset
    //remember to make the aperiodic server index 0

    //add taskset to params
    params.tasks = ts;

    //run in loop while FLAG_EXIT is not set
    while(!(flags & FLAG_EXIT)) {
        run(params, &flags);
    }

    if (flags & ERROR_FLAGS) {
        handleError(flags);
    }

	return 0;
}
