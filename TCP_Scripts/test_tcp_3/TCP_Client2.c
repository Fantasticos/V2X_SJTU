#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <math.h>
#include <netdb.h>  /* netdb is necessary for struct hostent */

#define PORT 4321   /* server port */

#define MAXDATASIZE 100


int main(int argc, char *argv[])
{
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
    double angle=1.1;
    struct timeval tv;
    long dtime=0;
    double x_recv = 0;
    double y_recv = 0;
    double x_est = 0;
    double y_est = 0;
    double theta_recv = 0;
    double vel_recv = 0;
    long server_sec = 0;
    long server_usec = 0;
    long server_time = 0;
    long client_sec = 0;
    long client_usec = 0;
    long client_time_send = 0;
    long client_time_recv = 0;
    int count_calibr=0;
    long commu_delay =0;

    int tcp_freq = 10;
    gettimeofday(&tv, NULL);
    long last_time = tv.tv_sec*1000000+tv.tv_usec;
    long curr_time = 0;
    long tar_time = (tv.tv_sec+1)*1000000+60000;

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
      client_time_send=curr_time+dtime;

      client_sec=client_time_send/1000000;
      client_usec=client_time_send-1000000*(client_time_send/1000000);
      //printf("Calibrated time now is %ld.%ld\n", client_sec,client_usec);
      //send message together with time stamp
      //angle++;
      char sendBuf[100];
      sprintf(sendBuf, "%f,%ld,%ld",angle,client_sec,client_usec);
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
      x_recv=strtod(p,NULL);
      p = strtok(NULL, ",");
      y_recv=strtod(p,NULL);
      p = strtok(NULL, ",");
      theta_recv=strtod(p,NULL);
      p = strtok(NULL, ",");
      vel_recv=strtod(p,NULL);
      p = strtok(NULL, ",");
      server_sec = atoi(p);
      p = strtok(NULL, ",");
      server_usec = atoi(p);
      server_time = server_sec*1000000+server_usec;
      client_time_send=tv.tv_sec*1000000+tv.tv_usec;
      gettimeofday(&tv, NULL);
      client_time_recv=tv.tv_sec*1000000+tv.tv_usec;
      if (count_calibr==0)
      {
      dtime=(2*server_time-client_time_recv-client_time_send)/2;//calculate the time difference between the two device
      printf("%ld\n",dtime);
      count_calibr=1;
      }
      else
      {
          commu_delay=client_time_recv-server_time+dtime;
          x_est = x_recv + vel_recv*cos(theta_recv*3.1415/180) * commu_delay/1000000;
          y_est = y_recv + vel_recv*sin(theta_recv*3.1415/180) * commu_delay/1000000;
          //printf("received lidar time is : %ld\n",server_time);
          printf("estimate x is : %f\n",x_est);
          printf("estimate y is : %f\n",y_est);
          printf("received theta is : %f\n",theta_recv);
          printf("received vel is : %f\n",vel_recv);
          //printf("Client_time= %ld\n",client_time_recv+dtime);
          printf("delay is %ld\n",commu_delay);
      }




    }
          usleep(1000);
    }
    close(sockfd);
    printf("tcp end");
    return 0;
}
