#include "Scheduler.h"

#include <stdlib.h>

AperList aperList;

void run(SchedParams params) {

}

Task* newTask(bool (*taskFunction)(bool* susFlag), uint32_t compTime) { //amogus
    Task* task = malloc(sizeof(Task));
    task->function = taskFunction;
    task->compTime = compTime;

    return task;
}

void addAperiodic(bool (*taskFunction)(bool* susFlag), uint32_t compTime) {
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

void addPeriodic(bool (*taskFunction)(bool* susFlag), uint32_t compTime, uint32_t period) {

}

void freeTask(Task* t) {
    free(t);
}

void deleteFirstAper(AperList* list) {
    AperListNode* head = list->head;

    if (head == list->tail) list->tail = NULL;
    list->head = head->next;

    freeTask(head->task);
    free(head);

    list->count--;
}

Task* getFirstAper(AperList* list) {
    return list->head->task;
}
