#include "unp.h"

void dg_echo(int fd, const struct *cliaddr, socklen_t clilen)
{
    int n;
    socklen_t len;
    char mesg[MAXLEN];

    for(;;)
    {
        len = clilen;
        n = recvfrom(sockfd, mesg, MAXLEN, 0, cliaddr, &len);

        sendto(sockfd, mesg, MAXLEN, 0, cliaddr, len);
    }
}

int main(int argc, char* argv[])
{
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERV_PORT);

    bind(sockfd, (const struct sockaddr*) &servaddr, sizeof(servaddr));

    dg_echo(sockfd, (struct sockaddr*) &cliaddr, sizeof(cliaddr));

    return 0;
}
