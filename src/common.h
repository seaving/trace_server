#ifndef __COMMON_H__
#define __COMMON_H__

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

#include <sys/time.h>

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

#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/thread.h>


#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) > (b) ? (b) : (a))


#endif


