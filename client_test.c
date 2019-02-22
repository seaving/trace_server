#include <arpa/inet.h>
#include <errno.h>
#include <libgen.h>
#include <netdb.h>
#include <resolv.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>

#include <sys/socket.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/file.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include <sys/param.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <pthread.h>

#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include <linux/sockios.h>
#include <sys/ioctl.h> 
#include <time.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <stdbool.h>
#include <stdarg.h>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/vfs.h>

#include <alloca.h>
#include <math.h>

#include <stdint.h>
#include <sys/inotify.h>

#define MAXLINE 1024
int success_count = 0;

void *test_work(void *arg)
{
	pthread_detach(pthread_self());
	//char *servInetAddr = "47.91.198.137";
	char *servInetAddr = "192.168.12.24";
	int socketfd;
	struct sockaddr_in sockaddr;
	char recvline[MAXLINE] = {0}, sendline[MAXLINE] = {0};
	int n;

	socketfd = socket(AF_INET,SOCK_STREAM,0);
	if (socketfd < 0)
	{
		printf("socket error %s errno: %d\n",strerror(errno),errno);
		return NULL;
	}
	
	memset(&sockaddr,0,sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(9110);
	inet_pton(AF_INET, servInetAddr, &sockaddr.sin_addr);
	if ((connect(socketfd, (struct sockaddr *) &sockaddr, sizeof(sockaddr))) < 0)
	{
		printf("connect error %s errno: %d\n",strerror(errno),errno);
		close(socketfd);
		return NULL;
	}

	//printf("send message to server\n");
	int i, j = 0;
	char *data = "ksdjflksdjkfjklsjelkwjeiowjflksdjlkfjsdlskdfjlsdjfsej";
	snprintf(sendline, sizeof(sendline) - 1, 
		"POST /index?Hw=&Ver=&Sn=&Ch=&Type=  HTTP/1.1 \r\n"
		"Content-Length: %d \r\n"
		"Host: %s \r\n"
		"\r\n"
		"%s", 
		//"MoonCake",
		//"9.0.1",
		//1,
		//4,
		(int)strlen(data),
		"log.skyroam.com.cn",
		data);

	for (i = 0; i < 1; i ++)
	{
		if ((send(socketfd, sendline, strlen(sendline), 0)) < 0)
		{
			printf("send mes error: %s errno : %d", strerror(errno), errno);
			close(socketfd);
			return NULL;
		}

		read(socketfd, recvline, MAXLINE);
		//printf("recv---> %s\n", recvline);
		if (strlen(recvline) > 0 && strstr(recvline, "400"))
			j ++;
		//sleep(1);
	}
	
	if (j == 1)
		success_count ++;
	
	close(socketfd);
	return NULL;
}

int main(int argc, char **argv)
{
	int i, k = 0;
	pthread_t thd;
	for (i = 0; i < 10000; i ++)
	{
		if (pthread_create(&thd, NULL, test_work, NULL) != 0)
		{
			
		}
		else
		{
			k ++;
		}
		
		//sleep(2);
	}
	
	while (1)
	{
		printf(">>>> worker count: %d, success_worker: %d\n", k, success_count);
		sleep(1);
	}
}



