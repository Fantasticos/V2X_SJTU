#include <v2x_api.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#define LOG_CONFIG_PATH     "/usr/local/mocar_log.conf"
#define MOUDLE_NAME         "bsm"
#define MAX_BSM_MSG_COUNT   128
//**********************TCP CODE START***********************
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 4321

#define BACKLOG 1
#define MAXRECVLEN 1024
double	latitude_recv;
double	angle_recv;
double  unixtime_send;
double  unixtime_recv;
//*********************TCP CODE END******************

typedef void (*timer_func)(union sigval sig);

int v2x_set_timer(timer_t timerId, int timeMsec)
{
	struct itimerspec its;

	its.it_value.tv_sec = timeMsec / 1000;
	its.it_value.tv_nsec	= (timeMsec % 1000)*1000000;
	its.it_interval.tv_sec = timeMsec / 1000;
	its.it_interval.tv_nsec	= (timeMsec % 1000)*1000000;

	if(timer_settime(timerId, 0, &its, NULL) < 0)
    {
		fprintf(stderr, "timer settime error\n");
        return -1;
    }

    return 0;
}

int v2x_create_timer(timer_t *timerId, timer_func func, void *param)
{
	struct sigevent sev;

	sev.sigev_notify = SIGEV_THREAD;
	sev.sigev_signo = SIGRTMIN;
	sev.sigev_value.sival_ptr = param;
	sev.sigev_notify_function = func;
	sev.sigev_notify_attributes = NULL;

	if(timer_create(CLOCK_REALTIME, &sev, timerId) < 0)
	{
		fprintf(stderr, "timer_create error!\n");
		return -1;
	}
	if((int)(*timerId) == -1)
    {
		fprintf(stderr, "timer_create error,id equ -1!\n");
        return -1;
    }

	return 0;
}

int v2x_fill_bsm_msg(v2x_msg_bsm_t* usr_bsm)
{
    static unsigned int     msgcount   = 0;
    unsigned int            id = 0x01020304;//{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

    if (usr_bsm == NULL)
    {
        fprintf(stderr, "invaild parameter");
        return -1;
    }

    memset(usr_bsm, 0, sizeof(v2x_msg_bsm_t));

    usr_bsm->msgcount         = msgcount % MAX_BSM_MSG_COUNT;
    msgcount++;

    usr_bsm->temp_id          = id;
    usr_bsm->unix_time        = unixtime_send;
    usr_bsm->vehicletype      = 4;
    usr_bsm->sve_evdsc_extent = 4;

    usr_bsm->brakeboost = 1;
    usr_bsm->auxbrakes  = 1;

    /*
     * Vehicle width and length here are in meters
     */
    usr_bsm->width  = 2.5;
    usr_bsm->length = 4.0;

    //  if_ctx->type |= BSM_PATHPREDICTION;

    /* events */
    usr_bsm->event_hazardlights           = 1;
    usr_bsm->event_absactivate            = 0;
    usr_bsm->event_tractionctrlloss       = 1;
    usr_bsm->event_stabilityctrlactivated = 0;
    usr_bsm->event_hazardbraking          = 0;
    usr_bsm->event_airbag                 = 1;

    /* ext-lights */
    usr_bsm->lowbeam          = 1;
    usr_bsm->highbeam         = 0;
    usr_bsm->leftturnsignal   = 0;
    usr_bsm->rightturnsignal  = 0;
    usr_bsm->hazardlights     = 1;
    usr_bsm->autolightcontrol = 1;
    usr_bsm->dtimerunlights   = 1;
    usr_bsm->foglights        = 1;
    usr_bsm->parkinglights    = 0;

    /* wipers */
    usr_bsm->wipers_swfnt        = 0;
    usr_bsm->wipers_rtfnt        = 0;
    usr_bsm->wipers_swrear       = 0;
    usr_bsm->wipers_rtrear       = 0;

    /* Vehicle Data */
    usr_bsm->vehiclemass        = 4500;
    usr_bsm->vehicleheight      = 6.30;
    usr_bsm->bumperheight_front = 0.87;
    usr_bsm->bumperheight_rear  = 0.85;

    //  if_ctx->type |= BSM_VEHICLESTATUS;

    /*
     * Lat and Long are filled by GPS
     */
    usr_bsm->positionalaccuracy[0] = 1.23;
    usr_bsm->positionalaccuracy[1] = 1.12;
    usr_bsm->positionalaccuracy[2] = 1.11;
    usr_bsm->transmissionstate     = 5;

    /*
     * Speed and heading are filled by GPS
     */
    usr_bsm->angle               = angle_recv;
    usr_bsm->longaccel           = 1.02;
    usr_bsm->lataccel            = 1.12;
    usr_bsm->vertaccel           = 4.22;
    usr_bsm->yawrate             = 1.32;
    usr_bsm->wheelbrake          = 1;
    usr_bsm->wheelbrakeavailable = 1;
    usr_bsm->sparebit            = 1;
    usr_bsm->traction            = 1;
    usr_bsm->abs                 = 0;
    usr_bsm->stabilitycontrol    = 1;

    //usr_bsm->latitude  = 30.495527;
		//test 20190328 zhongyuanliu
		usr_bsm->latitude  = 10;
    usr_bsm->longitude = 114.17543;
    usr_bsm->heading   = 0;
    usr_bsm->elevation = 100;
    usr_bsm->speed     = 1 * 3.6;

    return 0;
}


