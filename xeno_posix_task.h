#ifndef _XENO_POSIX_TASK_H_
#define _XENO_POSIX_TASK_H_
/*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/timerfd.h>
/*****************************************************************************/
#define START_DELAY_SECS 1 //1sec
#define NANOSEC_PER_SEC 1000000000
#define CLOCK_TO_USE CLOCK_MONOTONIC
#define TIMESPEC2NS(T) ((uint64_t) (T).tv_sec * NANOSEC_PER_SEC + (T).tv_nsec)

typedef int FDTIMER; //for fd timer
typedef uint32_t PRTIME; //for timer probe

/*****************************************************************************/
/* Creation of real-time periodic task using Xenomai Posix Skin 
 * MyPosixThread -> address of pthread descriptor
 * TimerFdForThread -> FD timer for periodicity (FDTIMER should be set globally to -1 at the beginning 
 *                     e.g., FDTIMER fdMotor = -1; )
 * TaskName -> Desired name of task 
 * Priority -> 0~99 with 99 as the highest priority
 * StackSizeInKo -> Defines the size of stack the system will allocate for the thread in kB
 * Period -> Period / Deadline of the task in nanoseconds
 * *pTaskFunction -> address of the task's body routine
 *****************************************************************************/ 
int pt_create_task_rt(pthread_t *MyPosixThread,FDTIMER *TimerFdForThread, char * TaskName, int Priority, int StackSizeInKo, unsigned int Period, void * (*pTaskFunction)(void *) );
/*****************************************************************************/
/* Creation of non real-time task 
 * MyPosixThread -> address of pthread descriptor
 * TaskName -> Desired name of task 
 * *pTaskFunction -> address of the task's body routine
 *****************************************************************************/ 
int pt_create_task_nrt(pthread_t *MyPosixThread,char * TaskName,void * (*pTaskFunction)(void *) );
/*****************************************************************************/
/* Wait for the next periodic release point using FD Timer
 * TimerFdForThread -> the FD timer of the task
 *****************************************************************************/ 
void pt_task_wait_rt_period(FDTIMER TimerFdForThread);
/*****************************************************************************/
/* Returns the current system time expressed in nanoseconds
 *****************************************************************************/ 
PRTIME pt_rt_timer_read(void);
/*****************************************************************************/
#endif