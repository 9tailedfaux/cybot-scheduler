#include "testTasks.h"
#include "Timer.h"
#include "lcd.h"

//example of a bad task function designed not to yield frequently enough
void yieldError(taskFuncFlag_t* flags) {
    timer_waitMillis(3);
}

//example of a task function that requires 2ms of compTime and will not finish if given less
uint8_t i = 0;
void twoMillisTask(taskFuncFlag_t* flags) {
    if (*flags & FLAG_RESET) i = 0;

    for (; i < 2; i++) {
        timer_waitMillis(1); //delay

        if (i == 1) *flags |= FLAG_FINISHED; //finished case
        break;
    }
}

//example of a task that takes only a single millisecond timeslot
void oneMilliTask(taskFuncFlag_t* flags) {
    timer_waitMillis(1);
    *flags |= FLAG_FINISHED;
}

//example of a terminal task
void exitTask(taskFuncFlag_t* flags) {
    lcd_puts("Terminal task ran");
    *flags |= FLAG_FINISHED | FLAG_EXIT;
}
