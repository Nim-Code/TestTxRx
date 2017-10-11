/*
gcc -o icmpRecv icmpRecv.c -R/usr/ucblib -L/usr/ucblib -lucb  -lnsl -lc
*/
#include	"includes.h"
int
icmpRecv(int icmpfd)
{
        int             Bytes_Read;
        struct icmp             *icmp_ptr;
        struct ip               *ip_ptr;
        int                     ip_hdr_len;
	char			udataptr[1500];

              if( (Bytes_Read = recv (icmpfd,udataptr,1500,0)) >0)
	 	{
		/*
                printf ("icmp_rec: rec'd data \n");
		*/

                ip_ptr = (struct ip *)udataptr;
                ip_hdr_len = ip_ptr->ip_hl << 2;
/*
                printf ("icmp_rec: ip header length = %d\n",ip_hdr_len);
*/
                icmp_ptr = (struct icmp *) (udataptr + ip_hdr_len);
 
                if (icmp_ptr->icmp_type == ICMP_SOURCEQUENCH)
                        {
                        /* printf(" icmp_rec: rec'd source quench\n"); */
			return(-1);
                        }
                else if (icmp_ptr->icmp_type == ICMP_ECHO)
			{
/*
                        printf(" icmp_rec: rec'd ping. Bytes_Read = %d.\n",Bytes_Read);
*/
			return(1);
			}
		  }
		else
			return(1);
}

