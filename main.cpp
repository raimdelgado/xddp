#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <malloc.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <math.h>

#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timerfd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <unistd.h>
#include <fcntl.h>

#include "xeno_posix_task.h"
#include "xeno_xddp.h"

#include <rtdk.h>

#define XDDP_SAMPLE 0

/* RT task */
pthread_t task_rt;
FDTIMER fd_task_rt = -1;

/* NRT task */
pthread_t task_nrt;


int quitFlag = 0;

void signalHandler(int signum)
{
	quitFlag = 1;
} 
void pAbort (const char *s)
{
	perror(s);
	abort();
}

int init_posix_tasks(){
	int err;
	err = pt_create_task_rt(&task_rt,&fd_task_rt,
		(char*)"Sample XDDP task", // name of the task checked in (cat /proc/xenomai/stat)
		99, //priority (99 for highest)
		32, //stack size in [kB]
		1000000000, // period/deadline in [ns]
		rt_sample); //function pointer for the task
	if(err)
	{
		printf("RT: Init task error (%d)\n",err);
		return err;
	}
	err = pt_create_task_nrt(&task_nrt,(char*)"Sample NRT task",nrt_sample);
	if(err)
	{
		printf("NRT: Init task error (%d)\n",err);
		return err;
	}

	return 0;
}


static void *rt_sample(void *arg)
{
	/* Timing */
	PRTIME start,response,end;
	start=response=end=0;
	int ret;
	int iCnt = 0, iRecv = 0;

	XENO_XDDP xRtSample; 
	rt_xddp_set_port(&xRtSample);

	ret = rt_xddp_bind(&xRtSample);
	if (ret)
		pAbort("Bind: XDDP Sample\n");

	end = pt_timer_read();

	while(1)
	{
		if(quitFlag)
		{
			rt_xddp_close(&xRtSample);
			break;
		}
		pt_task_wait_period(fd_task_rt);
		start = pt_timer_read();
		sprintf(xRtSample.send_buffer,"%d",iCnt++);
		rt_xddp_send(&xRtSample);

		rt_xddp_recv(&xRtSample);
		sscanf(xRtSample.recv_buffer,"%d",&iRecv);
		rt_printf("%d", iRecv);
		response = pt_timer_read() - start;
		end = start;
	}

}

static void *nrt_sample(void *arg)
{
	/* Timing */
	PRTIME start,response,end;
	start=response=end=0;

	/* NRT XDDP */
	LINUX_XDDP lnxNrtSample;
	nrt_xddp_set_port(&lnxNrtSample,XDDP_SAMPLE);
	nrt_xddp_open(&lnxNrtSample);

	end = pt_timer_read();

	while(1)
	{
		if(quitFlag)
		{
			nrt_xddp_close(&lnxNrtSample);
			break;
		}
		start = pt_timer_read();

		nrt_xddp_read(&lnxNrtSample);
		printf("NRT Receive: %s\t",lnxNrtSample.read_buffer);

		memcpy(lnxNrtSample.write_buffer,lnxNrtSample.read_buffer,sizeof(lnxNrtSample.read_buffer));

		nrt_xddp_write(&lnxNrtSample);

		response = pt_timer_read() - start;
		end = start; 

	}

}


int main(int argc, char** argv)
{
	int ret;

	sigset_t set;
	int sig;

	mlockall(MCL_CURRENT|MCL_FUTURE);

	sigemptyset(&set);
	sigaddset(&set,SIGINT);
	signal(SIGINT,signalHandler);
	sigaddset(&set, SIGTERM);
	signal(SIGTERM,signalHandler);
	sigaddset(&set,SIGHUP);
	pthread_sigmask(SIG_BLOCK,&set,NULL);


	if((ret = init_posix_tasks()) < 0)
		pAbort("Init POSIX error.\n");
	else
		printf("Init POSIX Done!\n")

	while(1)
	{
		usleep(1);
		if (quitFlag)
			break;
	}
	return 0;
}