#include "Socket.hpp"

#include "Logger.hpp"

Socket::~Socket() {
  fclose(_fd);
}

Socket *makeListeningSocket(int port) {}
Socket *makeClientSocket(int clientFD);
