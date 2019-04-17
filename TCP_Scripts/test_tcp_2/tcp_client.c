#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
 
const int port = 8888;
const char* ip = "192.168.253.10"; //服务器IP
 
int main()
{
	  int clt_sock = socket(AF_INET, SOCK_STREAM, 0); //创建套接字,即创建socket 
	  if(clt_sock < 0)
	  {
		  perror("socket");
		  return 1;
	  }
 
	  struct sockaddr_in addr; //绑定信息，即命名socket 
	  addr.sin_family = AF_INET; 
	  addr.sin_port = htons(port); 
	  addr.sin_addr.s_addr = inet_addr(ip); 
	  /*inet_addr函数将用点分十进制字符串表示的
	  IPv4地址转化为用网络字节序整数表示的IPv4地址 */
 
	  //发起连接
	  socklen_t addr_len = sizeof(addr);
	  int connect_fd = connect(clt_sock, (struct sockaddr*)&addr, addr_len);
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
          write(clt_sock, buf, strlen(buf));
		  size = read(clt_sock, buf, sizeof(buf));
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
	  close(clt_sock);
	  return 0;
}
