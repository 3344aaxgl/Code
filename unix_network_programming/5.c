#include "unp.h"

void sig_chld(int signo)
{
    pid_t pid;
    int stat;

    pid = wait(&stat);
    printf("child %d terminated\n", pid);
    return ;
}
//注册处理SIGCHLD信号的函数，必须在fork之前注册
Signal(SIGCHLD, sig_chld);

//在客户与服务器之间传递文本

void str_echo(int sockfd)
{
    long arg1, arg2;
    sszie_t n;
    char line[MAXLEN];

    for(;;)
    {
        if((n = readline(scokfd,line, MAXLEN)) == 0)
          return;
        if(sscanf(line, "%ld%ld", &arg1,&arg2) == 2)
          snprintf(line, sizeof(line), "%ld\n", arg1 + arg2);
        else
          snprintf(line, sizeof(line), "input error\n");
        n = strlen(line);
        writen(sockfd,line, n);
    }
}

struct args
{
    long arg1;
    long arg2;
};

struct result
{
    long sum;
};

void str_cli(FILE* fp, int sockfd)
{
    char sendline[MAXLEN];
    struct args args;
    struct result result;

    while(fgets(sendline, MAXLEN, stdin))
    {
        if(sscanf(sendline,"%ld%ld", &args.arg1, &args.arg2) != 2)
        {
            printf("invalid input: %s", sendline);
            continue;
        }
        writen(sockfd, &args, sizeof(args));
        if(readn(sockfd, &result, sizeof(result)) == 0)
        {
            perror("str_cli:server terminated prematurely");
            exit(-1);
        }
        printf("%ld\n", result.sum);
    }
}

void str_echo(int sockfd)
{
    sszie_t n;
    struct args args;
    struct result result;

    for(;;)
    {
        if((n = readn(sock_fd, &args, sizeof(args))) == 0)
          return;
        result.sum = args.arg1 + args.arg2;
        writen(sockfd, &result, sizoef(result));
    }
}

int main(int argc, char* argv[])
{
    int i,sock_fd[5];
    struct sockaddr_in sin;
    inet_pton(AF_INET, argv[1], &sin.sin_addr);
    sin.sin_port = htons(atoi(argv[2]));
    sin.sin_family = AF_INET;

    for(i = 0; i < 5; i++)
    {
        sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        connect(sock_fd, (const struct sockaddr*) &sin, sizeof(sin));
    }

    str_cli(stdin, sock_fd[0]);
    exit(0);
}