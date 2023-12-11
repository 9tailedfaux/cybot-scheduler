#include <stdio.h>
#include "lcd.h" //NOTE: LCD code is from CPRE 288 and was not developed by me
#include "testTasks.h"
#include <stdlib.h>

/**
 * main.c
 */

void handleError(runFlag_t flags) {
    char error_string[41]; //just large enough for largest error message + null byte
    if (flags & FLAG_YIELD_ERROR) {
        sprintf(error_string, "Task %d did not yield frequently enough", (flags & FLAG_TASKINDEX) >> 8);
    } 
    else if (flags & FLAG_INSUFFICIENT_COMPTIME) {
        sprintf(error_string, "Task %d was not given enough compTime", (flags & FLAG_TASKINDEX) >> 8);
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
    PeriodicTask* testTasks = (PeriodicTask*)malloc(sizeof(PeriodicTask) * 3);

    fillPeriodicTask(testTasks + 0, aperiodicServer, 1, 5);
    fillPeriodicTask(testTasks + 1, oneMilliTask, 1, 4);
    fillPeriodicTask(testTasks + 2, twoMillisTask, 2, 6);
    ts.size = 3;
    ts.tasks = testTasks;
    //remember to make the aperiodic server index 0

    //add taskset to params
    params.tasks = ts;

    //run in loop while FLAG_EXIT is not set
    while(!(flags & (FLAG_EXIT | ERROR_FLAGS))) {
        flags = run(params);
    }

    if (flags & ERROR_FLAGS) {
        handleError(flags);
    }

	return 0;
}
