#include <v2x_api.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LOG_CONFIG_PATH     "/usr/local/mocar_log.conf"
#define MOUDLE_NAME         "tim"

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

int v2x_msg_fill_tim(v2x_msg_tim_t* tim)
{
    static unsigned int msg_count = 0;
    mde_DATAFRAME_t* dataframe = NULL;
    int i = 0, j = 0;
    uint32_t*        adv_codes = NULL;

    if(NULL == tim)
    {
        return -1;
    }

    tim->msg_count = msg_count%128;
    tim->unique_id_len = 9;
    snprintf(tim->unique_id, 18, "%d", 12345678);
    tim->num_dataframes = 1;
    
    dataframe = calloc(tim->num_dataframes, sizeof(mde_DATAFRAME_t));
    if(NULL == dataframe)
    {
        return -1;
    } 

    for(i=0; i<tim->num_dataframes; i++)
    {
        dataframe[i].content_type    = 1;
        dataframe[i].further_info_id = 0;
        dataframe[i].pos3d.latitude  = 30.123456;
        dataframe[i].pos3d.longitude = 120.123456;
        dataframe[i].extent          = 3;
        dataframe[i].num_advisories  = 1;
        adv_codes                        = calloc(dataframe[i].num_advisories, sizeof(uint32_t));
        if (!adv_codes)
        {
            return -1;
        }
        for (j = 0; j < dataframe[i].num_advisories; j++)
        {
            adv_codes[j] = 30;
        }
        dataframe[i].advisory_codes = adv_codes;
    }

    tim->dataframes = dataframe;

    return 0;
}

void v2x_tim_tx_handle(union sigval sig)
{
    v2x_msg_tim_t  user_tim;
    int ret         = -1;

    memset(&user_tim, 0, sizeof(user_tim));
    ret = v2x_msg_fill_tim(&user_tim);
    if(0 != ret)
    {
        fprintf(stderr, "user tim fill failed!\n");
        mde_v2x_msg_tim_free(&user_tim);
        return; 
    }

    ret = mde_v2x_send_tim(&user_tim);
    if(0 != ret)
    {
        fprintf(stderr, "tim send failed!\n");
        mde_v2x_msg_tim_free(&user_tim);
        return; 
    }

    mde_v2x_msg_tim_free(&user_tim);

    fprintf(stderr, "txmsg-TIM: send msg successed\n"); 
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

void v2x_tim_recv_handle(v2x_msg_tim_t* user_tim)
{
    fprintf(stderr, "rxmsg-TIM: recv msg successed\n");  
}

int main()
{
    int ret = -1;
    timer_t tim_send_timer;

    ret = mde_v2x_init(LOG_CONFIG_PATH, MOUDLE_NAME);
    if(0 != ret)
    {
        fprintf(stderr, "v2x init faild\n");
        return -1;
    }
    fprintf(stderr, "v2x init success\n");

    ret = mde_v2x_register_tim(v2x_tim_recv_handle);
    if(0 != ret)
    {
        fprintf(stderr, "tim register faild\n");
        return -1;
    }
    fprintf(stderr, "tim register success\n");


    ret = v2x_create_timer(&tim_send_timer, v2x_tim_tx_handle, NULL);
    if(0 != ret)
    {
        fprintf(stderr, "timer create failed\n");
        return -1;   
    }
    ret = v2x_set_timer(tim_send_timer, 200);
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
