#include"unp.h"
#include<time.h>

int main(int argc,char* argv[])
{
    int listenfd,sockfd;
    struct sockaddr_in serv_sockaddr,clien_sockaddr;
    size_t socklen;
    char buf[MAXLEN];
    time_t tricks;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_sockaddr.sin_family = AF_INET;
    serv_sockaddr.sin_addr.s_addr = htonl(INANY_ADDR);
    serv_sockaddr.sin_port = htons(SERV_PORT);

    bind(listenfd, (const struct sockaddr*) &serv_sockaddr, sizeof(serv_sockaddr));
    listen(listenfd, LISTENQ);

    for(;;)
    {
        len = sizeof(clien_sockaddr);
        sockfd = accept(listenfd, (struct sockaddr*) &clien_sockaddr, &socklen);
        printf("connection from %s,port %d\n", inet_ntop(AF_INET, &clien_sockaddr.sin_addr, buf,sizeof(buf)),ntohs(clien_sockaddr.sin_port));
        ticks = time(NULL);
        snprintf(buf, sizeof(buf), "%.24s\r\n", ctime(&tricks));
        write(sockfd, buf, strlen(buf));
    }
    return 0;
}