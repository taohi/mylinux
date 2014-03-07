#include <stdio.h>
#include <pcap.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>

//这些宏参见RFC1700
#define IPv4    0x0800
#define IPv6    0x86DD
#define ARP     0x0806
#define RARP    0x8035

#define ICMP    0x01
#define IGMP    0x02
#define GGP     0x03
#define TCP     0x06 
#define UDP     0x11 

//链路层数据包格式
typedef struct {
    u_char DestMac[6];
    u_char SrcMac[6];
    u_short Etype;
}ETHHEADER;

//IP层数据包格式
typedef struct {
    int header_len:4;
    int version:4;
    u_char tos:8;
    int total_len:16;
    int ident:16;
    int flags:16;
    u_char ttl:8;
    u_char proto:8;
    int checksum:16;
    u_char sourceIP[4];
    u_char destIP[4];
}IPHEADER;

//回调函数
void pcap_handle(u_char* user,const struct pcap_pkthdr* header,const u_char* pkt_data)
{
    ETHHEADER *eth_header=(ETHHEADER*)pkt_data;
    int *counter = (int*)user;
    u_short Ether_Proto;
    IPHEADER *ip_header=NULL;

    printf("---------------Begin Analysis-----------------\n");
    printf("Packet number: %d\t",++(*counter));
    printf("Packet length: %d\n",header->len);
    Ether_Proto = ntohs(eth_header->Etype);
    
    printf("Source MAC : %02X-%02X-%02X-%02X-%02X-%02X\t",eth_header->SrcMac[0],eth_header->SrcMac[1],eth_header->SrcMac[2],eth_header->SrcMac[3],eth_header->SrcMac[4],eth_header->SrcMac[5]);
    printf("Dest MAC : %02X-%02X-%02X-%02X-%02X-%02X\n",eth_header->DestMac[0],eth_header->DestMac[1],eth_header->DestMac[2],eth_header->DestMac[3],eth_header->DestMac[4],eth_header->DestMac[5]);

    switch(Ether_Proto)
    {
        case IPv4:
            ip_header=(IPHEADER*)(pkt_data+14);
            printf("Source IP:%d.%d.%d.%d\t",ip_header->sourceIP[0],ip_header->sourceIP[1],ip_header->sourceIP[2],ip_header->sourceIP[3]);
            printf("Dest IP:%d.%d.%d.%d\n",ip_header->destIP[0],ip_header->destIP[1],ip_header->destIP[2],ip_header->destIP[3]);
            switch(ip_header->proto)
            {
                case ICMP:
                    printf("Protocol:Ethernet/IP/ICMP\n");
                    break;
                case IGMP:
                    printf("Protocol:Ethernet/IP/IGMP\n");
                    break;
                case TCP:
                    printf("Protocol:Ethernet/IP/TCP\n");
                    break;
                case UDP:
                    printf("Protocol:Ethernet/IP/UDP\n");
                    break;
                default:
                    printf("Protocol:Ethernet/IP/Others\n");
            }
            break;
        case ARP:
            printf("Protocol:Ethernet/ARP\n");
            break;
        case IPv6:
            printf("Protocol:Ethernet/IPv6\n");
            break;
        case RARP:
            printf("Protocol:Ethernet/RARP\n");
            break;
        default:
            printf("Other Ethernet Protocol.\n");
    }

    //显示数据帧内容
    int i;  
    printf("Raw Packet Content:\n");
    for(i=0; i<(int)header->len; ++i)  {  
        printf(" %02x", pkt_data[i]);  
        if( (i + 1) % 16 == 0 )   
            printf("\n");  
    }  
    printf("\n\n");
}

int main(int argc, char **argv)
{
    char *device="eth0";
    char errbuf[1024];
    pcap_t *phandle;
    int counter = 0;
    int is_promisc=0;

    bpf_u_int32 ipaddress,ipmask;
    //    struct bpf_program fcode;
    //    int datalink;

    if((device=pcap_lookupdev(errbuf))==NULL){
        perror(errbuf);
        return 1;
    }
    else
        printf("device: %s\n",device);

    phandle=pcap_open_live(device,200,is_promisc,500,errbuf);
    if(phandle==NULL){
        perror(errbuf);
        return 1;
    }

    if(pcap_lookupnet(device,&ipaddress,&ipmask,errbuf)==-1){
        perror(errbuf);
        return 1;
    }
    else{
        char ip[INET_ADDRSTRLEN],mask[INET_ADDRSTRLEN];
        if(inet_ntop(AF_INET,&ipaddress,ip,sizeof(ip))==NULL)
            perror("inet_ntop error");
        else if(inet_ntop(AF_INET,&ipmask,mask,sizeof(mask))==NULL)
            perror("inet_ntop error");
        printf("IP address: %s, Network Mask: %s\n",ip,mask);
    }

    //注释掉过滤器
    /*    
          int flag=1;
          while(flag){
    //input the design filter
    printf("Input packet Filter: ");
    char filterString[1024];
    scanf("%s",filterString);

    if(pcap_compile(phandle,&fcode,filterString,0,ipmask)==-1)
    fprintf(stderr,"pcap_compile: %s,please input again....\n",pcap_geterr(phandle));
    else
    flag=0;
    }

    if(pcap_setfilter(phandle,&fcode)==-1){
    fprintf(stderr,"pcap_setfilter: %s\n",pcap_geterr(phandle));
    return 1;
    }

    if((datalink=pcap_datalink(phandle))==-1){
    fprintf(stderr,"pcap_datalink: %s\n",pcap_geterr(phandle));
    return 1;
    }

    printf("datalink= %d\n",datalink);
    */
    pcap_loop(phandle,-1,pcap_handle,(u_char *)&counter);

    return 0;
}
