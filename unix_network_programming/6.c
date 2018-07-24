#include "unp.h"

void str_cli(FILE *fp, int sockfd)
{
    fd_set readset;
    int count;
    char sendline[MAXLEN];
    char recvline[MAXLEN];

    for (;;)
    {
        //每次调用select前设置关心的套接字描述符
        FD_ZERO(&readset);
        FD_SET(sock_fd, &readset);
        FD_SET(fileno(fp), &readset);
        //取得关心最大套接字 + 1
        int maxfd = (fileno(fp) > sock_fd ? fileno(fp) : sockfd) + 1;
        count = select(maxfd, &readset, NULL, NULL, NULL);
        //是否被设置
        if (FD_ISSET(fileno(fp), &readset))
        {
            if (fgets(sendline, MAXLEN, fp) == NULL)
              return;
            write(sockfd, sendline, strlen(sendline));
        }
        if (FD_ISSET(sockfd, &readset))
        {
            if(readline(sockfd, recvline, MAXLEN) == 0)
            {
                perror("recive FIN");
                exit(0);
            }
            fputs(recvline, stdout);
        }
    }
}