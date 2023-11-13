/*
 * Scheduler.h
 *
 *  Created on: Nov 13, 2023
 *      Author: winter
 */
#include <stdint.h>
#include <stdbool.h>

typedef struct {

} SchedParams;

typedef struct _Task {
    bool unique; //if true, only one task with this function is allowed
    bool (*function)(bool* susFlag); //the function that actually represents the work to be done
    uint32_t compTime; //computation time
} Task;

typedef struct _PeriodicTask {
    Task task;
    uint32_t period;
} PeriodicTaskTask;

void run(SchedParams params);

void addAperiodic(bool (*taskFunction)(bool* susFlag), uint32_t compTime);

Task* newTask(bool (*taskFunction)(bool* susFlag), uint32_t compTime);

void freeTask(Task*);

typedef struct _AperListNode {
    struct _AperListNode* next;
    Task* task;
} AperListNode;

typedef struct _AperList {
    AperListNode* head;
    AperListNode* tail;
    uint8_t count;
} AperList;

void deleteFirstAper(AperList*);
Task* getFirstAper(AperList*);
