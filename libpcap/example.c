#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <pcap.h>  
#include <netinet/if_ether.h>  
#include <arpa/inet.h>
#define IP 2048  
#define TCP 6  
 
typedef unsigned char UINT1;  
typedef unsigned short UINT2;  
typedef unsigned int UINT4;  
 
typedef struct ether  
{  
  UINT1 dest[6];  
  UINT1 src[6];  
  UINT2 proto;  
  UINT1 data[0];  
} tEther;  
 
typedef struct ip  
{  
  UINT1 hlen;   /*首部长度+版本 */ 
  UINT1 tos;   /*服务 */ 
  UINT2 len;   /*总长度 */ 
  UINT2 ipid;   /*标示 */ 
  UINT2 flagoff;  /*标示加偏移 */ 
  UINT1 ttl;   /*生存时间 */ 
  UINT1 proto;   /*协议 */ 
  UINT2 cksum;   /*首部检验和 */ 
  UINT4 src;   /*源地址 */ 
  UINT4 dest;   /*目地地址 */ 
  UINT1 data[0];  
} tIp;  
 
typedef struct tcp  
{  
  UINT2 sport;  
  UINT2 dport;  
  UINT4 seq;  
  UINT4 ack;  
  UINT1 hlen;  
  UINT1 code;  
  UINT2 window;  
  UINT2 chsum;  
  UINT2 urg;  
  char data[0];  
} tTcp;  
 
void proc_pkt (u_char * user, const struct pcap_pkthdr *hp, const u_char * packet);  
void print_addr (UINT4 ipaddr);  
 
int main ()  
{  
  char *dev = NULL;  
  pcap_t *descr;  
  struct pcap_pkthdr hdr;  
  u_char *packet;  
  char errbuf[PCAP_ERRBUF_SIZE];  
  int promisc = 0, cnt = 5;  
  int pcap_time_out = 100;  
  struct tEther *pEpkt;  
  UINT4 net, mask;  
 
  dev = pcap_lookupdev (errbuf);  
  pcap_lookupnet (dev, &net, &mask, errbuf);  
  descr = pcap_open_live (dev, BUFSIZ, promisc, pcap_time_out, errbuf);  
  printf ("网络号:");  
  print_addr (net);  
  printf ("网络掩码:");  
  print_addr (mask);  
  printf ("\n\n");  
  pcap_loop (descr, 10, proc_pkt, NULL);  
  printf ("%s\n", dev);  
 
  return 0;  
}  
 
void proc_pkt (u_char * user, const struct pcap_pkthdr *hp, const u_char * packet)  
{  
  tEther *pEther;  
  tIp *pIp;  
  int i;  
  pEther = (tEther *) packet;  

  //if (ntohs (pEther->proto) == ICMP) 
   //   printf("PING cmd found.\n");
  if (ntohs (pEther->proto) == IP) /*网络层的报头为IP*/ 
  {  
      pIp = (tIp *) pEther->data;  
      if (pIp->proto == TCP) /*传输层的报头为为tcp */ 
      {  
          tTcp *pTcp;  
          pTcp = (tTcp *) pIp->data;  
          printf ("目的MAC地址:");  
          for (i = 0; i < 6; i++)  
          {  
              if ((pEther->dest[i]) < 16)  
                  printf ("0");  
              printf ("%x ", pEther->dest[i]);  
          }  
          printf ("\n源MAC地址:");  
          for (i = 0; i < 6; i++)  
          {  
              if (((pEther->src[i]) < 16))  
                  printf ("0");  
              printf ("%x ", pEther->src[i]);  

          }  
          printf ("\n");  
          printf ("源IP地址:");  
          print_addr (pIp->src);  
          printf ("目的IP地址:");  
          print_addr (pIp->dest);  
          printf ("源端口:");  
          printf ("%hu\n", ntohs (pTcp->sport));  
          printf ("目的端口:");  
          printf ("%hu\n\n\n", ntohs (pTcp->dport));  
      }  
  }  
  return;  
}  

void print_addr (UINT4 ip_mask) 
{
    struct in_addr addr;
    char *net;
    addr.s_addr = ip_mask;
    net=inet_ntoa(addr);
    printf("%s\n",net);
}

