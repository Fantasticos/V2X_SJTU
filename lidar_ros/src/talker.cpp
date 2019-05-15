#include <ros/ros.h>                         //类似 C 语言的 stdio.h
#include <lidar_ros/lidar_grad.h>                  //要用到 msg 中定义的数据类型
#include <time.h>
int main(int argc,char **argv){
   ros::init(argc,argv,"talker");            //解析参数，命名节点为 talker
   ros::NodeHandle nh;                       //创建句柄，相当于一套工具，可以实例化 node，并且对 node 进行操作
   lidar_ros::lidar_grad msg;                      //创建 gps 消息
   msg.x = 1.0;                              //设置 x 初值
   msg.y = 1.0;
   msg.theta = 2.0;                          //设置 y 初值
   msg.vel = 1;
   msg.time_sec = 10000000;
   msg.time_usec = 999999;                //设置 time 初值
   ros::Publisher pub = nh.advertise<lidar_ros::lidar_grad>("lidar_info",1);//创建 publisher 对象
   ros::Rate loop_rate(20.0);                 //创建 rate 对象，定义循环发布的频率，1 HZ
   struct timeval tv;
   while(ros::ok()){                         //只要没有关闭，一直循环
      msg.x = 1 * msg.x;                  //以指数增长，每隔 0.1s
      msg.y = 1 * msg.y;
      msg.theta = 1 * msg.theta;                  //以指数增长，每隔 0.1s
      msg.vel = 1 * msg.vel;
      gettimeofday(&tv, NULL);
      msg.time_sec = tv.tv_sec;
      msg.time_usec = tv.tv_usec;
      ROS_INFO("time stamp %ld.%ld",msg.time_sec,msg.time_usec); //打印函数，类似 printf()
      pub.publish(msg);                      //发布消息
      loop_rate.sleep();                     //根据定义的发布频率，sleep
   }
   return 0;
}
