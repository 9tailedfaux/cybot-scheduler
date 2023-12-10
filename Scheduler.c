#include "Scheduler.h"
#include "Utils.h"
#include "Timer.h" //NOTE: Timer code is from CPRE 288 and was not developed by me

#include <stdlib.h>

AperList aperList;

void sched_init() {
    timer_init();
    timer_stop();
}

runFlag_t run(SchedParams params) {
    PeriodicSchedule* schedule = buildScheduleEDF(params.tasks);

    uint32_t i;
    for (i = 0; i < schedule->size; i++) {
        uint8_t currentTaskIndex = schedule->indices[i];
        PeriodicTask* currentTask = &(params.tasks.tasks[currentTaskIndex]);

        taskFuncFlag_t flags = 0;

        //if we are at a multiple of the task's period then it is new.
        if (i % currentTask->period == 0) {
            currentTask->task->remainingCompTime = currentTask->task->compTime;
            flags |= FLAG_RESET;  // prepare reset flag
        }

        //if current task's remainingCompTime is out of bounds, run aperiodic server instead
        if (currentTask->task->remainingCompTime-- <= 0) currentTask = params.tasks.tasks;

        timer_resume();

        while (timer_getMillis() < 1) {

            //call function
            currentTask->task->function(&flags);

            //if reset flag is set, clear it
            flags &= ~FLAG_RESET;

            if (flags & FLAG_EXIT) {
                return stopRun(FLAG_EXIT | currentTaskIndex, schedule);
            }

            //if more than 2 milliseconds have passed, the task did not yield often enough
            //it may need adjusted or be incompatible with this scheduler
            if (timer_getMillis() > 2) {
                return stopRun(FLAG_YIELD_ERROR | currentTaskIndex, schedule);
            }
        }

        //if task has run out of remainingCompTime but function did not finish, indicate that the task was not assigned enough time
        if (currentTask->task->remainingCompTime == 0 && !(flags & FLAG_FINISHED)) {
            return stopRun(FLAG_INSUFFICIENT_COMPTIME | currentTaskIndex, schedule);
        }

        //if task has NOT run out of remainingCompTime but function DID finish, set task's remainingCompTime to zero.
        //we will schedule the aperiodic server in its place
        if (currentTask->task->remainingCompTime > 0 && flags & FLAG_FINISHED) {
            currentTask->task->remainingCompTime = 0;
        }

        timer_stop();
    }

    return stopRun(0x0000, schedule);
}

runFlag_t stopRun(runFlag_t flags, PeriodicSchedule* schedule) {
    freePeriodicSchedule(schedule);
    timer_stop();
    return flags;
}

PeriodicSchedule* buildScheduleEDF(PeriodicTaskSet ts) {
    uint32_t lcm = leastCommonMultiple(ts);
    PeriodicSchedule* container = (PeriodicSchedule*)malloc(sizeof(PeriodicSchedule));
    container->size = lcm;
    uint8_t* schedule = (uint8_t*)malloc(sizeof(uint8_t) * lcm);
    PeriodicTask* tasks = ts.tasks;
    uint32_t i;
    for (i = 0; i < lcm; i++) {
        uint8_t j;
        uint8_t currentTask = 0;
        for (j = 0; j < ts.size; j++) {
            PeriodicTask thisTask = tasks[j];

            //if we have hit a multiple of the task's period, refill remaining computation time and set its deadline
            if (i % thisTask.period == 0) {
                //if the task wasn't fully allocated, the schedule is impossible. return null
                if (thisTask.task->remainingCompTime > 0) {
                    return NULL;
                }
                thisTask.task->remainingCompTime = thisTask.task->compTime;
                thisTask.deadline = i + thisTask.period;
            }

            //if task has no more computation time remaining, continue
            if (thisTask.task->remainingCompTime <= 0) continue;

            //else, if task is closer to its deadline than the current task, set current task to it
            if (thisTask.deadline - i < tasks[currentTask].deadline - i) currentTask = j;
        }

        //task 0 will be run as default if all are cleared so make task 0 the aperiodic server for best results

        schedule[i] = currentTask; //add the chosen task to schedule
        
        //we have scheduled the task for this time slot so decrement its remaining time
        tasks[currentTask].task->remainingCompTime--; 
    }

    container->indices = schedule;
    return container;
}

void aperiodicServer(taskFuncFlag_t* flags) {
    Task* currentTask = aperListPeek(&aperList);
    taskFuncFlag_t* aperFlags = 0;

    if (currentTask == NULL) return; //if there are aperiodic tasks, we yield

    //while a millisecond has not yet passed
    while (timer_getMillis() < 1) {

        //if current task's remainingCompTime is equal to its compTime then it is new
        if (currentTask->remainingCompTime == currentTask->compTime) {
            aperFlags |= FLAG_RESET; //prepare reset flag
        }

        //run function
        currentTask->function(aperFlags);

        aperFlags &= ~FLAG_RESET; // clear reset flag

    }

    // if function finished
    if (aperFlags & FLAG_FINISHED) {
        // remove it from the queue and free its memory
        aperListDequeue(&aperList);
        freeTask(currentTask);
    }
}

PeriodicTask* newPeriodicTask(void (*taskFunction)(taskFuncFlag_t* flags), uint32_t compTime, uint32_t period) {
    PeriodicTask* pt = (PeriodicTask*)malloc(sizeof(PeriodicTask));
    
    pt->task = newTask(taskFunction, compTime);
    pt->period = period;
    pt->deadline = period;

    return pt;
}

Task* newTask(void (*taskFunction)(taskFuncFlag_t* flags), uint32_t compTime) { //amogus
    Task* task = (Task*)malloc(sizeof(Task));
    task->function = taskFunction;
    task->compTime = compTime;
    task->remainingCompTime = compTime;

    return task;
}

void addAperiodic(void (*taskFunction)(taskFuncFlag_t* flags)) {
    Task* t = newTask(taskFunction, compTime);

    AperListNode* tail = aperList.tail;
    AperListNode* new = (AperListNode*)malloc(sizeof(AperListNode));
    new->task = t;
    new->next = NULL;
    if (tail) tail->next = new;
    else aperList.head = new;
    aperList.tail = new;
    aperList.count++;
}

void freeTask(Task* t) {
    free(t);
}

void freePeriodicTask(PeriodicTask* t) {
    freeTask(t->task);
    free(t);
}

void freePeriodicSchedule(PeriodicSchedule* s) {
    free(s->indices);
    free(s);
}

void freePeriodicTaskSet(PeriodicTaskSet* ts) {
    uint8_t i;
    for (i = 0; i < ts->size; i++) {
        freePeriodicTask(ts->tasks[i]);
        free(ts->tasks)
    }
    free(ts);
}

void freeAperList(AperList* list) {
    AperListNode* head = list->head;
    while (aperListDequeue(list)) { }
    free(list);
}

Task* aperListDequeue(AperList* list) {
    AperListNode* head = list->head;

    if (!head) return NULL;

    if (head == list->tail) list->tail = NULL;
    list->head = head->next;

    Task* t = head->task;

    free(head);

    list->count--;

    return t;
}

Task* aperListPeek(AperList* list) {
    return list->head->task;
}
