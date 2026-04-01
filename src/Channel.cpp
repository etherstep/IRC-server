#include "Channel.hpp"

Channel::Channel(const Client &client, const std::string &name) : _name(name) {
  User creator(client);
  creator.addOperatorPrivilege();
  _users.push_back(creator);
}

Channel::~Channel() {}

void Channel::addUser(const Client &client) {
  _users.emplace_back(client);
}

const std::string &Channel::getName(void) const {
  return (_name);
}

unsigned int Channel::getUserCount(void) const {
  return (_users.size());
}

void Channel::resetFlags(void) {
  _channelFlags = 0;
}

void Channel::toggleFlag(const ChannelFlag flag) {
  _channelFlags ^= static_cast<uint16_t>(flag);
}

bool Channel::isFlagOn(const ChannelFlag flag) {
  return (_channelFlags & static_cast<uint16_t>(flag));
}

// NOTE: Channel::User:
Channel::User::User(const Client &client) : _client(client) {}

Channel::User::User(const User &other)
    : _client(other._client), _isOperator(other._isOperator) {}

Channel::User::~User() {}

void Channel::User::toggleOperatorPrivilege(void) {
  _isOperator = !_isOperator;
}

void Channel::User::addOperatorPrivilege(void) {
  _isOperator = true;
}

void Channel::User::removeOperatorPrivilege(void) {
  _isOperator = false;
}
