#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<signal.h>
#include<sys/wait.h>
#include<select.h>
#include<sys/time.h>

#define SERV_PORT 54321

#define LISTENQ 256

#define MAXLEN 256

char * sock_ntop(const struct sockaddr * sa, socklen_t addrlen) 
{
    char portstr[8]; 
    static char str[256]; 

    switch (sa -> sa_family) 
    {
        case AF_INET: 
        {
          struct sockaddr_in * sin = (struct sockaddr_in * )sa; 
          inet_ntop(AF_INET, sin, str, sizeof(str)); 
          if (ntohs(sin -> sin_port) != 0) 
          {
              snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin -> sin_port)); 
              strcat(str, portstr); 
          }
        }
        case AF_INET6: 
        {
          struct sockaddr_in6 * sin6 = (struct sockaddr_in6 * )sa; 
          inet_ntop(AF_INET6, sin6, str, sizeof(str)); 
          if (ntohs(sin6 -> sin6_port) != 0) 
          {
              snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin6 -> sin6_port)); 
              strcat(str, portstr); 
          }
        }
    }
    return str; 
}

ssize_t readn(int fd, void* vptr,size_t n)
{
    size_t nleft;
    ssize_t nread;
    char* ptr;
    ptr = vptr;
    nleft = n;

    while(nleft > n)
    {
        if((nread = read(fd, ptr, nleft)) <0)
        {
            if(errno = EINTR)
              nread = 0;
            else
              return -1;
        }
        else if(nread == 0)
          break;
        nleft -= nread;
        ptr += nread;
    }
    return n - nleft;
}

ssize_t writen(int fd, void* vptr, size_t n)
{
    char* ptr = vptr;
    int nwrite;
    int nleft = n;

    while(nleft > 0)
    {
        if((nwrite = write(fd, ptr, nleft)) < 0)
        {
            if(errno ==EINTR)
              nwrite = 0;
            else 
              return -1;
        }
        
        nleft -= nwrite;
        ptr += nwrite;
    }
    return n ;
}


static int  read_cnt;
static char *read_ptr;
static char read_buf[MAXLEN];

static ssize_t my_read(int fd, char* ptr)
{
    if(read_cnt <= 0)
    {
        again:
        if((read_cnt = read(fd, read_buf, MAXLEN)) < 0)
        {
            if(errno = EINTR)
              goto again;
            else
              return -1;
        }
        else if(read_cnt == 0)
          return 0;
        read_ptr = read_buf;
    }
    read_cnt --;
    *ptr = *read_ptr++;
    return 1;
}

ssize_t readline(int fd, void* vptr, size_t maxlen)
{
    ssize_t n, rc;
    char c, *ptr;

    ptr = vptr;
    for(n =1; n < maxlen; n++)
    {
        if((rc = my_read(fd, &c)) == 1)
        {
            *ptr++ = c;
            if(c =='\n')
              break;
        }
        else if(rc ==0)
        {
          *ptr = 0;
          return n-1;
        }
        else
          return -1;
    }
    *ptr = 0;
    return n;
}

ssize_t readlinebuf(void **vptrptr)
{
    if(read_cnt)
      *vptrptr = read_ptr;
    return read_cnt;
}

typedef void Sigfunc(int);

Sigfunc * Signal(int signo, Sigfunc* func)
{
    struct sigaction act,oact;
    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    if(signo == SIGALRM)
    {
        #ifdef SA_INTERRUPT
          act.sa_flags |= SA_INTERRUPT;
        #endif
    }
    else
    {
        #ifdef SA_RESTART
          act.flags |= SA_RESTART;
    }
    if(sigaction(signo, &act, &oact) < 0)
      return SIG_ERR;
    return oact.sa_handler;
}

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

void sig_chld(int signo)
{
    pid_t pid;
    int stat;

    while((pid = waitpid(-1, &stat, WNOHANG)) > 0);
    printf("child %d terminated\n", pid);
    return ;
}