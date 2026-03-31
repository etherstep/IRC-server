#include "Server.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "Client.hpp"
#include "Logger.hpp"
#include "Parser.hpp"
#include "Utils.hpp"

#define SERVER_NAME "usvaIRC"

Server::Server(const int32_t port, const uint32_t backlogSize,
               const std::string &pwd)
    : _port(port), _backlogSize(backlogSize), _pwd(pwd) {
  int sendBufSize = SNDBUF_SIZE;
  int receiveBufSize = RCVBUF_SIZE;

  _listenSocket = Socket::makeListeningSocket(port);
  if (_listenSocket == nullptr)
    throw std::runtime_error("Port value out of bounds. Use value 0 - 65535");

  if (setsockopt(_listenSocket->getFD(), SOL_SOCKET, SO_RCVBUF, &receiveBufSize,
                 sizeof(receiveBufSize)) < 0)
    throw std::runtime_error(
        "Failed to set server listen socket receive buffer size");
  if (setsockopt(_listenSocket->getFD(), SOL_SOCKET, SO_SNDBUF, &sendBufSize,
                 sizeof(sendBufSize)) < 0)
    throw std::runtime_error(
        "Failed to set server listen socket send buffer size");
}

void Server::start(void) {
  if ((_epollFD = epoll_create(_backlogSize)) < 0)
    throw std::runtime_error("Failed to set server");
  _epoll.events = EPOLLIN;
  _epoll.data.fd = _listenSocket->getFD();
  if (epoll_ctl(_epollFD, EPOLL_CTL_ADD, _listenSocket->getFD(), &_epoll) < 0)
    throw std::runtime_error("Failed to start polling on listening socket");
  epollEvents = new struct epoll_event[_backlogSize];
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
  char buffer[1024] = {'p', 'i', 'n', 'g', '\r', '\n'};
  while (true) {
    std::cout << "Polling for new connections. Clients: ";
    std::cout << _clients.size() << std::endl;
    _nEpollFDs = epoll_wait(_epollFD, epollEvents, _backlogSize, 1000);
    for (int i = 0; i < _nEpollFDs; ++i) {
      if (epollEvents[i].data.fd == _listenSocket->getFD()) {
        int32_t clientFD = accept(_listenSocket->getFD(), NULL, NULL);
        if (clientFD < 0) {
          std::cerr << "Failed to accept connection to client\n";
          continue;
        }
        Socket *clientSocket = Socket::makeClientSocket(clientFD);

        struct epoll_event connectionPoll;
        connectionPoll.events = EPOLLIN;
        connectionPoll.data.fd = clientSocket->getFD();
        if (epoll_ctl(_epollFD, EPOLL_CTL_ADD, clientSocket->getFD(),
                      &connectionPoll) < 0) {
          std::cerr << "Failed to add connectiont to polling list\n";
          continue;
        }
        Client *connection = new Client(clientSocket);
        _clients.push_back(connection);
        _ClientMap[clientFD] = connection;
      } else {
        _ClientMap[epollEvents[i].data.fd]->readSocket();
        if (_ClientMap[epollEvents[i].data.fd]->hasMessage())
          send(epollEvents[i].data.fd, buffer, 5, 0);
      }
    }
  }
}

bool Server::passwordIsCorrect(const std::string &pwd) {
  return (_pwd.compare(pwd) == 0);
}

void Server::handlePassword(Client *client, const Command &cmd) {
  LOG << "handling PASS command";
  if (!client->isRegistered()) {
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
    client->setPasswordOK(true);
  } else {
    LOG << "Password doesn't match";
    replyMessage(client, Numeric::ERR_PASSWDMISMATCH, ":Incorrect password");
  }
}

void Server::handleNickname(Client *client, const Command &cmd) {
  LOG << "handling NICK command";
  if (!client->isPasswordOK()) {
    replyMessage(client, Numeric::ERR_PASSWDMISMATCH, ":Incorrect password");
    return;
  }

  if (cmd.params.empty()) {
    replyMessage(client, Numeric::ERR_NONICKNAMEGIVEN, ":No nickname given");
    return;
  }

  if (client->isRegistered() ||
      client->getState() == Client::State::NICK_RECEIVED) {
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
      client->setNickname(cmd.params[0]);
      client->setState(Client::State::NICK_RECEIVED);
      if (client->isRegistered()) {
        sendWelcomeMessages(client);
      }
    }
  } else {
    replyMessage(client, Numeric::ERR_ERRONEUSNICKNAME, ":Erroneous nickname");
    return;
  }
}

void Server::handleUserJoin(Client *client, const Command &cmd) {
  LOG << "handling USER command";
  if (!client->isPasswordOK()) {
    replyMessage(client, Numeric::ERR_PASSWDMISMATCH, ":Incorrect password");
    return;
  }

  if (client->isRegistered() ||
      client->getState() == Client::State::USER_RECEIVED) {
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

  client->setUserName(cmd.params[0].substr(0, 10));
  client->setRealName(cmd.params[3].substr(0, 50));
  client->setState(Client::State::USER_RECEIVED);
  if (client->isRegistered()) {
    sendWelcomeMessages(client);
  }
}

void Server::processMessage(Client *client) {
  std::string_view msg = client->extractMessage();
  auto             cmd = Parser::parse(msg);
  client->eraseMessage();
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
    LOG << "Malformed message received from " << client->getNickname();
  }
}

Server::~Server(void) {
  if (_epollFD != -1)
    close(_epollFD);
  if (epollEvents)
    delete[] epollEvents;
}

void Server::replyMessage(Client *client, int code, std::string const &msg) {
  std::ostringstream message;

  message << ":" << SERVER_NAME << " ";
  message << std::setw(3) << std::setfill('0') << code << " ";
  std::string target = client->getNickname();
  if (target.empty() || !client->isRegistered())
    target = "*";
  message << target << " ";
  message << msg << " \r\n";
  client->appendToOutgoing(message.str());
}

bool Server::isNicknameInUse(std::string const &nick) {
  for (auto client : _clients) {
    if (client->getNickname().compare(nick) == 0) {
      return true;
    }
  }
  return false;
}
