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
 *
 * bit 2    0 - task has not indicated that the program should terminate
 *          1 - task has indicated that the program should terminate
 *
 * bit 3:7  unused
 */
typedef uint8_t taskFuncFlag_t; //typedef for greater clarity when a uint8_t is used as a string of flags
#define FLAG_RESET 0x01 //pass-in flag for when task function should reset itself
#define FLAG_FINISHED 0x02 //pass-back flag for when task has finished
#define FLAG_EXIT 0x0004 //task is indicating that the program should terminate

#define ERROR_FLAGS 0x0003 //a combination of all flags that indicate an error state

/*
 * bit 0    0 - functions yielded often enough
 *          1 - yield error. a function did not yield often enough
 *
 * bit 1    0 - functions were given sufficient comptime
 *          1 - insufficient comptime error. a function was not given sufficient comptime
 *
 * bit 2    0 - task has not indicated that the program should terminate
 *          1 - task has indicated that the program should terminate
 *
 * bit 3:7  unused
 *
 * bit 8:15 stores the 8-bit unsigned index of the task that raised the other flag(s)
 */
typedef uint16_t runFlag_t; //flag to be used by run()
#define FLAG_YIELD_ERROR 0x0001 //a task did not yield often enough
#define FLAG_INSUFFICIENT_COMPTIME 0x0002 //a task did not complete in the comptime it was assigned
#define FLAG_TASKINDEX 0xFF00 //bits 15:8 store the task index that caused the issue
//FLAG_EXIT and ERROR_FLAGS are also relevant in runFlag_t

//for representing a basic, generic task
typedef struct {                                              //if true, only one task with this function is allowed
    void (*function)(taskFuncFlag_t* flags); //the function that actually represents the work to be done
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

//function to build a periodic schedule from a periodic task set
PeriodicSchedule* buildScheduleEDF(PeriodicTaskSet);

void aperiodicServer(taskFuncFlag_t* flags);

//function to initialize this entire scheduler
//CALL THIS FIRST
void sched_init(void);

//main program loop function
runFlag_t run(SchedParams params);
runFlag_t stopRun(runFlag_t flags, PeriodicSchedule* schedule);

//creates a new aperiodic task according to the specification and appends it to the linked list
void addAperiodic(void (*taskFunction)(taskFuncFlag_t* flags), uint32_t);

//creates and returns a new generic/aperiodic task according to the specification
Task* newTask(void (*taskFunction)(taskFuncFlag_t* flags), uint32_t);

//creates and returns a new periodic task according to the specification
PeriodicTask* newPeriodicTask(void (*taskFunction)(taskFuncFlag_t* flags), uint32_t, uint32_t);
void fillPeriodicTask(PeriodicTask* task, void (*taskFunction)(taskFuncFlag_t* flags), uint32_t compTime, uint32_t period);

//for freeing the struct and all relevant members
//you making use of this library only need to free what you directly allocated
//everything the library allocates it is already set up to free
void freeTask(Task*);
void freePeriodicTaskSet(PeriodicTaskSet* ts);
void freeAperList(AperList* list);
void freePeriodicSchedule(PeriodicSchedule* s);
void freePeriodicTask(PeriodicTask* t);

//dequeues a task from the aperiodic list
Task* aperListDequeue(AperList*);
Task* aperListPeek(AperList* list);
