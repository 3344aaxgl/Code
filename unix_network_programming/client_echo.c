#include "unp.h"

void str_cli(FILE* fp, int sock_fd)
{
    char sendline[256], recvline[256];
    while(fgets(sendline, MAXLEN, fp) !=NULL)
    {
        writen(sock_fd, sendline, strlen(sendline));
        if(readline(sock_fd, recvline, MAXLEN) == 0)
        {
            perror("str_cli:server terminated prematurely");
        }
        fputs(recvline, stdout);
    }
}

int main(int argc, char* argv[])
{
    int sock_fd;
    struct sockaddr_in sin;
    inet_pton(AF_INET, argv[1], &sin.sin_addr);
    sin.sin_port = htons(atoi(argv[2]));
    sin.sin_family = AF_INET;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    connect(sock_fd, (const struct sockaddr*) &sin, sizeof(sin));

    str_cli(stdin, sock_fd);
    exit(0);

    return 0;
}