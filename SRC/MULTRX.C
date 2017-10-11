#include "includes.h"

struct hostent *gethostbyname();
int net_join_multicast_group();
int multicast_join (int fd, const struct sockaddr_in sa_in, const char *ifname);
static int join_group();
char *convert_address();

/* #ifdef DEBUG */
char PROGRAM[80];
/* #endif */

main(argc, argv)
    int argc;
    char **argv;
    {
    int fd;
    struct sockaddr_in sin;
    struct ip_mreq mreq;
    char buffer[sizeof(struct message)];
    int s, i, j, result, done;
    struct hostent *hp;
    struct  message *ptestdata;
    int                     numMsgRecv = 1;
    char        IPAddr[20];
    char        inf[5];
    int port;
    int c;
    long int address;

    if(argc == 1)
        {
        printf(" USAGE: testRx  port  interface IP Address.\n");
        exit(1);
        }
    else 
    {
    	port =  atoi(&argv[1][0]);
        printf("port        = %d\n",port);
	strcpy(inf, &argv[2][0]);
	printf("Interface  = %s\n", inf);

	if(argc == 3)
        	printf("IP              = UNICAST\n");
    	else
    	{
    		strcpy(IPAddr,&argv[3][0]);
        	printf("IP              = %s\n",IPAddr);
    	}
    }

        printf("IS IT CORRECT?(Y/N)\n");
        c = getchar();
        if( c != 'Y' && c != 'y' )
                return;

    fd = socket (AF_INET, SOCK_DGRAM, 0);

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl (INADDR_ANY);
    sin.sin_port = htons ((u_short) (port));

    /* attach the label to the socket.              */
    result = bind (fd, (struct sockaddr *)&sin, sizeof(sin));
    if( result == -1)
	{
	perror("BIND ERROR");
	exit(1);
	}
    /*                 HERE IS THE MAGIC!!                   */
    /* Join the multicast group on all interfaces, this sends*/
    /* an IGMP Group membership Report out onto every        */
    /* attached network (for any router's which might be on  */
    /* those networks) and it tells the workstation's local  */
    /* IP and hardware to forward any multicast packets with */
    /* this address (on any network) up to us.               */
    sin.sin_addr.s_addr = inet_addr(IPAddr);
    address = ntohl(sin.sin_addr.s_addr);
    printf ("address 0x%x\n", address); 
    if ((((long int)(address)) & 0xf0000000) == 0xe0000000 )
		    printf (" Multicast  is OK %d\n",sin.sin_addr.s_addr);
    else 
		    printf (" Multicast  is NOT OK %d\n", sin.sin_addr.s_addr);

    printf ("IN_CLASSD %d\n", IN_CLASSD(sin.sin_addr.s_addr) );
    if(IN_CLASSD(address))
    {
	printf("IT IS CLASS D. Join Group Addr.\n");
	/*
    	result = net_join_multicast_group (fd, sin);
	*/
    	result = multicast_join (fd, sin, inf);
	if (result < 0) {
		printf ("multicast_join failed!\n");
		exit(0);
	}	
	printf ("multicast_joint !\n");
    }
    else 
	    printf ("Not Multicast!!!\n");

    ptestdata = (struct  message *)buffer;
	fprintf (stdout, "\n%s: Waiting for message:\n",
	    PROGRAM);
    while (1)
	{
	/* receive a single UPD MULTICAST packet from the    */
	/* address we setup above.                           */

	result = recvfrom (fd, buffer, sizeof(buffer), 0,
	    (struct sockaddr *)&sin, &s);
	if(result <0)
		{
		perror("recvfrom failed");
		}
	else
		{
		if(ptestdata->msgNumber == 1) /* Tx end restart */
		{
			numMsgRecv = 1;
			printf("THE TRANSMITTING END RESTART.\n");
		}
		printf("RECEIVING: MsgSize = %d Bytes. numMsgRecv = %d. PRI = %d. MsgNumber = %d\n",result, numMsgRecv, port, ntohl(ptestdata->msgNumber));
                numMsgRecv++;
		}

	}  /* while (!done) */

    /* Close the file before exiting.  This is not */
    /* really necessary, but is good practice.     */
    close (fd);
    }  /* main() */

/***********************************************************/
/*                   SUN Microsystems Code                 */
/*                        Starts here                      */
/***********************************************************/

/* Define the ifreq buffer size for SIOCGIFCONF requests */
/* XXX - changed from 256 to 32 for SVR4 alpha-4 bug */
#define NET_BROADCAST_IFCONF	(32)

/*
 * Find all the network interfaces capable of multicast and join
 * the multicast address on all of them.
 */

