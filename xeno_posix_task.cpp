/*
 *  This file is owned by the Embedded Systems Laboratory of Seoul National University of Science and Technology
 *  
 *  2018 Raimarius Delgado
*/
/****************************************************************************/
/* xeno_posix_task.cpp*/ 
/*****************************************************************************/
#include "xeno_posix_task.h"
/*****************************************************************************/
/* pt_create_task_rt */
/*****************************************************************************/
int pt_create_task_rt(pthread_t *MyPosixThread,FDTIMER *TimerFdForThread,  
	char *TaskName, int Priority, int StackSizeInKo, unsigned int Period, 
	void * (*pTaskFunction)(void *))
{
	pthread_attr_t ThreadAttributes;
	int err = pthread_attr_init(&ThreadAttributes);
	int period_checker = 0;
	if (err)
	{
		printf("RT Thread: attr_init() failed for thread '%s' with err=%d\n", TaskName, err );
		return -10;
	}
	err = pthread_attr_setinheritsched(&ThreadAttributes, PTHREAD_EXPLICIT_SCHED);
	if ( err )
	{
		printf("RT Thread: set explicit sched failed for thread '%s' with err=%d\n", TaskName, err );
		return -11;
	}
	err = pthread_attr_setdetachstate(&ThreadAttributes, PTHREAD_CREATE_DETACHED /*PTHREAD_CREATE_JOINABLE*/);
	if ( err )
	{
		printf("RT Thread: set detach state failed for thread '%s' with err=%d\n", TaskName, err );
		return -12;
	}
	err = pthread_attr_setschedpolicy(&ThreadAttributes, SCHED_FIFO);
	if ( err )
	{
		printf("RT Thread: set scheduling policy failed for thread '%s' with err=%d\n", TaskName, err );
		return -13;
	}
	struct sched_param paramA = { .sched_priority = Priority };
	err = pthread_attr_setschedparam(&ThreadAttributes, &paramA);
	if ( err )
	{
		printf("RT Thread: set priority failed for thread '%s' with err=%d\n", TaskName, err );
		return -14;
	}
	if ( StackSizeInKo>0 )
	{
		err = pthread_attr_setstacksize(&ThreadAttributes, StackSizeInKo*1024);
		if ( err )
		{
			printf("RT Thread: set stack size failed for thread '%s' with err=%d\n", TaskName, err );
			return -15;
		}
	}
	// calc start time of the periodic thread
	struct timespec start_time;
	if ( clock_gettime( CLOCK_TO_USE, &start_time ) )
	{
		printf( "RT Thread: Failed to call clock_gettime\n" );
		return -20;
	}
	/* Start one second later from now. */
	start_time.tv_sec += START_DELAY_SECS ;
	
	// if a timerfd is used to make thread periodic (Linux / Xenomai 3),
	// initialize it before launching thread (timer is read in the loop)
	struct itimerspec period_timer_conf;
	*TimerFdForThread = timerfd_create(CLOCK_TO_USE, 0);
	if ( *TimerFdForThread==-1 )
	{
		printf( "RT Thread: Failed to create timerfd for thread '%s'\n", TaskName);
		return -21;
	}
	period_timer_conf.it_value = start_time;
	period_timer_conf.it_interval.tv_sec = Period/NANOSEC_PER_SEC;
	period_timer_conf.it_interval.tv_nsec = (Period%NANOSEC_PER_SEC) ;
	// printf("sec: %ld\n",period_timer_conf.it_interval.tv_sec);
	// printf("nsec: %ld\n",period_timer_conf.it_interval.tv_nsec);
	if ( timerfd_settime(*TimerFdForThread, TFD_TIMER_ABSTIME, &period_timer_conf, NULL) )
	{
		printf( "RT Thread: Failed to set periodic tor thread '%s' with errno=%d\n", TaskName, errno);
		return -22;
	}
	err = pthread_create(MyPosixThread, &ThreadAttributes, (void *(*)(void *))pTaskFunction, (void *)NULL );
	if ( err )
	{
		printf( "RT Thread: Failed to create thread '%s' with err=%d !!!!!\n", TaskName, err );
		return -1;
	}
	else
	{

		pthread_attr_destroy(&ThreadAttributes);
		err = pthread_setname_np(*MyPosixThread, TaskName );
		if ( err )
		{
			printf("RT Thread: set name failed for thread '%s', err=%d\n", TaskName, err );
			return -40;
		}
		printf( "RT Thread: Created thread '%s' period=%d ns ok.\n", TaskName, Period);
		return 0;
	}
}
/*****************************************************************************/
/* pt_create_task_rt */
/*****************************************************************************/
int pt_create_task_nrt(pthread_t *MyPosixThread,char * TaskName,void * (*pTaskFunction)(void *) )
{
	int err;
	pthread_attr_t Thread_Attr;

	err = pthread_attr_init(&Thread_Attr);
	if (err)
	{
		printf("NRT Thread: attr_init() failed for thread '%s' with err=%d\n", TaskName, err);
		return -10;
	}

    err = pthread_attr_setdetachstate(&Thread_Attr, PTHREAD_CREATE_DETACHED /*PTHREAD_CREATE_JOINABLE*/);
    if ( err )
	{
		printf("NRT Thread: set detach state failed for thread '%s' with err=%d\n", TaskName, err );
		return -12;
	}

    err = pthread_attr_setinheritsched(&Thread_Attr, PTHREAD_EXPLICIT_SCHED);
    if ( err )
	{
		printf("NRT Thread: set explicit sched failed for thread '%s' with err=%d\n", TaskName, err );
		return -11;
	}

    err = pthread_attr_setschedpolicy(&Thread_Attr, SCHED_OTHER);
    if ( err )
	{
		printf("NRT Thread: set scheduling policy failed for thread '%s' with err=%d\n", TaskName, err );
		return -13;
	}

    err = pthread_create(MyPosixThread, &Thread_Attr, (void *(*)(void *))pTaskFunction, (void *)NULL );
    if ( err )
	{
		printf( "NRT Thread: Failed to create thread '%s' with err=%d !!!!!\n", TaskName, err );
		return -1;
	}
	else{
		pthread_attr_destroy(&Thread_Attr);
		err = pthread_setname_np (*MyPosixThread,"Actuator Task");
		if ( err )
		{
			printf("NRT Thread: set name failed for thread '%s', err=%d\n", TaskName, err );
			return -40;
		}
		printf("NRT Thread: Created thread '%s' ok.\n", TaskName);
		return 0;
    }
}
/*****************************************************************************/
/* pt_task_wait_rt_period */
/*****************************************************************************/
void pt_task_wait_rt_period(FDTIMER TimerFdForThread)
{
	int err = 0;
	uint64_t ticks;
	err = read(TimerFdForThread, &ticks, sizeof(ticks));
	if ( err<0 )
	{
		printf( "TimerFd wait period failed for thread with errno=%d\n", errno );
	}
	if ( ticks>1 )
	{
		printf( "TimerFd wait period missed for thread: overruns=%lu\n", (long unsigned int)ticks );
	}
}
/*****************************************************************************/
/* pt_rt_timer_read */
/*****************************************************************************/
PRTIME pt_rt_timer_read(void){
	struct timespec probe;
	clock_gettime(CLOCK_TO_USE,&probe);
	return TIMESPEC2NS(probe);
}
/*****************************************************************************/