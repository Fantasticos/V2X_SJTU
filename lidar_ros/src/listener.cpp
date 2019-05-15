#include <ros/ros.h>
#include <lidar_ros/lidar_grad.h>
#include <std_msgs/Float64.h>   //ROS自带的浮点类型，类似 float，但是不同
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>  /* netdb is necessary for struct hostent */

#define PORT 4321   /* server port */

#define MAXDATASIZE 100
/*
msg defination:
float64 x
float64 y
float64 theta
float64 vel
int64 time_sec
int64 time_usec
*/
double x = 0;
double y = 0;
double theta = 0;
double vel = 0;
long lidar_time_sec = 0;
long lidar_time_usec = 0;
void gpsCallback(const lidar_ros::lidar_grad::ConstPtr &msg){   //回调函数，参数类型为 ConstPtr 类型的指针，它被定义在之前编译生成的 gps.h 中，指向 gps 的消息
   x = msg->x;  //之所以是 distance.data，是因为 Floa32 是一个结构体，成员变量 data 才存储着值
   y = msg->x;
   theta = msg->theta;
   vel = msg->vel;
   lidar_time_sec = msg->time_sec;
   lidar_time_usec = msg->time_usec;
   //ROS_INFO("x is : %f", x);
}

int main(int argc,char** argv){
   ros::init(argc,argv,"listener");
   ros::NodeHandle n;
   ros::Subscriber sub = n.subscribe("lidar_info",1,gpsCallback);  //
    int sockfd, num;    /* files descriptors */
    char buf[MAXDATASIZE];    /* buf will store received text */
    struct hostent *he;    /* structure that will get information about remote host */
    struct sockaddr_in server;

    if (argc != 2)
    {
        printf("Usage: %s <IP Address>\n",argv[0]);
        exit(1);
    }

    if((he=gethostbyname(argv[1]))==NULL)
    {
        printf("gethostbyname() error\n");
        exit(1);
    }

    if((sockfd=socket(AF_INET,SOCK_STREAM, 0))==-1)
    {
        printf("socket() error\n");
        exit(1);
    }
    bzero(&server,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr = *((struct in_addr *)he->h_addr);
    if(connect(sockfd, (struct sockaddr *)&server, sizeof(server))==-1)
    {
        printf("connect() error\n");
        exit(1);
    }
    //the message to be send from PC
    struct timeval tv;
    long dtime=0;
    double angle_recv = 0;
    long server_sec = 0;
    long server_usec = 0;
    long server_time = 0;
    long client_sec = 0;
    long client_usec = 0;
    long client_time_send = 0;
    long client_time_recv = 0;
    int count_calibr=0;

    int tcp_freq = 10;
    gettimeofday(&tv, NULL);
    long last_time = tv.tv_sec*1000000+tv.tv_usec;
    long curr_time = 0;
    long tar_time = (tv.tv_sec+1)*1000000;

    while (1)
    {
      //Set the time stamp
      gettimeofday(&tv, NULL);
      curr_time = tv.tv_sec*1000000+tv.tv_usec;
      if (last_time<=tar_time&&curr_time>=tar_time)
      {
        tar_time+=1000000/tcp_freq;
        last_time = curr_time;
          printf("time now %ld",curr_time);
      ros::spinOnce();
      //Set the time stamp
      gettimeofday(&tv, NULL);
      client_time_send=lidar_time_sec*1000000+lidar_time_usec+dtime;
      //printf("%ld\n", client_time_send);
      client_sec=client_time_send/1000000;
      client_usec=client_time_send-1000000*(client_time_send/1000000);
      printf("Calibrated time now is %ld.%ld\n", client_sec,client_usec);
      //send message together with time stamp
      //latitude++;
      char sendBuf[100];
      sprintf(sendBuf, "%f,%f,%f,%f,%ld,%ld",x,y,theta,vel,client_sec,client_usec);
      if((num=send(sockfd,sendBuf,strlen(sendBuf)+1,0))==-1){
          printf("send() error\n");
          exit(1);
      }
      if((num=recv(sockfd,buf,MAXDATASIZE,0))==-1)
      {
          printf("recv() error\n");
          exit(1);
      }
      buf[num-1]='\0';
      //decode the recived message
      char *p;
      p = strtok(buf, ",");
      angle_recv=strtod(p,NULL);
      p = strtok(NULL, ",");
      server_sec = atoi(p);
      p = strtok(NULL, ",");
      server_usec = atoi(p);
      server_time = server_sec*1000000+server_usec;
      //for the first loop, server time is the time from RSU, for the latter ones,it is from OBPC
      if (count_calibr==0)
      {
      client_time_send=tv.tv_sec*1000000+tv.tv_usec;
      gettimeofday(&tv, NULL);
      client_time_recv=tv.tv_sec*1000000+tv.tv_usec;
      dtime=(2*server_time-client_time_recv-client_time_send)/2;//calculate the time difference between the two device
      count_calibr=1;
      printf("%ld\n", dtime);
      }
      //else
      //printf("received vehicle time is : %ld.%ld\n",server_sec,server_usec);


      }
      usleep(1000);
    }

    close(sockfd);
    printf("tcp end");
    return 0;
   return 0;
}
