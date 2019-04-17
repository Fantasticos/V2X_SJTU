#include <v2x_api.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LOG_CONFIG_PATH     "/usr/local/mocar_log.conf"
#define MOUDLE_NAME         "map"

#define DEFAULT_LANE_YPE            Mde_LaneType_vehicle
#define DEFAULT_VECHILE_LANE_ATTR   Mde_LaneAttributes_Vehicle_permissionOnRequest

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

int v2x_msg_fill_map(v2x_msg_map_t* map)
{
    static int msg_count = 0;
    int i = 0, j = 0, k= 0;

    if(NULL == map)
    {
        return -1;
    }

    map->msg_revision = 1;
    msg_count++;

    map ->n_geo_descr = 1;
    map->isec_geo_descr = calloc(map ->n_geo_descr, sizeof(mde_isec_geo_descr_t));
    if(NULL == map->isec_geo_descr)
    {
        return -1;
    }        

    for(i=0; i<map ->n_geo_descr; i++)
    {
        map->isec_geo_descr[i].intersection_id = 1+i;
        map->isec_geo_descr[i].msg_revision = 1+i;
        map->isec_geo_descr[i].ref_pt.latitude = 30.123456+i;
        map->isec_geo_descr[i].ref_pt.longitude = 120.123456+i;
        map->isec_geo_descr[i].ref_pt.elevation = 100+i;
        map->isec_geo_descr[i].lanewidth = 3.5;
        map->isec_geo_descr[i].n_lanelist = 1;

        
        map->isec_geo_descr[i].lanelist = calloc(map->isec_geo_descr[i].n_lanelist, sizeof(mde_lanelist_t));
        if(NULL == map->isec_geo_descr[i].lanelist)
        {
            return -1;
        }

        for(j=0; j<map->isec_geo_descr[i].n_lanelist; j++)
        {
            map->isec_geo_descr[i].lanelist[j].lane_id = j+1;
            map->isec_geo_descr[i].lanelist[j].phase = 1;
            map->isec_geo_descr[i].lanelist[j].man = MDE_MAN_STRAIGHT_ALLOWED;
            map->isec_geo_descr[i].lanelist[j].signal_group_id = 1;
            map->isec_geo_descr[i].lanelist[j].lane_attr.direction = MDE_LANE_DIRECTION_INGRESS;
            map->isec_geo_descr[i].lanelist[j].lane_attr.sharedlane = MDE_OVERLAP_LANE_DESCR;   

            map->isec_geo_descr[i].lanelist[j].n_nodepoint = 10;
            map->isec_geo_descr[i].lanelist[j].node_point = calloc(map->isec_geo_descr[i].lanelist[j].n_nodepoint, sizeof(mde_node_point_t));
            if(NULL == map->isec_geo_descr[i].lanelist[j].node_point)
            {
                return -1;
            }    
            for(k=0; k<map->isec_geo_descr[i].lanelist[j].n_nodepoint; k++)
            {
                map->isec_geo_descr[i].lanelist[j].node_point[k].node_id.node.latitude = 30.123456+i*0.0001;
                map->isec_geo_descr[i].lanelist[j].node_point[k].node_id.node.longitude = 120.123456+i*0.0001;
                map->isec_geo_descr[i].lanelist[j].node_point[k].node_id.node.elevation = 100+k;
                map->isec_geo_descr[i].lanelist[j].node_point[k].node_id.lanewidth = 3.5;
            }   
        }    
    }   

    return 0;
}

void v2x_map_tx_handle(union sigval sig)
{
    v2x_msg_map_t user_map;
    int ret = -1;

    memset(&user_map, 0, sizeof(user_map));
    ret = v2x_msg_fill_map(&user_map);
    if(0 != ret)
    {
        fprintf(stderr, "user map fill failed!\n");
        mde_v2x_msg_map_free(&user_map);
        return;
    }

    ret = mde_v2x_send_map(&user_map);
    if(0 != ret)
    {
        fprintf(stderr, "map send failed\n");
        mde_v2x_msg_map_free(&user_map);
        return;   
    }

    mde_v2x_msg_map_free(&user_map);

    fprintf(stderr, "txmsg-MAP: send msg successed\n");    
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

void v2x_map_recv_handle(v2x_msg_map_t* user_map)
{
    fprintf(stderr, "rxmsg-MAP: recv msg successed\n");  
}

int main()
{
    int ret = -1;
    timer_t map_send_timer;

    ret = mde_v2x_init(LOG_CONFIG_PATH, MOUDLE_NAME);
    if(0 != ret)
    {
        fprintf(stderr, "v2x init faild\n");
        return -1;
    }
    fprintf(stderr, "v2x init success\n");

    ret = mde_v2x_register_map(v2x_map_recv_handle);
    if(0 != ret)
    {
        fprintf(stderr, "map register faild\n");
        return -1;
    }
    fprintf(stderr, "map register success\n");


    ret = v2x_create_timer(&map_send_timer, v2x_map_tx_handle, NULL);
    if(0 != ret)
    {
        fprintf(stderr, "timer create failed\n");
        return -1;   
    }
    ret = v2x_set_timer(map_send_timer, 1000);
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
