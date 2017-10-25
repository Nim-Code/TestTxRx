#ifndef ICMPRECV_H
#define ICMPRECV_H
#define UDP_MULTICAST6_BASE_PORT	(u_long)9900
#define UDP_MULTICAST_BASE_PORT	UDP_MULTICAST6_BASE_PORT
#endif
int icmpRecv(int icmpfd);


