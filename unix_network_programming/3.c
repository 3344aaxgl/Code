#include<stdio.h>
#include<string.h>


//点分十进制转二进制
int chartobinary(char *str)
{
    char *p = str;
    char *q = p;
    int sum = 0;
    unsigned int result = 0;
    while(*q != 0)
    {
        if(*q == '.')
        {
            if(sum < 0 ||sum > 255 )
              return 0;
            result = (result << 8) | sum;
            sum = 0;
        }
        else if(*q >='0' && *q <= '9' )
        {
            //取得

            sum =(sum*10) + *q-'0';
        }
        q++;
    }
    result = (result << 8) | sum;
    return result;
}

//依据套接字地址结构自动将二进制地址转换成点分十进制地址
char* sock_ntop(const struct sockaddr* sa, socklen_t addrlen)
{
    char portstr[8];
    static char str[256];

    switch(sa->sa_family)
    {
        case AF_INET:
          struct sockaddr_in* sin = (struct sockaddr_in*)sa;
          inet_ntop(AF_INET, sin, str, sizeof(str));
          if(ntohs(sin->sin_port) != NULL)
          {
              snprintf(portstr,sizeof(portstr),":%d", ntohs(sin->sin_port));
              strcat(str,portstr);
          }
        case AF_INET6:
          struct sockaddr_in6 sin6 = (struct sockaddr_in6*) sa;
          inet_ntop(AF_INET6, sin6, str, sizeof(str));
          if(ntohs(sin6->sin6_port) != NULL)
          {
              snprintf(portstrstr, sizeof(portstr), ":%d",ntohs(sin6->sin6_port));
              strcat(str,portstr);
          }
    }
    return str;
}



int main()
{
  char *ip = "192.168.77.51";

  int result  = chartobinary(ip);
  printf("%u\n", result);
  return 0;
}