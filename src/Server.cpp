#include "Server.hpp"

#include <sys/epoll.h>

#include <cerrno>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

#include "Client.hpp"
#include "Logger.hpp"
#include "Parser.hpp"
#include "Utils.hpp"

#define SERVER_NAME "usvaIRC"
#include <cstring>

#include "Client.hpp"
#include "Logger.hpp"

Server::Server(const int32_t port, const uint32_t backlogSize,
               const std::string &pwd)
    : _listenSocket(Socket::makeListeningSocket(port)),
      _port(port),
      _backlogSize(backlogSize),
      _pwd(pwd) {
  int sendBufSize = SNDBUF_SIZE;
  int receiveBufSize = RCVBUF_SIZE;

  if (setsockopt(_listenSocket.getFD(), SOL_SOCKET, SO_RCVBUF, &receiveBufSize,
                 sizeof(receiveBufSize)) < 0)
    throw std::runtime_error(
        "Failed to set server listen socket receive buffer size");
  if (setsockopt(_listenSocket.getFD(), SOL_SOCKET, SO_SNDBUF, &sendBufSize,
                 sizeof(sendBufSize)) < 0)
    throw std::runtime_error(
        "Failed to set server listen socket send buffer size");
}

void Server::start(void) {
  if ((_epollFD = epoll_create(_backlogSize)) < 0)
    throw std::runtime_error("Failed to set server");
  _epoll.events = EPOLLIN;
  _epoll.data.fd = _listenSocket.getFD();
  if (epoll_ctl(_epollFD, EPOLL_CTL_ADD, _listenSocket.getFD(), &_epoll) < 0)
    throw std::runtime_error("Failed to start polling on listening socket");
  _epollEvents = new struct epoll_event[_backlogSize];
}

// FIXME: Should we store client IP and port to a struct inside Socket *
// (not NULL arguments in accept?)
// int32_t clientFD = accept(_listener->getFD(),
//                           (struct sockaddr *)&CLIENTADDRESS,
//                           &CLIENTADDRESSLENGTH);
// FIXME: Do we need to store connectionPoll struct for later use?
// FIXME: What to do if adding clientSocket->getFD to polling fails?
// FIXME: What to do if accept() fails?
void Server::run(void) {
  while (true) {
    LOG << "Polling for new connections. Clients: ";
    LOG << _clients.size();
    _nEpollFDs = epoll_wait(_epollFD, _epollEvents, _backlogSize, POLL_TIME);
    for (int i = 0; i < _nEpollFDs; ++i) {
      // check for disconnected clients and remove them from the map
      if (_epollEvents[i].events & (EPOLLHUP | EPOLLRDHUP | EPOLLERR)) {
        removeClient(_epollEvents[i].data.fd);
        continue;
      }

      if (_epollEvents[i].data.fd == _listenSocket.getFD()) {
        while (true) {  // loop until accept() returns -1 and
                        // errno is EAGAIN or EWOULDBLOCK
          int32_t clientFD = accept(_listenSocket.getFD(), NULL, NULL);
          if (clientFD == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
              break;
            } else {
              throw std::runtime_error("fatal error accepting clients");
            }
          } else {
            LOG << "New connection accepted on FD: " << clientFD;
            _sockets.try_emplace(clientFD, Socket::makeClientSocket(clientFD));
            _clients.try_emplace(clientFD, Client());

            struct epoll_event connectionPoll{};
            connectionPoll.events = EPOLLIN | EPOLLHUP | EPOLLRDHUP | EPOLLERR;
            connectionPoll.data.fd = clientFD;
            if (epoll_ctl(_epollFD, EPOLL_CTL_ADD, clientFD, &connectionPoll) <
                0) {
              removeClient(clientFD);
              std::cerr << "Failed to add connection to polling list\n";
              continue;
            }
          }
        }
      } else {
        char    buffer[2048] = {};
        auto    it = _sockets.find(_epollEvents[i].data.fd);
        ssize_t received;
        if (it != _sockets.end()) {
          received = it->second.receiveData(buffer, sizeof(buffer));
        } else {
          epoll_ctl(_epollFD, EPOLL_CTL_DEL, _epollEvents[i].data.fd, NULL);
          continue;
        }
        if (received <= 0) {
          std::cerr << "failed to receive\n";
          continue;
        } else {
          std::cout << "bytes: " << received << std::endl;
          LOG << "client " << _epollEvents[i].data.fd
              << " received: " << buffer;
          auto client = _clients.find(_epollEvents[i].data.fd);
          if (client != _clients.end()) {
            client->second.appendToRecvBuffer(buffer);
            std::string command;
            while (!(command = client->second.extractMessage()).empty()) {
              auto cmd = Parser::parse(command);
              LOG << "Received full command: " << command;
              processMessage(client->second, cmd);
            }
          } else {
            std::cerr << "Got rogue network packet" << std::endl;
          }
        }
      }
    }
  }
}

bool Server::passwordIsCorrect(const std::string &pwd) {
  return (_pwd.compare(pwd) == 0);
}

void Server::removeClient(int fd) {
  _clients.erase(fd);
  _sockets.erase(fd);
}

