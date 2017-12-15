#include "mytcp.h"
#include "myrtsp.h"
#include "myrtsp_rtp.h"
#include "myrtsp_session.h"
#include "real.h"
#include <stdio.h>
#include <stdlib.h>

unsigned int gettime()
{
    struct timeval tv;

    if(gettimeofday(&tv, NULL) != 0)
        return 0;
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

int strat()
{

	int fd = 0;
	rtsp_t *s;
	char host[32];
	char *mrl;
	char path[128];
	int port = 554;
	char *server;
	int ret = 0;
	int fd_h264;
	rtsp_session_t * rtsp_session = NULL;
	char *buffer = (char *)malloc(8*1024);
	unsigned int startime;
	sprintf(host,"%s","192.168.168.178");
    mrl = (char *)malloc (128);
	sprintf(mrl,"%s","rtsp://192.168.168.178:554/Streaming/Channels/101?transportmode=unicast&profile=Profile_1");
	sprintf(path,"%s","Streaming/Channels/101?transportmode=unicast&profile=Profile_1");

	fd = connect2Server(host,port);
	if(fd < 0)
	{
		printf("connect2Server error\n");
	}
	rtsp_session = rtsp_session_start(fd,&mrl,path,host,port,&port,0,"admin","admin888");

	fd_h264 = open("h264",O_WRONLY|O_CREAT);
	if(fd_h264 <= 0)
	{
		perror("open fail");
		return -1;
	}
	startime = gettime();
	while(1)
	{
		ret = read_streamdata(rtsp_session->rtp_session->rtp_socket,buffer,8*1024);
		if(ret <= 0)
			break;
		ret = write(fd_h264,buffer,8*1024);
		if(startime + rtsp_session->s->timeout * 1000 <= gettime())
		{
			startime =gettime();
			rtsp_request_setparameter(rtsp_session->s,NULL);
		}

	}

	return 0;
}


int main(int argc,char *argv[])
{
	strat();
	return 0;
}
