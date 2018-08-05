#include "unp.h"

void dh_cli(FILE* fp, int fd,const struct sockaddr* pservaddr, socklen_t servlen)
{
    int n;
    struct sockaddr preply_addr; 
    char sendline[MAXLEN], recvline[MAXLEN + 1];
    socklen_t len;
    while(fgets(sendline, MAXLEN, fp) != NULL)
    {
        sendto(fd, sendline, strlen(sendline), 0, &servaddr, servlen);
        //n = recvfrom(fd, recvline, MAXLEN, 0, NULL, NULL);
        len = socklen;
        n = recvfrom(fd, recvline, MAXLEN, 0, &preply_addr, &len);
        if(len != servlen || memcmp(&preply_addr, pservaddr, len))
        {
            printf("reply from %s (ignored)\n", sock_ntop(&preply_addr, len));
            continue;
        }
        recvline[n] = 0;
        fputs(recvline, stdout);
    }
}

int main(int argc, char *argv[])
{
    struct servaddr;
    int sockfd;
    socklen_t socklen;

    if(argc != 2)
    {
        perror("useage:udpcli <IPaddress>");
        exit(-1);
    }

    sockfd = socket(AF_INET, SO_DGRAM, 0);
    bzero(&servaddr, sizeof(servaddr));

    //点分十进制转二进制
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);

    dg_cli(stdin,sockfd, &servaddr, sizeof(servaddr));

    return 0;
}