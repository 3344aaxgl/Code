#include "unp.h"

#define SERV_PORT 54321
#define LISTENQ 256

int str_echo(int fd)
{
    char buf[MAXLEN];
    int num;
    while(1)
    {
        if((num = read(fd, buf, sizeof(buf))) <0)
        {
            if(errno = EINTR)
              ;           
            else
            {
                perror("read error");
                exit(-1);    
            };            
        }
        else if( num == 0)
          break;
        else
          writen(fd,buf,num);
    }
}

int main(int argc, char*argv[])
{
    struct sockaddr_in serv_sockaddr,client_sockaddr;
    socklen_t clilen;
    int serv_fd,conn_fd;
    pid_t childpid;

    serv_fd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&serv_sockaddr, 0, sizeof(serv_sockaddr));
    serv_sockaddr.sin_family = AF_INET;
    serv_sockaddr.sin_port = htons(SERV_PORT);
    //通用地址
    serv_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(serv_fd, (const struct sockaddr*) &serv_sockaddr, sizeof(serv_sockaddr));
    listen(serv_fd, LISTENQ);

    for(;;)
    {
        //阻塞与accept，等待连接到来
        conn_fd = accept(serv_fd, (struct sockaddr*) &client_sockaddr, &clilen);
        //fork子进程处理新连接
        if((childpid = fork()) == 0)
        {
            close(serv_fd);
            str_echo(conn_fd);
            exit(0);
        }
        close(conn_fd);
    }
    return 0;
}

