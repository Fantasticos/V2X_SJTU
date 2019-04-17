//From CSDN https://blog.csdn.net/tbadolph/article/details/76095814
#include <stdio.h>  
#include <string.h>  
#include <stdlib.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <sys/un.h>  
#include <arpa/inet.h>  
#include <netinet/in.h>  

int main (int argc, char * argv[])  
{  
    unsigned int myport;
    int client_sockfd;
      //创建套接字,即创建socket   

        if (argc !=3 )
    {
        printf("Please Input %s  IP Port\n",argv[1]);
        exit(EXIT_FAILURE);    
    }

       client_sockfd = socket(AF_INET, SOCK_STREAM, 0);   
      if(client_sockfd < 0)  
      {  
          perror("socket");  
          return 1;  
      }  
 
      //绑定信息，即命名socket   
      struct sockaddr_in addr;   
      addr.sin_family = AF_INET;   
      addr.sin_port = htons(atoi(argv[2]));   
      //inet_addr函数将用点分十进制字符串表示的IPv4地址转化为用网络   
      //字节序整数表示的IPv4地址   
      addr.sin_addr.s_addr = inet_addr(argv[1]);   
 
      //不需要监听  
 
      //发起连接  
//    struct sockaddr_in peer;  
      socklen_t addr_len = sizeof(addr);  
      int connect_fd = connect(client_sockfd, (struct sockaddr*)&addr, addr_len);  
      if(connect_fd < 0)  
      {  
          perror("connect");  
          return 2;  
      }  
      char buf[1024];  
 
      while(1)  
      {  
          memset(buf, '\0', sizeof(buf));  
          printf("client please enter: ");  
          fflush(stdout);  
          ssize_t size = read(0, buf, sizeof(buf) - 1);  
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
              return 4;  
          }  
         // printf("client: %s\n", buf);  
          write(client_sockfd, buf, strlen(buf));  
          size = read(client_sockfd, buf, sizeof(buf));  
          if(size > 0)  
          {  
              buf[size] = '\0';  
          }  
          else if(size == 0)  
          {  
              printf("read is done...\n");  
              break;  
          }  
          else   
          {  
              perror("read");  
              return 5;  
          }  
          printf("server: %s\n", buf);  
       }  
      close(client_sockfd);  
      return 0;  
}

