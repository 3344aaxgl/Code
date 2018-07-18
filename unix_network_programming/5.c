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
signale(SIGCHLD, sig_chld);