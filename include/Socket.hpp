#ifndef SOCKET_HPP
#define SOCKET_HPP
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstdint>

class Socket {
  public:
    ~Socket();
    Socket *makeListeningSocket(int port);
    Socket *makeClientSocket(int clientFD);

  private:
    Socket(int fd);
    int32_t _fd;
};

#endif  // SOCKET_HPP
