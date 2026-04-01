#include "Channel.hpp"

Channel::Channel() {}

Channel::~Channel() {}

void Channel::resetChannelFlags(void) {
  _channelFlags = 0;
}

void Channel::toggleChannelFlag(const ChannelFlag flag) {
  switch (flag) {
    case ChannelFlag::ANONYMOUS:
      _channelFlags ^= (1 << static_cast<uint16_t>(flag));
      break;
    case ChannelFlag::INVITE_ONLY:
      break;
    case ChannelFlag::MODERATED:
      break;
    case ChannelFlag::NO_MESSAGES_FROM_OUTSIDE:
      break;
    case ChannelFlag::QUIT_CHANNEL:
      break;
    case ChannelFlag::PRIVATE_CHANNEL:
      break;
    case ChannelFlag::SECRET_CHANNEL:
      break;
    case ChannelFlag::SERVER_REOP_CHANNEL:
      break;
    case ChannelFlag::TOPIC_SET_MY_CHANOP_ONLY:
      break;
    default:
      throw std::runtime_error("Unidentified flag");
      break;
  }
}