void Server::handlePassword(Client &client, const Command &cmd) {
  LOG << "handling PASS command";
  if (!client.isRegistered()) {
    replyMessage(client, Numeric::ERR_ALREADYREGISTRED,
                 ":Unauthorized command (already registered)");
    return;
  }

  if (cmd.params.empty()) {
    replyMessage(client, Numeric::ERR_NEEDMOREPARAMS,
                 "PASS :Not enough parameters");
    return;
  }

  if (passwordIsCorrect(cmd.params[0])) {
    LOG << "Password matches";
    client.setPasswordOK(true);
  } else {
    LOG << "Password doesn't match";
    replyMessage(client, Numeric::ERR_PASSWDMISMATCH, ":Incorrect password");
  }
}

void Server::handleNickname(Client &client, const Command &cmd) {
  LOG << "handling NICK command";
  if (!client.isPasswordOK()) {
    replyMessage(client, Numeric::ERR_PASSWDMISMATCH, ":Incorrect password");
    return;
  }

  if (cmd.params.empty()) {
    replyMessage(client, Numeric::ERR_NONICKNAMEGIVEN, ":No nickname given");
    return;
  }

  if (client.isRegistered() ||
      client.getState() == Client::State::NICK_RECEIVED) {
    replyMessage(client, Numeric::ERR_ALREADYREGISTRED,
                 ":Unauthorized command (already registered)");
    return;
  }

  if (Utils::validateNickname(cmd.params[0])) {
    if (isNicknameInUse(cmd.params[0])) {
      replyMessage(client, Numeric::ERR_NICKNAMEINUSE,
                   ":Nickname already in use");
      return;
    } else {
      client.setNickname(cmd.params[0]);
      client.setState(Client::State::NICK_RECEIVED);
      if (client.isRegistered()) {
        sendWelcomeMessages(client);
      }
    }
  } else {
    replyMessage(client, Numeric::ERR_ERRONEUSNICKNAME, ":Erroneous nickname");
    return;
  }
}

void Server::handleUserJoin(Client &client, const Command &cmd) {
  LOG << "handling USER command";
  if (!client.isPasswordOK()) {
    replyMessage(client, Numeric::ERR_PASSWDMISMATCH, ":Incorrect password");
    return;
  }

  if (client.isRegistered() ||
      client.getState() == Client::State::USER_RECEIVED) {
    replyMessage(client, Numeric::ERR_ALREADYREGISTRED,
                 ":Unauthorized command (already registered)");
    return;
  }

  if (cmd.params.empty() || cmd.params.size() != 4) {
    replyMessage(client, Numeric::ERR_NEEDMOREPARAMS,
                 "USER :Incorrect parameter count");
    return;
  }

  if (cmd.params[0].find_first_of("@!") != std::string::npos) {
    // Could also just remove illegal chars instead of rejecting message?
    replyMessage(client, Numeric::ERR_NEEDMOREPARAMS,
                 "USER :Illegal characters in username");
    return;
  }

  client.setUsername(cmd.params[0].substr(0, 10));
  client.setRealname(cmd.params[3].substr(0, 50));
  client.setState(Client::State::USER_RECEIVED);
  if (client.isRegistered()) {
    sendWelcomeMessages(client);
  }
}

void Server::handleCapNegotiation(Client &client, const Command &cmd) {
  (void)client, (void)cmd;
}

void Server::sendWelcomeMessages(Client &client) {
  (void)client;
}

void Server::processMessage(Client &client, std::optional<Command> const &cmd) {
  if (cmd.has_value()) {
    auto it = _functionMap.find(cmd->command);
    if (it != _functionMap.end()) {
      auto handler = it->second;
      (this->*handler)(client, *cmd);
    } else {
      replyMessage(client, Numeric::ERR_UNKNOWNCOMMAND,
                   cmd->command + " :command not known");
    }
  } else {
    LOG << "Malformed message received from " << client.getNickname();
  }
}

Server::~Server(void) {
  if (_epollFD != -1)
    close(_epollFD);
  if (_epollEvents)
    delete[] _epollEvents;
}

void Server::replyMessage(Client &client, int code, std::string const &msg) {
  std::ostringstream message;

  message << ":" << SERVER_NAME << " ";
  message << std::setw(3) << std::setfill('0') << code << " ";
  std::string target = client.getNickname();
  if (target.empty() || !client.isRegistered())
    target = "*";
  message << target << " ";
  message << msg << " \r\n";
  client.appendToResponseBuffer(message.str());
}

bool Server::isNicknameInUse(std::string const &nick) {
  for (auto &[fd, client] : _clients) {
    if (client.getNickname().compare(nick) == 0) {
      return true;
    }
  }
  return false;
}

void Server::disconnectUser(int32_t fd) {
  epoll_ctl(_epollFD, EPOLL_CTL_DEL, fd, NULL);
  _clients.erase(fd);
  _sockets.erase(fd);
}

// INFO: Channel management:
Channel &Server::newChannel(const Client &client, const std::string &name) {
  _channels.emplace_back(std::make_unique<Channel>(*this, client, name));
  return (*_channels.back());
}

std::vector<std::unique_ptr<Channel>> &Server::getChannels(void) {
  return (_channels);
}

Channel &Server::findChannel(const std::string &target) const {
  for (const auto &e : _channels) {
    if (e->getName() == target)
      return (*e);
  }
  throw std::runtime_error("Channel with name " + target + " not found");
}
