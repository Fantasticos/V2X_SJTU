#include <v2x_api.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LOG_CONFIG_PATH     "/usr/local/mocar_log.conf"
#define MOUDLE_NAME         "rsa"

#define MAX_INTERSECTION_LANE_NUM   (32)

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

int v2x_msg_fill_rsa(v2x_msg_rsa_t *user_rsa)
{
    static unsigned int count = 0;

    if(NULL == user_rsa)
    {
        return -1;
    }
    
    user_rsa->msg_count = count%128;
    count++;

    user_rsa->min_of_year = 1000;
    user_rsa->type_event = 513;
    user_rsa->full_posvec.pos3d.latitude = 30.12345;
    user_rsa->full_posvec.pos3d.longitude = 120.12345;

    return 0;  
}

void v2x_rsa_tx_handle(union sigval sig)
{
    v2x_msg_rsa_t  user_rsa;
    int ret         = -1;

    memset(&user_rsa, 0, sizeof(user_rsa));
    ret = v2x_msg_fill_rsa(&user_rsa);
    if(0 != ret)
    {
        fprintf(stderr, "user rsa fill failed!\n");
        return; 
    }

    ret = mde_v2x_send_rsa(&user_rsa);
    if(0 != ret)
    {
        fprintf(stderr, "rsa send failed!\n");
        return; 
    }

    fprintf(stderr, "txmsg-RSA: send msg successed\n"); 
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

void v2x_rsa_recv_handle(v2x_msg_rsa_t* user_rsa)
{
    fprintf(stderr, "rxmsg-RSA: recv msg successed\n");  
}

int main()
{
    int ret = -1;
    timer_t rsa_send_timer;

    ret = mde_v2x_init(LOG_CONFIG_PATH, MOUDLE_NAME);
    if(0 != ret)
    {
        fprintf(stderr, "v2x init faild\n");
        return -1;
    }
    fprintf(stderr, "v2x init success\n");

    ret = mde_v2x_register_rsa(v2x_rsa_recv_handle);
    if(0 != ret)
    {
        fprintf(stderr, "rsa register faild\n");
        return -1;
    }
    fprintf(stderr, "rsa register success\n");


    ret = v2x_create_timer(&rsa_send_timer, v2x_rsa_tx_handle, NULL);
    if(0 != ret)
    {
        fprintf(stderr, "timer create failed\n");
        return -1;   
    }
    ret = v2x_set_timer(rsa_send_timer, 200);
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
