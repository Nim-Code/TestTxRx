#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include "tpipv6.h"
#include <time.h>
#include <io.h>
#else
#include <sys/param.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <linux/in.h>
#include <linux/ip.h>
#ifndef LINUX
#include <arpa/inet.h>
#endif
#include <sys/uio.h>
#include <linux/types.h>
#include <net/if.h>
/* #include <netinet/ip.h> */
/* #include <netinet/in.h> */
#include <errno.h>
#endif
#ifndef IPPROTO_IPV6
#endif
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <stddef.h>
#include <memory.h>
#include <malloc.h>

#define UDP_MULTICAST6_BASE_PORT	(u_long)9900
#define UDP_MULTICAST_BASE_PORT	UDP_MULTICAST6_BASE_PORT
#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif
#ifndef u_char
#define u_char unsigned char
#endif
#ifndef u_long
#define u_long unsigned long
#endif
#ifndef u_int
#define u_int unsigned int
#endif
#ifndef u_short
#define u_short unsigned short
#endif

struct  message{
                unsigned int	msgNumber;
                char    	data[10000];
                };

