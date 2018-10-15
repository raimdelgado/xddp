/*
 *  This file is owned by the Embedded Systems Laboratory of Seoul National University of Science and Technology
 *  
 *  2018 Raimarius Delgado
*/
/****************************************************************************/
/* xeno_posix_task.cpp*/ 
/*****************************************************************************/

#include "xeno_xddp.h"
/*****************************************************************************/
static void pAbort(const char *s)
{
    perror(s);
    abort();
}
/*****************************************************************************/
void rt_xddp_set_port(XENO_XDDP* node,int port){

	node->port_no = port;
	node->mode = NUMBER;
}
/*****************************************************************************/
void rt_xddp_set_label(XENO_XDDP* node,char* labell){

	node->label = labell;
	node->mode = LABEL;

}
/*****************************************************************************/
int rt_xddp_bind(XENO_XDDP* node)
{
	int ret;
	size_t poolsz; //bytes
	struct sockaddr_ipc saddr;

	node->sck = socket(AF_RTIPC, SOCK_DGRAM, IPCPROTO_XDDP);
    if (node->sck < 0) {
            pAbort("XDDP Socket");
    }

    /*
     * Set a local 16k pool for the RT endpoint. Memory needed to
     * convey datagrams will be pulled from this pool, instead of
     * Xenomai's system pool.
     */
    poolsz = 16384; /* bytes */
    ret = setsockopt(node->sck, SOL_XDDP, XDDP_POOLSZ,
                     &poolsz, sizeof(poolsz));
    if (ret)
            pAbort("XDDP Socket PoolSize");
	/*
     * Bind the socket to the port, to setup a proxy to channel
     * traffic to/from the Linux domain.
     *
     * saddr.sipc_port specifies the port number to use.
     */
	memset(&saddr, 0, sizeof(saddr));
    saddr.sipc_family = AF_RTIPC;
    saddr.sipc_port = node->port_no;
    ret = bind(node->sck, (struct sockaddr *)&saddr, sizeof(saddr));    
    if (ret)
            pAbort("XDDP bind");
	else
		printf("XDDP Port: %d is bound\n",node->port_no);
	return ret;
}
/*****************************************************************************/
int rt_xddp_send(XENO_XDDP* node)
{

	int ret;
	int len;
	
	len = strlen(node->send_buffer);
	ret = sendto(node->sck,node->send_buffer,len,0,NULL,0);

	if (ret != len)
        pAbort("XDDP Sendto");

	return ret;

}
/*****************************************************************************/
void rt_xddp_recv(XENO_XDDP* node)
{
	int ret;
	int len;
	
	ret = recvfrom(node->sck,node->recv_buffer,sizeof(node->recv_buffer),0,NULL,0);

	if (ret <= 0)
        pAbort("XDDP Recvfrom");

}
/*****************************************************************************/
