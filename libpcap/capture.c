#include <stdio.h>
#include <pcap.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>

#define UDP 17
#define TCP 6
//链路层数据包格式
typedef struct {
    u_char DestMac[6];
    u_char SrcMac[6];
    u_char Etype[2];
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
//协议映射表
char *Proto[]={
    "Reserved","ICMP","IGMP","GGP","IP","ST","TCP"
};
//回调函数
void pcap_handle(u_char* user,const struct pcap_pkthdr* header,const u_char* pkt_data)
{
    ETHHEADER *eth_header=(ETHHEADER*)pkt_data;
    printf("---------------Begin Analysis-----------------\n");
    printf("----------------------------------------------\n");
    printf("Packet length: %d \n",header->len);
    //解析数据包IP头部
    if(header->len>=14){
        IPHEADER *ip_header=(IPHEADER*)(pkt_data+14);
        //解析协议类型
        char strType[100];
        if(ip_header->proto<=6)
            strcpy(strType,Proto[ip_header->proto]);
        else if(ip_header->proto ==17)
        {
            strcpy(strType,"UDP");
        }

        printf("Source MAC : %02X-%02X-%02X-%02X-%02X-%02X==>",eth_header->SrcMac[0],eth_header->SrcMac[1],eth_header->SrcMac[2],eth_header->SrcMac[3],eth_header->SrcMac[4],eth_header->SrcMac[5]);
        printf("Dest MAC : %02X-%02X-%02X-%02X-%02X-%02X\n",eth_header->DestMac[0],eth_header->DestMac[1],eth_header->DestMac[2],eth_header->DestMac[3],eth_header->DestMac[4],eth_header->DestMac[5]);
        
        printf("Source IP : %d.%d.%d.%d==>",ip_header->sourceIP[0],ip_header->sourceIP[1],ip_header->sourceIP[2],ip_header->sourceIP[3]);
        printf("Dest IP : %d.%d.%d.%d\n",ip_header->destIP[0],ip_header->destIP[1],ip_header->destIP[2],ip_header->destIP[3]);
        
        printf("Protocol : %s\n",strType);
        
        //显示数据帧内容
        int i;  
        for(i=0; i<(int)header->len; ++i)  {  
            printf(" %02x", pkt_data[i]);  
            if( (i + 1) % 16 == 0 )   
                printf("\n");  
        }  
        printf("\n\n");
    }
}

int main(int argc, char **argv)
{
    char *device="eth0";
    char errbuf[1024];
    pcap_t *phandle;
    
    bpf_u_int32 ipaddress,ipmask;
//    struct bpf_program fcode;
//    int datalink;
    
    if((device=pcap_lookupdev(errbuf))==NULL){
        perror(errbuf);
        return 1;
    }
    else
        printf("device: %s\n",device);
    
    phandle=pcap_open_live(device,200,0,500,errbuf);
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
    pcap_loop(phandle,-1,pcap_handle,NULL);

    return 0;
}
