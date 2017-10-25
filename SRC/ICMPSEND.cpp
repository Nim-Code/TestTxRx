#define	MAXWAIT		10	/* max time to wait for response */
#define	MAXPACKET	4096
#ifndef	MAXHOSTNAMELEN	
#define	MAXHOSTNAMELEN	64
#endif

#include <sys/time.h>
#include <sys/utsname.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int	verbose;
u_char	packet[MAXPACKET];
extern	int errno;

int	s;	/* socket file descriptor */
struct hostent	*hp;
struct timezone tz;

struct sockaddr whereto;
int datalen;

char usage[] = "Usage: sendicmp type host \n";

char 	*hostname;
char	hnamebuf[MAXHOSTNAMELEN];
char	*inet_ntoa();

int	ident;

int	timing = 0;
int	tmin = 99999999;
int	tmax = 0;
int	tsum = 0;
int	send_icmp(unsigned icmptype,unsigned icmpcode, struct hostent *hp,unsigned int sourceport,unsigned int destport);
void tvsub(register struct timeval *out,register struct timeval *in);
unsigned in_cksum(unsigned *addr,int len);

int main(int argc, char *argv[])
{
	struct	sockaddr_in from;
	char	*toaddr = NULL;
	struct sockaddr_in *to = (struct sockaddr_in *)&whereto;
	struct protoent *proto;
	unsigned int	sourceport, destport;

	int	len = sizeof(packet);
	int	fromlen = sizeof(from);
	int	cc;
	
	if (argc != 4) {
		printf("Usage : %s dest-host srcport destport\n", argv[0]);
		exit(1);
	}

	sourceport = atoi(argv[2]);
	destport = atoi(argv[3]);

	printf("Sending");
        printf(" ICMP SOURCE QUENCH");   /*  HARDCODE */
        printf(" message to %s\n", argv[1]);
	printf(" source port : %d, destination port : %d\n",
		sourceport, destport);
	printf(" source/dest : defined on %s\n", argv[1]);

	bzero((char *)&whereto, sizeof(struct sockaddr));
	to->sin_family = AF_INET;
	to->sin_addr.s_addr = inet_addr(argv[1]);
	if (to->sin_addr.s_addr != -1) {
		strcpy(hnamebuf, argv[1]);
		hostname = hnamebuf;
	} else {
		hp = gethostbyname(argv[1]);
		if (hp) {
			to->sin_family = AF_INET;
			bcopy(hp->h_addr, (caddr_t)&to->sin_addr.s_addr, hp->h_length);
			hostname = hp->h_name;
			/*
			toaddr = inet_ntoa(to->sin_addr.s_addr);
			*/
			printf("hp is Good.\n");
		} else {
			printf("%s: unknown host %s\n", argv[0], argv[1]);
			exit(1);
		}
	}

	datalen = 64 - 8;
	if (datalen > MAXPACKET) {
		fprintf(stderr, "ping: packet size too large\n");
		exit(1);
	}
	if (datalen >= sizeof(struct timeval))
		timing = 1;

	ident = getpid() & 0xFFFF;

	if ((proto = getprotobyname("icmp")) == NULL) {
		fprintf(stderr, "icmp: unknown protocol\n");
		exit(10);
	}
	if ((s = socket(AF_INET, SOCK_RAW, proto->p_proto)) < 0) {
		perror("icmp: socket");
		exit(5);
	}

        /* HARDCODE */
	printf("ICMP_SOURCEQUENCH is %d\n",ICMP_SOURCEQUENCH);
	send_icmp(ICMP_SOURCEQUENCH , 0, hp, sourceport, destport);
	/*
	send_icmp(0 , 0, hp, sourceport, destport);
	send_icmp(3 , 4, hp, sourceport, destport);
	send_icmp(4 , 0, hp, sourceport, destport);
	send_icmp(5 , 1, hp, sourceport, destport);
	send_icmp(8 , 0, hp, sourceport, destport);
	send_icmp(9 , 0, hp, sourceport, destport);
	send_icmp(10 , 0, hp, sourceport, destport);
	send_icmp(11 , 0, hp, sourceport, destport);
	send_icmp(12 , 0, hp, sourceport, destport);
	send_icmp(13 , 0, hp, sourceport, destport);
	send_icmp(14 , 0, hp, sourceport, destport);
	send_icmp(17 , 0, hp, sourceport, destport);
	send_icmp(18 , 0, hp, sourceport, destport);
	*/

/*
** I just want the program to send the packet, not receive it.

	if ((cc = recvfrom(s, packet, len, 0, &from, &fromlen)) < 0) {
		perror("ping: recvfrom");
		exit(0);
	}
	pr_pack(packet, cc, &from);

**
*/
	exit(0);
}




