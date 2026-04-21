#include <sys/epoll.h>

#include <charconv>
#include <exception>
#include <iostream>
#include <string>

#include "Logger.hpp"
#include "Server.hpp"

#ifndef GIT_HASH
#define GIT_HASH "unknown_build"
#endif

#define MAX_EVENTS 10
struct epoll_event ev, events[MAX_EVENTS];
int                epollfd, nfds;

int32_t validatePortValue(const char *port) {
  int32_t       val{};
  const int32_t len = std::strlen(port);
  auto [ptr, ec] = std::from_chars(port, port + len, val);
  const bool not_all_digits = (ptr != port + len);
  const bool out_of_range = (val < 1024) || (val > 65535);
  if (ec != std::errc{} || not_all_digits || out_of_range) {
    std::cerr << "Invalid port range/value" << std::endl;
    std::exit(EXIT_FAILURE);
  }
  return val;
}

int main(int ac, char **av) {
  if (ac != 3) {
    std::cerr << "Please use args <port> <password>" << std::endl;
    return (1);
  }
  try {
    Logger::setLogFile("irc_server.log");
    LOG << "Starting server, build: " << GIT_HASH;
    Server server(validatePortValue(av[1]), BACKLOG_SIZE, av[2]);
    server.start();
    server.run();
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return (1);
  }
}
