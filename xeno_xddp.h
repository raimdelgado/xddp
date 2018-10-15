#ifndef _XENO_XDDP_H_
#define _XENO_XDDP_H_
/*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <malloc.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <rtdm/ipc.h>
/*****************************************************************************/

typedef enum {

	UNKNOWN = 0,
	NUMBER,
	LABEL,

} xddp_mode;


typedef struct {
	int port_no;
	char* label;
	char send_buffer[128];
	char recv_buffer[128];
	xddp_mode mode;
//	struct sockaddr_ipc saddr;
	int sck;
} XENO_XDDP; 

typedef struct {
	int port_no;
	char* label;
	char write_buffer[128];
	char read_buffer[128];
	xddp_mode mode;
//	struct sockaddr_ipc saddr;
	int fd;
} LINUX_XDDP; 

int rt_xddp_bind(XENO_XDDP* node);
void rt_xddp_set_port(XENO_XDDP* node,int port);
void rt_xddp_set_label(XENO_XDDP* node,char* labell);
int rt_xddp_send(XENO_XDDP* node);
void rt_xddp_recv(XENO_XDDP* node);
void rt_xddp_close(XENO_XDDP* node);


void nrt_xddp_set_port(LINUX_XDDP* node,int port);
void nrt_xddp_set_label(LINUX_XDDP* node,char* labell);
void nrt_xddp_open(LINUX_XDDP* node);
int nrt_xddp_write(LINUX_XDDP* node);
void nrt_xddp_read(LINUX_XDDP* node);
void nrt_xddp_close(LINUX_XDDP* node);

/*****************************************************************************/
#endif
