#include "unp.h"
#include<limits.h>

void str_cli(FILE *fp, int sockfd)
{
    fd_set readset;
    int count;
    char sendline[MAXLEN];
    char recvline[MAXLEN];
    FD_ZERO(&readset);

    for (;;)
    {
        //每次调用select前设置关心的套接字描述符
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

void str_cli(FILE *fp, int sockfd)
{
    fd_set readset;
    int count, maxfd, stdineof;
    char sendline[MAXLEN];
    char recvline[MAXLEN];
    FD_ZERO(&readset);
    stdineof = 0;
    for (;;)
    {
        //每次调用select前设置关心的套接字描述符
        FD_SET(sock_fd, &readset);
        FD_SET(fileno(fp), &readset);
        //取得关心最大套接字 + 1
        maxfd = (fileno(fp) > sock_fd ? fileno(fp) : sockfd) + 1;
        count = select(maxfd, &readset, NULL, NULL, NULL);
        //是否被设置
        if (FD_ISSET(fileno(fp), &readset))
        {   //不在使用自有缓冲区的函数
            if (read(fielno(fp),sendline, MAXLEN) == 0)
            {
                //标准输入接收到EOF，关闭套接字
                stdineof = 1;
                shutdown(sockfd, SHUT_WR);
                FD_CLR(fileno(fp), &readset);
                continue;
            }
            write(sockfd, sendline, strlen(sendline));
        }
        if (FD_ISSET(sockfd, &readset))
        {
            if(read(sockfd, recvline, MAXLEN) == 0)
            {
                if(stdineof == 1)
                  return ;
                else
                {
                    perror("server terminate prematurely");
                    exit(-1);
                }
            }
            write(fileno(stdout),recvline, strlen(recvline));
        }
    }
}

int main()
{
    int listenfd,maxfd,clifd,maxi,i,n;
    struct sockaddr_in servaddr,cliaddr;
    socklen_t len;
    fd_set readset,allset;
    int count,client[FD_SETSIZE];
    char buf[MAXLEN];

    memset(servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bind(listenfd, (const struct sockaddr*) &servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);

    maxfd = listenfd;
    maxi = -1;
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    for(;;)
    {
        readset = allset
        count = select(maxfd + 1,&readset, NULL,NULL,NULL);
        if(FD_ISSET(listenfd, &readset))
        {
            len = sizeof(cliaddr);
            clifd = accept(listenfd,(struct sockaddr*) &cliaddr, &len);
            for(i = 0; i< FD_SETSIZE; i++)
            {
                //找到第一个未被使用的用来存储已连接套接字
                if(client[i] < 0)
                {
                  client[i] = clifd;
                  break;
                }
            }
            if (i == FD_SETSIZE)
            {
                perror("too many client");
                exit(-1);
            }
            FD_SET(clifd, &allset);
            if(clifd > maxfd)
              maxfd = clifd;
            if(maxi < i)
              maxi = i;
            //只有监听套接字有事件发生
            if(--count <= 0 )
                continue;
        }
        for(i = 0; i <= maxi; i++)
        {
            if(client[i] < 0)
              continue;
            if(FD_ISSET(sockfd,&readset))
            {
                //改成不使用自有缓存的函数
                if((n = readn(client[i], buf, MAXLEN)) == 0)
                {
                    close(client[i]);
                    FD_CLR(sockfd,&allset);
                    client[i] = -1;
                }
                else
                {
                    writen(client[i], buf, n);
                }

                if(--count <= 0)
                  break;
            }
        }
    }
    return 0;
}

int main()
{
    int i, maxi, listenfd, connfd, sockfd;
    int nready;
    ssize_t n;
    char buf[MAXLEN];
    socklen_t clilen;
    struct pollfd client[OPEN_MAX];
    struct sockaddr_in cliaddr, servaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    bind(listenfd, (const struct sockaddr*) & servaddr, sizeof(servaddr));
    listen(listen, LISTENQ);

    client[0].fd = listenfd;
    client[0].events = POLLRDNORM;
    for(i = 0;i < OPEN_MAXl; i++)
        client[i].fd = -1;
    maxi = 0;

    for(;;)
    {
        nready = poll(client, maxi + 1, INFTIME);

        if(client[0].revents & POLLRDNORM)
        {
            clilen = sizeof(cliaddr);
            connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &clilen);

            for(i = 0; i < OPEN_MAX; i++)
              if(client[i].fd < 0)
              {
                client[i].fd = connfd;
                break;
              }
            if(i == OPEN_MAX)
            {
                perror("too many clients");
                exit(-1);
            }
            client[i].events = POLLRDNORM;
            if(maxi < i)
              maxi = i;
            if(--nready <= 0)
              continue;
        }
        for(i = 1; i < maxi; i++)
        {
            if((sockfd = client[i].fd) < 0)
              continue;
            if(client[i].revents & (POLLRDNORM | POLLERR))
            {
                if((n = read(sockfd, buf, MAXLEN)) < 0)
                {
                    if(errno == ECONNREST)
                    {
                        close(sockfd);
                        client[i].fd = -1
                    }
                    else
                    {
                        perror(read error);
                        exit(-1);
                    }
                }
                else if(n == 0)
                {
                    close(sockfd);
                    client[i].fd = -1;
                }
                else
                  writen(sockfd, buf, n);
                if(--nready <= 0)
                  break;
            }
        }
    }
    return 0;
}