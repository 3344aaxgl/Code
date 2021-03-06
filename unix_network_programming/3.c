#include < stdio.h > 
#include < string.h > 


//点分十进制转二进制
int chartobinary(char * str) {
    char * p = str; 
    char * q = p; 
    int sum = 0; 
    unsigned int result = 0; 
    while ( * q != 0) {
        if ( * q == '.') {
            if (sum < 0 || sum > 255)
              return 0; 
            result = (result << 8) | sum; 
            sum = 0; 
        }
        else if ( * q >= '0' &&  * q <= '9') {
            //取得

            sum = (sum * 10) +  * q - '0'; 
        }
        q++; 
    }
    result = (result << 8) | sum; 
    return result; 
}

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

#define MAXLEN 256
static int  read_cnt;
static char *read_ptr;
static char rad_buf[MAXLEN];

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
    *ptr = *read_buf++;
    return 1;
}

ssize_t readline(int fd, void* ptr, size_t maxlen)
{
    sszie_t n, rc;
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

sszie_t readlinebuf(void **vptrptr)
{
    if(read_cnt)
      *vptrptr = read_ptr;
    return read_cnt;
}


int main() {
  char * ip = "192.168.77.51"; 

  int result = chartobinary(ip); 
  printf("%u\n", result); 
  return 0; 
}