void v2x_bsm_tx_handle(union sigval sig)
{
    v2x_msg_bsm_t   user_bsm;
    int ret         = -1;

    memset(&user_bsm, 0, sizeof(user_bsm));
    ret = v2x_fill_bsm_msg(&user_bsm);
    if(0 != ret)
    {
        fprintf(stderr, "user bsm fill failed\n");
        return;
    }

    ret = mde_v2x_send_bsm(&user_bsm);
    if(0 != ret)
    {
        fprintf(stderr, "bsm send failed\n");
        return;
    }

    fprintf(stderr, "txmsg-BSM: send msg successed, angle %f, time %f\n", user_bsm.angle, unixtime_send);

}

void v2x_bsm_recv_handle(v2x_msg_bsm_t* user_bsm)
{
    fprintf(stderr, "rxmsg-BSM: recv msg successed, latitude %f, time %f\n", user_bsm->latitude, user_bsm->unix_time);
		latitude_recv=user_bsm->latitude;
		unixtime_recv=user_bsm->unix_time;
}

int main()
{
	//****************************TCP CODE START**************************
	char buf[MAXRECVLEN];
	int listenfd, connectfd;   /* socket descriptors */
	struct sockaddr_in server; /* server's address information */
	struct sockaddr_in client; /* client's address information */
	socklen_t addrlen;
	/* Create TCP socket */
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
			/* handle exception */
			perror("socket() error. Failed to initiate a socket");
			exit(1);
	}

	/* set socket option */
	int opt = SO_REUSEADDR;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	bzero(&server, sizeof(server));

	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(listenfd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
			/* handle exception */
			perror("Bind() error.");
			exit(1);
	}

	if(listen(listenfd, BACKLOG) == -1)
	{
			perror("listen() error. \n");
			exit(1);
	}

	addrlen = sizeof(client);
	//**************BSM START*******************************
	int ret = -1;
	timer_t bsm_send_timer;

	ret = mde_v2x_init(LOG_CONFIG_PATH, MOUDLE_NAME);
	if(0 != ret)
	{
			fprintf(stderr, "v2x init faild\n");
			return -1;
	}
	fprintf(stderr, "v2x init success\n");
	//************BSM END************************************
	while(1){
			if((connectfd=accept(listenfd,(struct sockaddr *)&client, &addrlen))==-1)
				 {
					perror("accept() error. \n");
					exit(1);
				 }

			struct timeval tv;
			gettimeofday(&tv, NULL);
				 printf("You got a connection from client's ip %s, port %d at time %ld.%ld\n",inet_ntoa(client.sin_addr),htons(client.sin_port), tv.tv_sec,tv.tv_usec);

			int iret=-1;
			ret = mde_v2x_register_bsm(v2x_bsm_recv_handle);
			if(0 != ret)
			{
					fprintf(stderr, "bsm register faild\n");
					return -1;
			}
			fprintf(stderr, "bsm register success\n");

			ret = v2x_create_timer(&bsm_send_timer, v2x_bsm_tx_handle, NULL);
			if(0 != ret)
			{
					fprintf(stderr, "timer create failed\n");
					return -1;
			}
			ret = v2x_set_timer(bsm_send_timer, 100);
			if(0 != ret)
			{
					fprintf(stderr, "timer set failed\n");
					return -1;
			}
			int count_calibr=0;
			while(1)
			{
					iret = recv(connectfd, buf, MAXRECVLEN, 0);
					if(iret>0)
					{
						//decode the recived message
							char *p;
 							p = strtok(buf, ",");
	 						angle_recv=strtod(p,NULL);
	 						p = strtok(NULL, ",");
							long client_sec = atoi(p);
							p = strtok(NULL, ",");
							long client_usec = atoi(p);
							printf("angle is : %f\n", angle_recv);
							unixtime_send = (double)client_sec+((double)client_usec)/1000000;

				}
					else
					{
							close(connectfd);
							break;
					}
					//send the latitude back together with the time
					char sendBuf[100];
					if (count_calibr>=0)
					{
						gettimeofday(&tv, NULL);
						sprintf(sendBuf, "%f,%ld,%ld",latitude_recv,tv.tv_sec,tv.tv_usec);
										/* print client's ip and port */
						send(connectfd, sendBuf,strlen(sendBuf)+1, 0);
						count_calibr=1;
					}
					else
					{
						sprintf(sendBuf, "%f,%ld,%ld",latitude_recv,(long)unixtime_recv,(long)((unixtime_recv-(long)unixtime_recv)*1000000));
										/* print client's ip and port */
						send(connectfd, sendBuf,strlen(sendBuf)+1, 0);

			}
			}
/* send to the client welcome message */
			}
	close(listenfd); /* close listenfd */
	printf("tcp end");
	return 0;
	//*************************TCP CODE END*********************************

		/*
		int ret = -1;
    timer_t bsm_send_timer;

    ret = mde_v2x_init(LOG_CONFIG_PATH, MOUDLE_NAME);
    if(0 != ret)
    {
        fprintf(stderr, "v2x init faild\n");
        return -1;
    }
    fprintf(stderr, "v2x init success\n");

    ret = mde_v2x_register_bsm(v2x_bsm_recv_handle);
    if(0 != ret)
    {
        fprintf(stderr, "bsm register faild\n");
        return -1;
    }
    fprintf(stderr, "bsm register success\n");


    ret = v2x_create_timer(&bsm_send_timer, v2x_bsm_tx_handle, NULL);
    if(0 != ret)
    {
        fprintf(stderr, "timer create failed\n");
        return -1;
    }
    ret = v2x_set_timer(bsm_send_timer, 100);
    if(0 != ret)
    {
        fprintf(stderr, "timer set failed\n");
        return -1;
    }

    while(1)
    {
        sleep(10);
    }

	return 0;
	*/
}
