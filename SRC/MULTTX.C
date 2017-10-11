/* To run this program, change the executeable file as:
   # chown root testTx
   # chmod 4455
   */

#include        "includes.h"

extern	int icmpRecv();
struct sockaddr_in      serv_addr;

struct hostent *gethostbyname();

main(argc, argv)
    int argc;
    char **argv;
    {
    int     msgNumber = 1;
    int     msgCount = 0;
    int	error;
    int fd,tfd;
    struct sockaddr_in addr;
    char msg[1024];
    int i, msglen, done, result;
    struct hostent *hp;
    struct message testdata;
    unsigned char ttl;  /* time-to-live in hops */
    u_char loop = 0;
    int icmpfd;
    int	c;
    char	IPAddr[20];

    int priority,msgSize,delay,variableRate;

        if(argc < 6)
        {
        printf(" USAGE: testTx  priority  msgSize(bytes)  interval(msec)  variableRate IPAddr.\n");
        exit(1);
        }
        priority =  atoi(&argv[1][0]);
        msgSize  =  atoi(&argv[2][0]);
        delay  =    atoi(&argv[3][0]);
	variableRate = atoi(&argv[4][0]);
	strcpy(IPAddr,&argv[5][0]);

        printf("priority	= %d\n",priority);
        printf("msgSize		= %d bytes\n",msgSize);
        printf("Initial delay	= %d msec\n",delay);
        printf("Variable rate	= %d 	1=YES 2=NO\n",variableRate);
	printf("IP 		= %s\n",IPAddr);
	printf("IS IT CORRECT?(Y/N)\n");
	c = getchar();
	if( c != 0x59 && c != 0x79 )
		return;
    if( (icmpfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
                        {
                        perror("icmp socket");
                        exit(1);
                        }
        fcntl(icmpfd,F_SETFL,O_NONBLOCK);

    fd = socket (AF_INET, SOCK_DGRAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(IPAddr);
/*
    addr.sin_port = (u_short) (UDP_MULTICAST_BASE_PORT + priority);
*/

    addr.sin_port = htons ((u_short) (UDP_MULTICAST_BASE_PORT + priority));

    if(IN_CLASSD(addr.sin_addr.s_addr))

        {
        printf("IT IS CLASS D. Set TTL.\n");
        ttl = (u_char) 10;
        if( setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof (ttl)) <0 )
                perror("setsockopt:IP_MULTICAST_TTL\n");
        if( setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&loop, sizeof (loop)) <0 )
                perror("setsockopt:IP_MULTICAST_LOOP\n");
        }


    /* Get multicast address from /etc/hosts file   */
    /* or NIS, or the Domain Name Server, depending */
    /* on how the workstation is configured.        */
    /*
    hp = gethostbyname ("Multicast_Tester");
    memcpy ((void *)&addr.sin_addr.s_addr, (void *)hp->h_addr,
	hp->h_length);
    */

    while (1)
	{
	/********* PUT YOUR APPLICATION HERE *********/
         testdata.msgNumber  = htonl (msgNumber);

	/* MULTICAST one UDP datagram to the address */
	/* we prepared above.                        */
tryAgain:
         if(icmpRecv(icmpfd) > 0)
                {
		 if(sendto (fd, (char *)&testdata, msgSize, 0, (struct sockaddr *)&addr, sizeof (addr)) < 0 )
		  {
		  printf("sendto ERROR.\n");
		  exit(1);
		  }
		  printf("SENDING: MsgSize = %d Bytes. INTERVAL = %d msec. PRI=%d. MsgNumber = %d\n",msgSize,delay,priority,msgNumber);
                        msgNumber++;
			msgCount++;
                 }
         else
                {
                 while(icmpRecv(icmpfd) < 0);
                 printf("CONGESTION EXPERIENCED ON SUBNET. MSG WILL BE SENT WHEN CONGESTION IS CLEAR.\n");
		 if(variableRate)
                 	delay = delay + 200;
		 msgCount = 0;
                 sleep(20);
                 goto tryAgain;
                }
	if(variableRate) 
	{
	   if(msgCount == 50)
		{
		if(delay > 500)
			delay = delay - 200;
		msgCount = 0;
		}
	}
        usleep (delay*1000);
	}  /* while (!done) */

    /* Close the file before exiting.  This is not */
    /* really necessary, but is good practice.     */
    close (fd);
    }  /* main() */

