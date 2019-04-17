#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
 
const int port = 8888;
const char* ip = "192.168.253.1"; //服务器端IP
 
int main()
{
	  int ser_sock = socket(AF_INET, SOCK_STREAM, 0); //创建套接字,即创建socket 
	  if(ser_sock < 0)
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
	  
	  if(bind(ser_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) 
	  {	            
	       perror("bind"); 
	       return 2; 	   
	  } 
 
	  int listen_sock = listen(ser_sock, 5); //监听socket
	  if(listen_sock < 0)
	  {
		  perror("listen");
		  return 3;
	  }
 
      /*接受连接*/
	  struct sockaddr_in peer;
	  socklen_t peer_len;
	  char buf[1024];
	  int accept_fd = accept(ser_sock, (struct sockaddr*)&peer, &peer_len);
 
	  if(accept_fd < 0)
	  {
		  perror("accept");
		  return 4;
	  }
	  else
	  {
		  printf("connect from %s, port %d \n", inet_ntop(AF_INET,&peer.sin_addr, buf, 1024), ntohs(peer.sin_port));
	  }
 
	  while(1)
	  {
		  memset(buf, '\0', sizeof(buf));
		  ssize_t size = read(accept_fd, buf, sizeof(buf) - 1);
		  if(size > 0)
		  {
			  printf("client: %s\n", buf);
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
          printf("server:");
		  fflush(stdout);
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
		  write(accept_fd, buf, strlen(buf));
	  }
		close(ser_sock);
	  return 0;
}
