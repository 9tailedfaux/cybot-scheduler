#include <stdint.h>
#include "Scheduler.h"

uint32_t leastCommonMultiple(PeriodicTaskSet ts) {
    uint32_t max = 0;
    while(1) {
        bool flag = false;
        uint8_t i;
        for(i = 0; i < ts.size; i++) {
            uint32_t p = ts.tasks[i].period;
            if(p > max) max = p;
            if(max % p) flag = true;
        }
        if (!flag) return max;
        max++;
    }
}
