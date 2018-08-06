#include "unp.h"

int main(int argc ,char* argv[])
{
    int sockfd;
    socklen_t len;
    struct sockaddr_in cliaddr, servaddr;

    if(argc != 2)
    {
        perror("useage:udpcli <IPaddress>");
        exit(-1);
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));

    len = sizeof(cliaddr);
    getsockname(sockfd, (sockaddr*) &cliaddr,&len);
    printf("local address %s\n", sock_ntop((struct sockaddr*) &sockaddr,len));
    return 0;
}
