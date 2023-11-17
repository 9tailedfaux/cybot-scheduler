/*
 * Scheduler.h
 *
 *  Created on: Nov 13, 2023
 *      Author: winter
 *
 *  a task function should "yield" its time with a return statement whenever it is able to
 *  it must also be able to resume where it left off when called again
 */
#include <stdint.h>
#include <stdbool.h>

/*
 * bit 0    0 - function should resume where it last left off
 *          1 - function should reset itself
 *
 * bit 1    0 - function has not yet finished
 *          1 - function has finished
 */
typedef uint8_t taskFuncFlag_t; //typedef for greater clarity when a uint8_t is used as a string of flags
#define FLAG_RESET 0x01
#define FLAG_FINISHED 0x02

//for representing a basic, generic task
typedef struct {
    bool unique;                                                //if true, only one task with this function is allowed
    bool (*function)(taskFuncFlag_t* flags, uint32_t compTime); //the function that actually represents the work to be done
    uint32_t compTime;                                          //computation time
    uint32_t remainingCompTime;                                 //remaining computation time
} Task;

//for representing a periodic task
typedef struct _PeriodicTask {
    Task* task; //nested basic task struct
    uint32_t period; //how often the task will recur
    uint32_t deadline; //this is a multiple of period
} PeriodicTask;

//for representing a set of tasks
typedef struct {
    PeriodicTask* tasks; //array of periodic tasks
    uint8_t size; //number of periodic tasks
} PeriodicTaskSet;

//for representing a task schedule
// NOTE: this stores indices which correspond to the task set it was built from
// this was done for memory conservation as a pointer is 32 bits but an index is only 8 bits
// this is important because this array may become large with a value for each time slot in a schedule
// there are no empty time slots as the aperiodic server will fill any empty slots
typedef struct {
    uint8_t size; //the number of time slots
    uint8_t* indices; //the index of a corresponding task in the task set this was built from
} PeriodicSchedule;

//a set of parameters with which to run a cycle of the scheduler
typedef struct {
    PeriodicTaskSet tasks; //task set to generate schedule from
} SchedParams;

//node in a linked list queue data structure for aperiodic task management
typedef struct _AperListNode {
    struct _AperListNode* next;
    Task* task;
} AperListNode;

typedef struct _AperList {
    AperListNode* head;
    AperListNode* tail;
    uint8_t count;
} AperList;

PeriodicSchedule* buildScheduleEDF(PeriodicTaskSet);
void run(SchedParams params);
void addAperiodic(bool (*taskFunction)(taskFuncFlag_t* flags, uint32_t compTime), uint32_t);
Task* newTask(bool (*taskFunction)(taskFuncFlag_t* flags, uint32_t compTime), uint32_t);
void freeTask(Task*);
Task* aperListDequeue(AperList*);
