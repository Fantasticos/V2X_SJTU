#include <v2x_api.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LOG_CONFIG_PATH     "/usr/local/mocar_log.conf"
#define MOUDLE_NAME         "spat"

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

int v2x_msg_fill_spat(v2x_msg_spat_t* spat)
{
    int i = 0;

    spat->n_intersections   = 1;
    spat->Intersection_list = calloc(spat->n_intersections, sizeof(mde_Intersection_t));
    if (!spat->Intersection_list)
    {
        fprintf(stderr, "SPAT: Intersection_list calloc fail\n");
        return -1;
    }
    spat->Intersection_list->intersection_id     = 1;
    spat->Intersection_list->num_movement_states = MAX_INTERSECTION_LANE_NUM;
    spat->Intersection_list->movement_states = calloc(spat->Intersection_list->num_movement_states, sizeof(mde_MovementState_t));
    if (!spat->Intersection_list->movement_states)
    {
        fprintf(stderr, "SPAT: movement_states calloc fail\n");
        return -1;
    }

    for(i=0; i<MAX_INTERSECTION_LANE_NUM; i++)
    {
        spat->Intersection_list->movement_states[i].num_movement_event = 1;
        spat->Intersection_list->movement_states[i].movement_event = calloc(spat->Intersection_list->movement_states->num_movement_event,
                                                                    sizeof(mde_MovementEventList_t));
        if(!spat->Intersection_list->movement_states->movement_event)
        {
            fprintf(stderr, "SPAT: movement_event calloc fail\n");
            return -1;
        }

        spat->Intersection_list->movement_states[i].movement_event->phase_state                  = 0;   //state
        spat->Intersection_list->movement_states[i].movement_event->time_change_info.likely_time = 12;  //timing
    }
 
    return 0;
}

void v2x_spat_tx_handle(union sigval sig)
{
    v2x_msg_spat_t  user_spat;
    int ret         = -1;

    memset(&user_spat, 0, sizeof(user_spat));
    ret = v2x_msg_fill_spat(&user_spat);
    if(0 != ret)
    {
        fprintf(stderr, "user spat fill failed!\n");
        mde_v2x_msg_spat_free(&user_spat);
        return; 
    }

    ret = mde_v2x_send_spat(&user_spat);
    if(0 != ret)
    {
        fprintf(stderr, "spat send failed!\n");
        mde_v2x_msg_spat_free(&user_spat);
        return; 
    }

    mde_v2x_msg_spat_free(&user_spat);

    fprintf(stderr, "txmsg-SPAT: send msg successed\n"); 
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

void v2x_spat_recv_handle(v2x_msg_spat_t* user_spat)
{
    fprintf(stderr, "rxmsg-SPAT: recv msg successed\n");  
}

int main()
{
    int ret = -1;
    timer_t spat_send_timer;

    ret = mde_v2x_init(LOG_CONFIG_PATH, MOUDLE_NAME);
    if(0 != ret)
    {
        fprintf(stderr, "v2x init faild\n");
        return -1;
    }
    fprintf(stderr, "v2x init success\n");

    ret = mde_v2x_register_spat(v2x_spat_recv_handle);
    if(0 != ret)
    {
        fprintf(stderr, "spat register faild\n");
        return -1;
    }
    fprintf(stderr, "spat register success\n");


    ret = v2x_create_timer(&spat_send_timer, v2x_spat_tx_handle, NULL);
    if(0 != ret)
    {
        fprintf(stderr, "timer create failed\n");
        return -1;   
    }
    ret = v2x_set_timer(spat_send_timer, 200);
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
