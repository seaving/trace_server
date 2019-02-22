#ifndef __SOCKET_H__
#define __SOCKET_H__

void socket_close(int socket);

int socket_setfd_noblock(int socket);
int socket_setfd_block(int socket);
int socket_listen(char *bindip, int listen_port, int listen_counts);

#endif

