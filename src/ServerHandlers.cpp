#include <assert.h>

#include <iostream>
#include <span>
#include <string>

#include "Client.hpp"
#include "Logger.hpp"
#include "Server.hpp"
#include "Utils.hpp"

Channel &Server::newChannel(const Client &client, const std::string &name) {
  _channels.try_emplace(name, std::make_unique<Channel>(*this, client, name));
  return (*_channels.at(name));
}

std::unordered_map<std::string, std::unique_ptr<Channel>> &Server::getChannels(
    void) {
  return (_channels);
}

std::optional<std::reference_wrapper<Channel>> Server::findChannel(
    const std::string &channelName) const {
  auto it = _channels.find(channelName);
  if (it != _channels.end()) {
    return (std::ref(*(*it).second));
  }
  return (std::nullopt);
}

// INFO: PRIVMSG
void Server::handlePrivMsg(int32_t fd, const Command &cmd) {
  LOG << "handling PRIVMSG command";
  std::string buffer;
  for (auto &it : std::span(cmd.params).subspan(1))
    buffer += it;
  OptionalClient sender = _clients.at(fd);
  if (!sender)
    return;
  std::string prefix = ":" + sender->get().getNickname() + "!~" +
                       sender->get().getUsername() + "@" +
                       sender->get().getHostname();
  // INFO: channel
  if (cmd.params[0][0] == '#') {
    OptionalChannel channel = findChannel(cmd.params[0]);
    if (!channel) {
      channel = newChannel(*sender, cmd.params[0]);
      _channels.try_emplace(cmd.params[0], &channel->get());
    }
    std::string fullMessage =
        prefix + " PRIVMSG " + cmd.params[0] + " :" + buffer + "\r\n";
    for (auto &member : channel->get().getUsers()) {
      if (!member.second || !member.second->getClient() ||
          member.second->getClient() == &sender->get())
        continue;
      const auto &nick = member.second->getNickName();
      auto        it = _nickToFd.find(nick);
      if (it == _nickToFd.end())
        continue;
      replyMessage(it->second, fullMessage);
    }
    return;
  }

  // INFO: /msg
  OptionalClient target = findClientByName(cmd.params[0]);
  if (!target) {
    std::string errStr =
        sender->get().getNickname() + " " + cmd.params[0] + " :No such nick";
    replyNumeric(fd, Numeric::ERR_NOSUCHNICK, errStr);
    return;
  }
  std::string targetNick(target->get().getNickname());
  std::string fullMessage =
      prefix + " PRIVMSG " + targetNick + " :" + buffer + "\r\n";
  replyMessage(_nickToFd.at(targetNick), fullMessage);
}

// INFO: KICK
void Server::handleKick(int32_t fd, const Command &cmd) {
  LOG << "handling KICK command";
  if (cmd.params.size() < 2) {
    replyNumeric(fd, Numeric::ERR_NEEDMOREPARAMS, ":Not enough parameters");
    return;
  }

  //  FIXME: vv Throw here only for development/debugging purposes vv
  if (cmd.params.size() > 3) {
    throw std::runtime_error("Too many params for KICK command");
  }
  // FIXME: ^^ Throw here only for development/debugging purposes ^^

  OptionalChannel          channel;
  std::vector<std::string> users;
  std::string              comment;
  for (size_t i = 0; i < cmd.params.size(); ++i) {
    switch (i) {
      case 0: {
        channel = findChannel(cmd.params[i]);
        if (!channel.has_value()) {
          replyNumeric(fd, Numeric::ERR_NOSUCHCHANNEL, ":No such channel");
          return;
        }
        break;
      }
      case 1: {
        std::string       user;
        std::stringstream userStream(cmd.params[i]);
        while (std::getline(userStream, user, ',')) {
          std::cout << user << std::endl;
          users.push_back(user);
        }
        break;
      }
      case 2: {
        comment = cmd.params[i];
        std::cout << comment << std::endl;
        break;
      }
    }
  }
  for (size_t i = 0; i < users.size(); ++i) {
    OptionalClient clientToKick = findClientByName(users[i]);
    if (!clientToKick.has_value()) {
      replyNumeric(fd, Numeric::ERR_NOSUCHNICK, ":No such nick");
      continue;
    }
    OptionalUser user = channel->get().findUser(users[i]);
    if (!user.has_value()) {
      replyNumeric(fd, Numeric::ERR_NOSUCHNICK,
                   ":No such nick " + users[i] + " on channel " +
                       channel->get().getName());
      continue;
    }
    int32_t     fdToKick = _nickToFd.at(users[i]);
    std::string message =
        cmd.command + " " + channel->get().getName() + " " + users[i];
    if (comment.empty() == false) {
      message += ": " + comment + "\r\n";
    } else {
      message += "\r\n";
    }
    replyMessage(fdToKick, message);
    channel->get().kickUser(user->get());
  }
}

