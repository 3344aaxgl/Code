#include "unp.h"

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
}