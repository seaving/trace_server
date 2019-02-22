#include "includes.h"

void socket_close(int socket)
{
	if (socket > 0)
	{
		close(socket);
	}
}

int socket_setfd_noblock(int socket)
{
    int flag = fcntl(socket, F_GETFL, 0);
    if (flag < 0)
    {
		LOG_TRACE_PERROR("fcntl F_GETFL error");
		return -1;
    }
	if (fcntl(socket, F_SETFL, (flag | O_NONBLOCK)) < 0)
	{
		LOG_TRACE_PERROR("fcntl F_SETFL error");
		return -1;
	}
	
	return 0;
}

int socket_setfd_block(int socket)
{
    int flag = fcntl(socket, F_GETFL, 0);
    if (flag < 0)
    {
		LOG_TRACE_PERROR("fcntl F_GETFL error");
		return -1;
    }
	if (fcntl(socket, F_SETFL, (flag & ~O_NONBLOCK)) < 0)
	{
		LOG_TRACE_PERROR("fcntl F_SETFL error");
		return -1;
	}
	
	return 0;
}

int socket_listen(char *bindip, int listen_port, int listen_counts)
{
    struct sockaddr_in server_addr;
    int listenfd;
    int ret = -1;
    
    //初始化服务器地址
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(listen_port);
    if (bindip == NULL)
    {
    	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else
    {
		server_addr.sin_addr.s_addr = inet_addr(bindip);
    }
    
    //创建监听套接字
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd <= 0)
    {
        LOG_TRACE_PERROR("create socket error");
        return -1;
    }
    
    //绑定地址
    int len = sizeof(struct sockaddr);
    ret = bind(listenfd, (struct sockaddr *) &server_addr, len);
    if (ret == -1)
    {
        LOG_TRACE_PERROR("bind error, "
        	"lisent_fd=%d, port=%d", listenfd, listen_port);
        socket_close(listenfd);
        return -1;
    }
    
    //开始监听,设置最大连接请求
    ret = listen(listenfd, listen_counts);
    if (ret == -1)
    {
        LOG_TRACE_PERROR("listen error, "
        	"listenfd=%d, listen_counts=%d", listenfd, listen_counts);
        socket_close(listenfd);
        return -1;
    }
    
    return listenfd;
}


