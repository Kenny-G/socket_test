#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <iostream>
#include <string.h>
#include <time.h>
#include<stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>

using namespace std;
using std::vector;

#define INVALID_SOCKET -1
#define SERVER_PORT 9888
#define MAX_BUFSIZE 65535
#define MAX_SERVER_EVENT 256

bool mylisten(int& epid, int& skid)
{
  // listen
  int m_nListenEpfd = epoll_create(MAX_SERVER_EVENT);
  if (m_nListenEpfd < 0)
  {
    std::cout << "create epoll error" << std::endl;
    return false;
  }
  std::cout << "create epoll success:" << m_nListenEpfd << std::endl;

  int sockid = 0;
  sockid = socket(AF_INET, SOCK_STREAM, 0);
  if (sockid == INVALID_SOCKET)
  {
    std::cout << "socket error" << std::endl;
    return false;
  }
  std::cout << "socket bulid success:" << sockid << std::endl;

  int re = 1;
  setsockopt(sockid, SOL_SOCKET, SO_REUSEADDR, &re, sizeof(re));

  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(SERVER_PORT);

  if (::bind(sockid, (sockaddr *) &addr, sizeof(addr)) == -1)
  {
    std::cout << "bind error" << std::endl;
    return false;
  }
  std::cout << "bind success" << std::endl;

  if(::listen(sockid, 256) == -1)
  {
    std::cout << "listen error" << std::endl;
    return false;
  }
  std::cout << "begin listen" << std::endl;

  epoll_event event;
  bzero(&event, sizeof(event));
  event.data.fd = sockid;
  event.events = EPOLLIN | EPOLLERR;
  epoll_ctl(m_nListenEpfd, EPOLL_CTL_ADD, sockid, &event);

  epid = m_nListenEpfd;
  skid = sockid;

  return true;
}

int main()
{
  int epid = 0;
  int sockid = 0;

  if (mylisten(epid, sockid) == false)
    return -1;

  int clientsock = 0;
  int clientepid = 0;
  sockaddr_in clientaddr;

  // wait connect
  while(true)
  {
    struct epoll_event events[MAX_SERVER_EVENT];

    bzero(events, MAX_SERVER_EVENT * sizeof(epoll_event));
    int nfds = epoll_wait(epid, events, MAX_SERVER_EVENT, 20);
    for (int i = 0; i < nfds; ++i)
    {
      //std::cout << "nfds:" << i << "  事件:" << (int)events[i].events << std::endl;
      if (events[i].data.fd == sockid)
      {
        sockaddr_in caddr;
        int addrlen = sizeof(caddr);
        bzero(&caddr, sizeof(caddr));
        int cfd = ::accept(sockid, (struct sockaddr*)&caddr, (socklen_t *)&addrlen);
        if(-1 == cfd)
        {
          sleep(1);
        }
        else
        {
          //accept(cfd, caddr);
          clientsock = cfd;
          clientaddr = caddr;
          int nBuf = MAX_BUFSIZE * 2;
          setsockopt(clientsock,SOL_SOCKET,SO_RCVBUF,(const char*)&nBuf,sizeof(int));
          setsockopt(clientsock,SOL_SOCKET,SO_SNDBUF,(const char*)&nBuf,sizeof(int));

          // set no block
          int flags = fcntl(clientsock, F_GETFL, 0);
          flags |= O_NONBLOCK;
          fcntl(clientsock, F_SETFL, flags);

          clientepid = epoll_create(MAX_SERVER_EVENT);
          epoll_event ev;
          bzero(&ev, sizeof(ev));
          ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
          //ev.data.ptr = this;
          epoll_ctl(clientepid, EPOLL_CTL_ADD, clientsock, &ev);

          std::cout << "connect success: " << clientsock << " " << inet_ntoa(caddr.sin_addr) <<
            " " << caddr.sin_port << std::endl;
          usleep(1000);
        }
      }
    }
    if (clientepid)
    {
      bzero(events, MAX_SERVER_EVENT * sizeof(epoll_event));
      nfds = epoll_wait(clientepid, events, MAX_SERVER_EVENT, 20);
      for (int i = 0; i < nfds; ++i)
      {
        // recv data
        if (events[i].events & EPOLLIN)
        {
          std::vector<unsigned char> buffer;
          buffer.resize(MAX_BUFSIZE);

          int ret = ::recv(clientsock, &(buffer[0]), MAX_BUFSIZE, 0);
          std::cout << "socket 接受data," << ret << std::endl;
          if (ret > 0 && ret < MAX_BUFSIZE)
          {
            std::cout << "data: ";
            for (int i = 0; i < ret; ++i)
              std::cout << " " << buffer[i];
            std::cout << std::endl;

            int* readint = (int*)&(buffer[0]);
            std::cout << "the interger recv is:" << readint[0] << std::endl;
          }
        }
      }
    }
  }

  return 0;
}

