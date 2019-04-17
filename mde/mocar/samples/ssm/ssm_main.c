#include <v2x_api.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LOG_CONFIG_PATH     "/usr/local/mocar_log.conf"
#define MOUDLE_NAME         "ssm"

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

int v2x_msg_fill_ssm(v2x_msg_ssm_t* ssm)
{
    int i = 0, k = 0;

    ssm->bit_mask = 0x00;
    ssm->second   = 100;

    ssm->sslist_len = 1;
    for (i = 0; i < ssm->sslist_len; i++)
    {
        ssm->sslist[i].bit_mask       = 0x00;
        ssm->sslist[i].sequenceNumber = 1;
        ssm->sslist[i].id.bit_mask    = 0x00;
        ssm->sslist[i].id.id          = 258;

        ssm->sslist[i].ss_pkg_list_len = 1;
        for (k = 0; k < ssm->sslist[i].ss_pkg_list_len; k++)
        {
            ssm->sslist[i].ss_pkg_list[k].bit_mask           = 0x00;
            ssm->sslist[i].ss_pkg_list[k].in_bound_on_choice = 1;

            ssm->sslist[i].ss_pkg_list[k].in_bound_access_point.lane_id = 1 + k;
            ssm->sslist[i].ss_pkg_list[k].prio_resp                     = 4;
            ssm->sslist[i].ss_pkg_list[k].signal_request.vehicle_id.station_id = 101;
        }
    }

    return 0;
}

void v2x_ssm_tx_handle(union sigval sig)
{
    v2x_msg_ssm_t  user_ssm;
    int ret         = -1;

    memset(&user_ssm, 0, sizeof(user_ssm));
    ret = v2x_msg_fill_ssm(&user_ssm);
    if(0 != ret)
    {
        fprintf(stderr, "user ssm fill failed!\n");
        return; 
    }

    ret = mde_v2x_send_ssm(&user_ssm);
    if(0 != ret)
    {
        fprintf(stderr, "ssm send failed!\n");
        return; 
    }

    fprintf(stderr, "txmsg-SSM: send msg successed\n"); 
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

void v2x_ssm_recv_handle(v2x_msg_ssm_t* user_ssm)
{
    fprintf(stderr, "rxmsg-SSM: recv msg successed\n");  
}

int main()
{
    int ret = -1;
    timer_t ssm_send_timer;

    ret = mde_v2x_init(LOG_CONFIG_PATH, MOUDLE_NAME);
    if(0 != ret)
    {
        fprintf(stderr, "v2x init faild\n");
        return -1;
    }
    fprintf(stderr, "v2x init success\n");

    ret = mde_v2x_register_ssm(v2x_ssm_recv_handle);
    if(0 != ret)
    {
        fprintf(stderr, "ssm register faild\n");
        return -1;
    }
    fprintf(stderr, "ssm register success\n");


    ret = v2x_create_timer(&ssm_send_timer, v2x_ssm_tx_handle, NULL);
    if(0 != ret)
    {
        fprintf(stderr, "timer create failed\n");
        return -1;   
    }
    ret = v2x_set_timer(ssm_send_timer, 200);
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
