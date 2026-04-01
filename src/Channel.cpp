#include "Channel.hpp"

Channel::Channel() {}

Channel::~Channel() {}

void Channel::resetChannelFlags(void) {
  _channelFlags = 0;
}

void Channel::toggleChannelFlag(const ChannelFlag flag) {
  _channelFlags ^= 1 << flag;

  // FIX: Switch to handle more logic? ChannelFlag already quarantees type
  // safety switch (flag) {
  //   case ChannelFlag::ANONYMOUS:
  //     break;
  //   case ChannelFlag::INVITE_ONLY:
  //     break;
  //   case ChannelFlag::MODERATED:
  //     break;
  //   case ChannelFlag::NO_MESSAGES_FROM_OUTSIDE:
  //     break;
  //   case ChannelFlag::QUIT_CHANNEL:
  //     break;
  //   case ChannelFlag::PRIVATE_CHANNEL:
  //     break;
  //   case ChannelFlag::SECRET_CHANNEL:
  //     break;
  //   case ChannelFlag::SERVER_REOP_CHANNEL:
  //     break;
  //   case ChannelFlag::TOPIC_SET_MY_CHANOP_ONLY:
  //     break;
  //   default:
  //     throw std::runtime_error("Unidentified flag");
  //     break;
  // }
}

uint16_t operator<<(uint16_t shift, Channel::ChannelFlag flag) {
  return (shift << static_cast<uint16_t>(flag));
}

// NOTE: USER:

Channel::User::User() {}
Channel::User::~User() {}

uint16_t operator<<(uint16_t shift, Channel::User::Privilege privilege) {
  return (shift << static_cast<uint16_t>(privilege));
}
