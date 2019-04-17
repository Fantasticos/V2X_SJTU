#include <v2x_api.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LOG_CONFIG_PATH     "/usr/local/mocar_log.conf"
#define MOUDLE_NAME         "rtcm"

#define MAX_INTERSECTION_LANE_NUM   (32)

static unsigned char test_rtcm_data[] = {0x59,0x7E,0x7D,0x7F,0x71,0x72,0x43,0x51,0x7B,0x4A,0x40,0x41,0x5E,0x46,0x65,0x40,0x6C,0x6F,
                                         0x77,0x4F,0x40,0x64,0x48,0x54,0x50,0x42,0x43,0x4E,0x40,0x69,0x69,0x43,0x40,0x59,0x4B,0x70,0x7D,
                                         0x55,0x5A,0x65,0x76,0x49,0x74,0x73,0x73,0x50,0x7F,0x4B,0x7B,0x4D,0x58,0x54,0x55,0x55,0x75};

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

int minute_of_the_year(struct tm* utc_time)
{
    int minute = 0;

    minute = (utc_time->tm_yday * 24 * 60) + (utc_time->tm_hour * 60) + utc_time->tm_min;
    return minute;
}

int v2x_msg_fill_rtcm(v2x_msg_rtcm_t* rtcm)
{
    static int msgcount = 0;

    if (rtcm == NULL)
    {
        return -1;
    }

    rtcm->msg_count = msgcount % 128;
    msgcount++;

    rtcm->min_of_year = 1000;
    rtcm->rev = J2735_RTCM_REV2;
    rtcm->rtcm_msg_list_count = 1;
    rtcm->rtcm_msg_list[0].rtcm_msg_len = sizeof(test_rtcm_data);
    memcpy(rtcm->rtcm_msg_list[0].rtcm_msg, test_rtcm_data, sizeof(test_rtcm_data));

    return 0;
}

void v2x_rtcm_tx_handle(union sigval sig)
{
    v2x_msg_rtcm_t  user_rtcm;
    int ret         = -1;

    memset(&user_rtcm, 0, sizeof(user_rtcm));
    ret = v2x_msg_fill_rtcm(&user_rtcm);
    if(0 != ret)
    {
        fprintf(stderr, "user rtcm fill failed!\n");
        return; 
    }

    ret = mde_v2x_send_rtcm(&user_rtcm);
    if(0 != ret)
    {
        fprintf(stderr, "rtcm send failed!\n");
        return; 
    }

    fprintf(stderr, "txmsg-RTCM: send msg successed\n"); 
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

void v2x_rtcm_recv_handle(v2x_msg_rtcm_t* user_rtcm)
{
    fprintf(stderr, "rxmsg-RTCM: recv msg successed\n");  
}

int main()
{
    int ret = -1;
    timer_t rtcm_send_timer;

    ret = mde_v2x_init(LOG_CONFIG_PATH, MOUDLE_NAME);
    if(0 != ret)
    {
        fprintf(stderr, "v2x init faild\n");
        return -1;
    }
    fprintf(stderr, "v2x init success\n");

    ret = mde_v2x_register_rtcm(v2x_rtcm_recv_handle);
    if(0 != ret)
    {
        fprintf(stderr, "rtcm register faild\n");
        return -1;
    }
    fprintf(stderr, "rtcm register success\n");


    ret = v2x_create_timer(&rtcm_send_timer, v2x_rtcm_tx_handle, NULL);
    if(0 != ret)
    {
        fprintf(stderr, "timer create failed\n");
        return -1;   
    }
    ret = v2x_set_timer(rtcm_send_timer, 200);
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
}
