#include <v2x_api.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LOG_CONFIG_PATH     "/usr/local/mocar_log.conf"
#define MOUDLE_NAME         "srm"

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

int v2x_msg_fill_srm(v2x_msg_srm_t *srm)
{
    int i=0;
    srm->bit_mask = 0x00 | 0x20;
    srm->second = 100;
    srm->requestor.bit_mask = 0x00 | 0x80;
    srm->requestor.id_choice = 2;
    srm->requestor.id.station_id = 6666;
    srm->requestor.requestor_type_bit_mask = 0x00 | 0x40;
    srm->requestor.requestor_type.role = 0;
    srm->requestor.requestor_type.req_importance_level = 14;

    srm->srm_list_count = 1;

    for (i=0; i<srm->srm_list_count; i++)
    {
        srm->srmlist[i].bit_mask = 0x00;
        srm->srmlist[i].signal_request.bit_mask = 0x00;
        srm->srmlist[i].signal_request.id_bit_mask = 0x00;
        srm->srmlist[i].signal_request.iid = 258 ;
        srm->srmlist[i].signal_request.request_id = 1;
        srm->srmlist[i].signal_request.req_type = 1;
        srm->srmlist[i].signal_request.inbound_accesspoint_choice =1;
        srm->srmlist[i].signal_request.in_bound_access_point.lane_id = 1;
    }

    return 0;
}

void v2x_srm_tx_handle(union sigval sig)
{
    v2x_msg_srm_t  user_srm;
    int ret         = -1;

    memset(&user_srm, 0, sizeof(user_srm));
    ret = v2x_msg_fill_srm(&user_srm);
    if(0 != ret)
    {
        fprintf(stderr, "user srm fill failed!\n");
        return; 
    }

    ret = mde_v2x_send_srm(&user_srm);
    if(0 != ret)
    {
        fprintf(stderr, "srm send failed!\n");
        return; 
    }

    fprintf(stderr, "txmsg-SRM: send msg successed\n"); 
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

void v2x_srm_recv_handle(v2x_msg_srm_t* user_srm)
{
    fprintf(stderr, "rxmsg-SRM: recv msg successed\n");  
}

int main()
{
    int ret = -1;
    timer_t srm_send_timer;

    ret = mde_v2x_init(LOG_CONFIG_PATH, MOUDLE_NAME);
    if(0 != ret)
    {
        fprintf(stderr, "v2x init faild\n");
        return -1;
    }
    fprintf(stderr, "v2x init success\n");

    ret = mde_v2x_register_srm(v2x_srm_recv_handle);
    if(0 != ret)
    {
        fprintf(stderr, "srm register faild\n");
        return -1;
    }
    fprintf(stderr, "srm register success\n");


    ret = v2x_create_timer(&srm_send_timer, v2x_srm_tx_handle, NULL);
    if(0 != ret)
    {
        fprintf(stderr, "timer create failed\n");
        return -1;   
    }
    ret = v2x_set_timer(srm_send_timer, 200);
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
