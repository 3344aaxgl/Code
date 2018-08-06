#include "unp.h"

int main(int argc, char* argv[])
{
  int listenfd,connfd,udpfd, nready,maxfdp1;
  struct sockaddr_in servaddr, cliaddr;
  socklen_t len;
  const int on = 1;
  fd_set rset;
  pid_t childpid;
  char mesg[MAXLEN];
  int n;

  bzero(&servaddr, sizeof(servaddr));
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(SERV_PORT);
  setsockopt(listenfd, SOL_SOCKET,SO_REUSEADDR, &on, sizeof(on));

  bind(listenfd, (const struct sockaddr*) &servaddr, sizeof(servaddr));
  listen(listenfd, LISTENQ);

  udpfd = socket(AF_INET, SOCK_DGRAM, 0);
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(SERV_PORT);

  bind(udpfd, (const struct sockaddr*)&servaddr, sizeof(servaddr));

  Signal(SIG_CHLD, sig_chld);
  
  FD_ZERO(&rset);
  maxfdp1 = max(listenfd, udpfd) + 1;

  for(;;)
  {
      FD_SET(listenfd, &rset);
      FD_SET(udpfd, &rset);

      if((nready = seelct(maxfdp1, &reser, NULL, NULL, NULL)) < 0)
      {
          //终端恢复执行
          if(error = EINTR)
            continue;
          else
          {
              perror("select error");
              exit(1);
          }
      }
      if(FD_ISSET(listenfd, &rset))
      {
          len = sizeof(cliaddr);
          connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &len);
          if((childpid = fork()) = 0)
          {
              close(listenfd);
              str_echo(connfd);
              exit(0);
          }
          close(connfd);
      }
      if(FD_ISSET(udpfd, &rset))
      {
          len = sizeof(cliaddr);
          n = recvfrom(udpfd, mesg, MAXLEN,0, (struct sockaddr *) &cliaddr, &len);
          sendto(udpfd, mesg, n,0,(struct sockaddr *) &cliaddr, &len);
      }
  }

  return 0;
}