int
net_join_multicast_group(
	int			fd,
	struct sockaddr_in*	addr)
{
/*
	struct ifconf		ifc;
	struct ifreq		ifbuf[NET_BROADCAST_IFCONF];
	struct ifreq*		ifr;
	int			i;
	int			count;
	struct sockaddr_in*	sin;
*/
	/* Get a list of all network interfaces */
/*
	ifc.ifc_len = sizeof (ifbuf);
	ifc.ifc_buf = (caddr_t)ifbuf;
	if (ioctl(fd, SIOCGIFCONF, (char*)&ifc) < 0) {
		return (-1);
	}
*/
	/* Loop through the interfaces, looking for appropriate ones */
/*
	errno = 0;
	count = 0;
	ifr = ifc.ifc_req;
	for (i = ifc.ifc_len / sizeof (struct ifreq); i > 0; i--, ifr++) {
		struct ip_mreq	mreq;

		if (ifr->ifr_addr.sa_family != AF_INET)
			continue;

		if (ioctl(fd, SIOCGIFFLAGS, (char*)ifr) < 0) {
			continue;
		}
*/
		/* Skip uninteresting interfaces */
/*
		if (((ifr->ifr_flags & IFF_UP) == 0) ||
		    (ifr->ifr_flags & IFF_LOOPBACK) ||
		    ((ifr->ifr_flags & IFF_MULTICAST) == 0)) {
			continue;
		}
		if (ioctl(fd, SIOCGIFADDR, (char*)ifr) < 0)
			continue;

		sin = (struct sockaddr_in*)addr;
		mreq.imr_multiaddr = sin->sin_addr;
		mreq.imr_interface.s_addr =
		    ((struct sockaddr_in*)&ifr->ifr_addr)->sin_addr.s_addr;
		if (join_group(fd, &mreq) < 0) {
			return (-1);
		}
		count++;
	}
	if (count == 0) {
*/
		/* Could not find a suitable interface */
/*
		if (errno == 0)
			errno = ENOTCONN;
		return (-1);
	}
*/
	return (0);
}

/*
 * Keep reference counts on the multicast addresses
 * since the kernel only allows us to join the same group once
 * per socket.
 */
struct multiaddr {
	struct multiaddr*	next;
	struct in_addr		group;
	struct in_addr		interface;
	int			socket;
	int			count;
};
static struct multiaddr*	multiaddr;


/* Maintain reference counts to join/leave multicast groups */
/*
static int
join_group(
	int 			fd,
	struct ip_mreq*		mreq)
{
	struct multiaddr*	p;
	char string[80];

	p = multiaddr;
	while (p != NULL) {
		if ((p->socket == fd) &&
		    (p->group.s_addr == mreq->imr_multiaddr.s_addr) &&
		    (p->interface.s_addr == mreq->imr_interface.s_addr)) {
			p->count++;
	fprintf(stdout,
 	    "join_group:  %lx / %lx / %d   references: %d\n",
 	    p->group.s_addr, p->interface.s_addr,
 	    p->socket, p->count);

	fprintf 
	    (stdout, "\n\n*********************************************\n");
	fprintf (stdout, "%s:  Joining a multicast group\n", PROGRAM);
	fprintf (stdout, "*********************************************\n");
	fprintf (stdout, "join_group(): group      %s\n", 
	    convert_address (string, p->group.s_addr));
	fprintf (stdout, "join_group(): interface  %s\n", 
	    convert_address (string, p->interface.s_addr));
	fprintf (stdout, "*********************************************\n");
			return (0);
		}
		p = p->next;
	}
	if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
	    (char*)mreq, sizeof (*mreq)) < 0)
		return (-1);
	p = (struct multiaddr *)malloc(sizeof (struct multiaddr));
	if (p == NULL) {
		return (-1);
	}
	p->socket = fd;
	p->group = mreq->imr_multiaddr;
	p->interface = mreq->imr_interface;
	p->count = 1;
	p->next = multiaddr;
	multiaddr = p;
 	fprintf(stdout, "join_group:  %lx / %lx / %d   references: %d\n",
     		p->group.s_addr, p->interface.s_addr, p->socket, p->count);

	fprintf 
	    (stdout, "\n\n*********************************************\n");
	fprintf (stdout, "%s:  Joining a multicast group\n", PROGRAM);
	fprintf (stdout, "*********************************************\n");
	fprintf (stdout, "join_group(): group      %s\n", 
	    convert_address (string, p->group.s_addr));
	fprintf (stdout, "join_group(): interface  %s\n", 
	    convert_address (string, p->interface.s_addr));
	fprintf (stdout, "*********************************************\n");
	return (0);
}
*/
/* #ifdef DEBUG */
char *
convert_address (string, hex_address)
    char *string;
    unsigned long hex_address;
    {
    unsigned long temporary_1;
    unsigned long temporary_2;
    unsigned long temporary_3;
    unsigned long temporary_4;

    temporary_1 = (hex_address & 0xFF000000) >> 24;
    temporary_2 = (hex_address & 0x00FF0000) >> 16;
    temporary_3 = (hex_address & 0x0000FF00) >> 8;
    temporary_4 = hex_address & 0x000000FF;

    sprintf (string, "%lu.%lu.%lu.%lu",
	temporary_1,
	temporary_2,
	temporary_3,
	temporary_4);

    return (string);
    }  /* convert_address */
/* #endif */
int
multicast_join (int fd, const struct sockaddr_in sa_in, const char *ifname) {
   struct ip_mreqn mreq;
   unsigned char loop = 0;

   memcpy(&mreq.imr_multiaddr, &sa_in.sin_addr, sizeof(struct in_addr) );
   mreq.imr_ifindex = if_nametoindex(ifname);
   if (setsockopt (fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)) < 0) {
	   printf ("IP_MULTICAST_LOOP failed \n");
	   exit(0);
   }
   return (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq) ));
}
