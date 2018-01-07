#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <vector>

using namespace std;
using std::vector;

const string server_ip = "172.24.15.237";
#define SERVER_PORT 9888
#define MAX_BUFSIZE 65535

int main()
{
  int sockid = 0;
  sockid = socket(AF_INET, SOCK_STREAM, 0);
  if (sockid < 0)
  {
    std::cout << "socket error" << std::endl;
    return -1;
  }

  struct timeval timeo;
  timeo.tv_sec = 0;
  timeo.tv_usec = 100000; //100 ms

  int len = sizeof(timeo);
  setsockopt(sockid, SOL_SOCKET, SO_SNDTIMEO, &timeo, len);

  sockaddr_in addr;
  bzero(&addr, sizeof(addr));

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
  addr.sin_port = htons((uint16_t)SERVER_PORT);

  int ret = ::connect(sockid, (sockaddr*)&addr, sizeof(sockaddr_in));
  if (ret != 0)
  {
    std::cout << "connet error: " << ret << std::endl;
    return -1;
  }
  std::cout << "connect success!" << std::endl;

  // set send and recv
  int nBuf = MAX_BUFSIZE * 2;
  setsockopt(sockid,SOL_SOCKET,SO_RCVBUF,(const char*)&nBuf,sizeof(int));
  setsockopt(sockid,SOL_SOCKET,SO_SNDBUF,(const char*)&nBuf,sizeof(int));

  // set no block
  int flags = fcntl(sockid, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(sockid, F_SETFL, flags);


  //sleep(3);
  vector<int> testsend;
  testsend.push_back(1234);

  ret = ::send(sockid, &(testsend[0]), testsend.size() * sizeof(int), 0);
  if (ret > 0)
    std::cout << "send success, ret:" << ret << std::endl;

  return 0;
}

