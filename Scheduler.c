#include "Scheduler.h"
#include "Utils.h"

#include <stdlib.h>

AperList aperList;

void run(SchedParams params) {
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
        if (currentTask->task->remainingCompTime-- <= 0) currentTask = &(params.tasks.tasks[0]); //i'm being sneaky and post-decrementing remainingCompTime here. if this doesn't work, let it be known i tried

        //while a millisecond has not yet passed

            //call function
            currentTask->task->function(flags);

            //if reset flag is set, clear it
            flags &= ~FLAG_RESET;

            //if more than 2 milliseconds have passed, the task did not yield often enough and may need adjusted or be incompatible with this scheduler
            //if more than 1 millisecond has passed, break out of this loop

        //if task has run out of remainingCompTime but function did not finish, indicate that the task was not assigned enough time

        //if task has NOT run out of remainingCompTime but function DID finish, set task's remainingCompTime to zero. we will schedule the aperiodic server in its place
        if (currentTask->task->remainingCompTime > 0 && flags & FLAG_FINISHED) {
            currentTask->task->remainingCompTime = 0;
        }
    }

    freePeriodicSchedule(schedule);
    freePeriodicTaskSet(params.tasks);
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
        tasks[currentTask].task->remainingCompTime--; //we have scheduled the task for this time slot so decrement its remaining time
    }

    container->indices = schedule;
    return container;
}

bool aperiodicServer(taskFuncFlag_t* flags, uint32_t compTime) {
    Task* currentTask = aperListPeek(&aperList);
    taskFuncFlag_t* aperFlags = 0;

    if (currentTask == NULL) return; //if there are aperiodic tasks, we yield

    //while a millisecond has not yet passed

        //if current task's remainingCompTime is equal to its compTime then it is new
        if (currentTask->remainingCompTime == currentTask->compTime) {
            aperFlags |= FLAG_RESET; //prepare reset flag
        }

        //run function
        currentTask->function(aperFlags);

        aperFlags &= ~FLAG_RESET; // clear reset flag

    // if function finished
    if (aperFlags & FLAG_FINISHED) {
        // remove it from the queue and free its memory
        aperListDequeue(&aperList);
        freeTask(currentTask);
    }
}

Task* newTask(bool (*taskFunction)(taskFuncFlag_t* flags, uint32_t compTime), uint32_t compTime) { //amogus
    Task* task = malloc(sizeof(Task));
    task->function = taskFunction;
    task->compTime = compTime;
    task->remainingCompTime = compTime;

    return task;
}

void addAperiodic(bool (*taskFunction)(taskFuncFlag_t* flags, uint32_t compTime), uint32_t compTime) {
    Task* t = newTask(taskFunction, compTime);

    AperListNode* tail = aperList.tail;
    AperListNode* new = malloc(sizeof(AperListNode));
    new->task = t;
    new->next = NULL;
    if (tail) tail->next = new;
    else aperList.head = new;
    aperList.tail = new;
    aperList.count++;
}

void addPeriodic(bool (*taskFunction)(taskFuncFlag_t* flags, uint32_t compTime), uint32_t compTime, uint32_t period) {

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

Task* aperListDequeue(AperList* list) {
    AperListNode* head = list->head;

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
