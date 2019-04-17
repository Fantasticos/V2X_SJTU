//From CSDN https://blog.csdn.net/tbadolph/article/details/76095814
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<sys/socket.h>
#include<resolv.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<fcntl.h>   
 
int main (int argc, char * argv[])  
{  
    //创建套接字,即创建socket   
    int server_sockfd, client_sockfd;
    unsigned  int myport;
    socklen_t len;
    struct sockaddr_in server_addr, client_addr;
    char buf[1024];  
    //绑定信息，即命名socket    
    server_addr.sin_family = AF_INET;   
    server_addr.sin_port = htons(myport);
        
        if (argc !=3 )
    {
        printf("Please Input %s  IP Port\n",argv[0]);
        exit(EXIT_FAILURE);    
    }
    //inet_addr函数将用点分十进制字符串表示的IPv4地址转化为用网络字节序整数表示的IPv4地址
    if(argv[1])
        server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    else
        server_addr.sin_addr.s_addr = INADDR_ANY;
    if(argv[2])
        myport = atoi(argv[2]);
    else
        myport = 7575;
 
    //创建socket
    if((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)  
    {        
         perror("socket");  
         return 1;  
    }  
 
     //绑定
    if(bind(server_sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)   
    {                           
        perror("bind");   
        return 2;         
    }   
 
        //监听socket   
    if(listen(server_sockfd, 5) < 0)  
    {    
        perror("listen");  
        return 3;  
    }  
 
        
      //int accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen)  
      //sockfd参数是执行过listen系统调用的监听socket；addr参数用来获取被  
      //接受连接的远端socket地址，该socket地址的长度由addrlen参数指出  
      //accept成功时返回一个新的连接socket，该socket唯一的标识了被接受  
      //的这个连接，服务器可通过读写该socket来与被接受连接对应的客户端通信    
 
      len = sizeof(struct sockaddr);
      client_sockfd = accept(server_sockfd, (struct sockaddr*)&client_addr, (socklen_t *)&len);  
 
      if(client_sockfd < 0)  
      {  
          perror("accept");  
          return 4;  
      }  
      else  
      {  
          printf("connected with IP: %s  and port: %d\n", inet_ntop(AF_INET,&client_addr.sin_addr, buf, 1024), ntohs(client_addr.sin_port));  
          
      }  
    printf("Waiting client connecting……");
      while(1)  
      {  
          memset(buf, '\0', sizeof(buf));  
          ssize_t size = read(client_sockfd, buf, sizeof(buf) - 1);  
 
          if(size > 0)  
          {  
              printf("client#: %s\n", buf);  
          }  
          else if(size == 0)  
          {  
              printf("read is done...\n");  
              break;  
          }  
          else   
          {  
              perror("read");  
              break;  
          }  

          printf("server please enter: ");  
          fflush(stdout);  //刷新标准输出缓冲区，把输出缓冲区里的东西打印到标准输出设备上
          size = read(0, buf, sizeof(buf) - 1);  
          if(size > 0)  
          {  
              buf[size - 1] = '\0';  
          }  
          else if(size == 0)  
          {  
              printf("read is done...\n");  
              break;  
          }  
          else  
          {  
              perror("read");  
              break;  
          }     
          write(client_sockfd, buf, strlen(buf));  
      }  
        close(server_sockfd);  
      return 0;  
}  