int send_icmp(unsigned icmptype,unsigned icmpcode, struct hostent *hp,unsigned int sourceport,unsigned int destport)
{
	static u_char outpack[8 + 20 + 8];
	register struct icmp *icp = (struct icmp *)outpack;
	int	i, cc;
	register u_char *datap = &outpack[8];

	struct	utsname	name;
	struct	hostent	*localhost;

	icp->icmp_type = icmptype;
	icp->icmp_code = icmpcode; 
	icp->icmp_cksum = 0;
	icp->icmp_seq = 0;
	icp->icmp_id = ident;

	cc = 8 + 20 + 8;

	icp->icmp_cksum = in_cksum((unsigned *)icp, cc);

	i = sendto(s, outpack, cc, 0, &whereto, sizeof(struct sockaddr));

	if (i < 0 || i != cc) {
		if (i < 0) perror("sendto");
		printf("ping: wrote %s %d chars, ret=%d\n",
			hostname, cc, i);
		fflush(stdout);
	}
}

char *pr_type(register int t)
{
	static char *ttab[] = {
		"Echo Reply",
		"ICMP 1",
		"ICMP 2",
		"Dest Unreachable",
		"Source Quenche",
		"Redirect",
		"ICMP 6",
		"ICMP 7",
		"Echo",
		"ICMP 9",
		"ICMP 10",
		"Time Exceeded",
		"Parameter Problem",
		"Timestamp",
		"Timestamp Reply",
		"Info Request",
		"Info Reply",
	};

	if (t < 0 || t > 16)
		return("OUT-OF-RANGE");
	return(ttab[t]);
}

void pr_pack(char *buf,int cc,struct	sockaddr_in *from)
{
	

	struct ip *ip;
	register struct icmp *icp;
	register long *lp = (long *)packet;
	register int i;
	struct timeval tv;
	struct timeval *tp;
	int hlen, triptime;

	from->sin_addr.s_addr = ntohl(from->sin_addr.s_addr);
	gettimeofday(&tv, &tz);

	ip = (struct ip *)buf;
	hlen = ip->ip_hl << 2;
	if (cc < hlen + ICMP_MINLEN) {
		if (verbose)
		{
			int let= 20;
			char adr[let];
			inet_ntop(AF_INET,&(from->sin_addr.s_addr) ,adr,let);
		printf("packet too short (%d bytes) from %s\n",cc,adr);
		}
		return;
	}
	cc -= hlen;
	icp = (struct icmp *)(buf + hlen);
	if (icp->icmp_type != ICMP_ECHOREPLY) {
		if (verbose) {
		int let= 20;
		char adr[let];
		inet_ntop(AF_INET,&(from->sin_addr.s_addr) ,adr,let);
		printf("%d bytes from %s: ", cc, adr);
			printf("icmp_type=%d (%s)\n",
				icp->icmp_type, pr_type(icp->icmp_type));
			for (i=0; i<12; i++) 
				printf("x%2.2x: x%8.8x\n",
					i*sizeof(long), *lp++);
			printf("icmp_code=%d\n", icp->icmp_code);
		}
		return;
	}
	if (icp->icmp_id != ident)
		return;

	tp = (struct timeval *)&icp->icmp_data[0];
	int let= 20;
	char adr[let];
	inet_ntop(AF_INET,&(from->sin_addr.s_addr) ,adr,let);
	printf("%d bytes from %s: ", cc, adr);
	printf("icmp_seq=%d.  ", icp->icmp_seq);
	if (timing) {
		tvsub(&tv, tp);
		triptime = tv.tv_sec*1000+(tv.tv_usec/1000);
		printf("time=%d. ms\n", triptime);
		tsum += triptime;
		if (triptime < tmin)
			tmin = triptime;
		if (triptime > tmax)
			tmax = triptime;
	} else
		putchar('\n');
}

/* Checksum routine for Internet Protocol family headers. */
unsigned in_cksum(unsigned *addr,int len)
{
	register int nleft = len;
	register unsigned *w = addr;
	register unsigned answer;
	register int sum = 0;
	u_short odd_byte = 0;

	/* using 32bit accumulator (sum), we add sequential 16 bit
	   words to it, and at the end, fold back all the carry bits from
	   the top 16 bits into the lower 16bits.
	*/

	while ( nleft > 1 ) {
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1) {
		*(u_char *)(&odd_byte) = *(u_char *)w;
		sum += odd_byte;
	}

	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	answer = ~sum;
	return(answer);
}

void tvsub(register struct timeval *out,register struct timeval *in)
{
	if ((out->tv_usec -= in->tv_usec) < 0) {
		out->tv_sec--;
		out->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
	return;
}





