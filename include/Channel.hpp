#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

// a - toggle the anonymous channel flag;
// i - toggle the invite - only channel                      flag;
// m - toggle the moderated                                  channel;
// n - toggle the no messages to channel from clients on the outside;
// q - toggle the quiet channel                              flag;
// p - toggle the private channel                            flag;
// s - toggle the secret channel                             flag;
// r - toggle the server reop channel                        flag;
// t - toggle the topic settable by channel operator only flag;
class Channel {
  private:
    uint16_t     _channelFlags = 0;
    std::string  _key = "";
    unsigned int _userLimit = 0;

    // NOTE: Nickname masks
    std::string banMask = "";
    std::string banExceptionMask = "";
    std::string invitationMask = "";

    // TODO: Vector of clients
    // TODO: Vector of chanops
    // TODO: Vector of channel creators

  public:
    Channel();
    Channel(const Channel &other) = delete;
    Channel &operator=(const Channel &other) = delete;
    ~Channel();

    enum class ChannelFlag : uint16_t {
      // NOTE: Mandatory:
      INVITE_ONLY = 1,
      TOPIC_SET_MY_CHANOP_ONLY = 1 << 1,
      PASSWORD_PROTECTED = 1 << 2,
      LIMITED_USER_COUNT = 1 << 3,

      // NOTE: Not in subject:
      ANONYMOUS = 1 << 4,
      MODERATED = 1 << 5,
      NO_MESSAGES_FROM_OUTSIDE = 1 << 6,
      QUIET_CHANNEL = 1 << 7,
      PRIVATE_CHANNEL = 1 << 8,
      SECRET_CHANNEL = 1 << 9,
      SERVER_REOP_CHANNEL = 1 << 10,
    };

    // NOTE: Operator commands:
    // void kickUser(User &user);
    // void inviteUser(User &user);
    // void changeTopic(const std::string &topic);
    // void viewTopic(void);
    // void setMode(Mode mode);

    void toggleChannelFlag(const ChannelFlag flag);
    void resetChannelFlags(void);

    // O - give "channel creator" status;
    void giveChannelCreatorStatus(void);

    // o - give / take channel   operator privilege;
    void toggleChannelOperatorPrivilege(void);

    // v - give / take the voice privilege;
    void toggleVoicePrivilege(void);

    // k - set / remove the channel       key(password);
    void toggleChannelKey(void);

    // l - set / remove the user limit to channel;
    void toggleUserLimit(void);

    // b - set / remove ban mask to keep users                           out;
    void toggleBanMask(void);

    // e - set / remove an exception mask to override a ban              mask;
    void toggleExceptionMaskToOverrideBanMask(void);

    // I - set / remove an invitation mask to automatically override the
    // invite-only flag; };
    void toggleInvitationMaskToOverrideInviteOnlyFlag(void);
};

uint16_t operator<<(uint16_t shift, Channel::ChannelFlag flag);