// INFO: JOIN
void Server::handleJoin(int32_t fd, const Command &cmd) {
  LOG << "handling JOIN command";
  if (cmd.params.size() < 1) {
    replyNumeric(fd, Numeric::ERR_NEEDMOREPARAMS, ":Not enough parameters");
    return;
  }

  //  FIXME: vv Throw here only for development/debugging purposes vv
  if (cmd.params.size() > 2) {
    throw std::runtime_error("Too many params for JOIN command");
  }
  // FIXME: ^^ Throw here only for development/debugging purposes ^^

  std::vector<std::string> channelNames;
  std::vector<std::string> channelKeys;
  for (size_t i = 0; i < cmd.params.size(); ++i) {
    switch (i) {
      case 0: {
        std::string       channel;
        std::stringstream channelStream(cmd.params[i]);
        while (std::getline(channelStream, channel, ',')) {
          channelNames.push_back(channel);
        }
        break;
      }
      case 1: {
        std::string       key;
        std::stringstream keyStream(cmd.params[i]);
        while (std::getline(keyStream, key, ',')) {
          channelKeys.push_back(key);
        }
        break;
      }
    }
  }
  // FIXME: Should we allow the amount of keys be different than channel amount?
  if (channelKeys.size() > 0 && channelNames.size() != channelKeys.size()) {
    replyNumeric(fd, Numeric::ERR_NEEDMOREPARAMS, ":Not enough parameters");
    return;
  }
  for (size_t i = 0; i < channelNames.size(); ++i) {
    OptionalChannel channel = findChannel(channelNames[i]);
    Client         &clientToAdd = _clients.at(fd);
    LOG << clientToAdd.getNickname() + " trying to join channel " +
               channelNames[i];
    if (!channel.has_value()) {
      LOG << channelNames[i] + " not found. Creating channel " +
                 channelNames[i];
      newChannel(clientToAdd, channelNames[i]);
      continue;
    } else {
      LOG << channelNames[i] + " found. " + clientToAdd.getNickname() +
                 " joining the joining";
      // FIXME: Need to implement password checks!
      channel->get().addUser(clientToAdd);
    }
  }
}
// void Server::handleJoin(int32_t fd, const Command &cmd) {
//   LOG << "handling JOIN command";
//   Client         &client = _clients.at(fd);
//   OptionalChannel channel = findChannel(cmd.params[0]);
//   if (!channel) {
//     std::cout << "channel not found, creating new channel " << cmd.params[0]
//               << '\n';
//     channel = newChannel(_clients.find(fd)->second, cmd.params[0]);
//     _channels.try_emplace(cmd.params[0], &channel->get());
//   }
//   channel->get().addUser(client);
//   if (cmd.params.size() < 1) {
//     replyNumeric(fd, Numeric::ERR_NEEDMOREPARAMS, ":Not enough parameters");
//     return;
//   }
// }

// INFO: QUIT
void Server::handleQuit(int fd, const Command &cmd) {
  std::string quitMsg = "Client Quit";
  if (!cmd.params.empty()) {
    quitMsg = cmd.params[0];
  }
  Client     &client = _clients.at(fd);
  std::string errorMsg = "ERROR :Closing Link: (Quit: " + quitMsg + ")\r\n";
  replyMessage(fd, errorMsg);
  client.setShouldClose(true);
  LOG << "Client " << fd << " initiated QUIT sequence.";
}
