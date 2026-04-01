#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

class Channel {
  public:
    class User;

  private:
    uint16_t     _channelFlags = 0;
    std::string  _key = "";
    unsigned int _userLimit = 0;

    // NOTE: Nickname masks
    std::string banMask = "";
    std::string banExceptionMask = "";
    std::string invitationMask = "";

    std::vector<Channel::User> _users;

  public:
    Channel();
    Channel(const Channel &other) = delete;
    Channel &operator=(const Channel &other) = delete;
    ~Channel();

    class User {
      private:
        // NOTE: Store the global client in here
        uint16_t _privileges = 0;

      public:
        User();
        User(const User &other) = delete;
        User &operator&(const User &other) = delete;
        ~User();

        enum class Privilege : uint16_t {
          OWNER = 1,
          OPERATOR = 1 << 1,
        };

        // FIXME: Use toggle or separate add and remove?
        // void togglePrivilege(const Privilege privilege);
        // TODO: add privilege to user
        void addPrivilege(const Privilege privilege);
        // TODO: remove privilege from user
        void removePrivilege(const Privilege privilege);
    };

    enum class ChannelFlag : uint16_t {
      // NOTE: Mandatory:
      INVITE_ONLY = 1,
      TOPIC_SET_MY_CHANOP_ONLY = 1 << 1,
      PASSWORD_PROTECTED = 1 << 2,
      LIMITED_USER_COUNT = 1 << 3,

      // WARN: Not in subject:
      // ANONYMOUS = 1 << 4,
      // MODERATED = 1 << 5,
      // NO_MESSAGES_FROM_OUTSIDE = 1 << 6,
      // QUIET_CHANNEL = 1 << 7,
      // PRIVATE_CHANNEL = 1 << 8,
      // SECRET_CHANNEL = 1 << 9,
      // SERVER_REOP_CHANNEL = 1 << 10,
    };

    // NOTE: Operator commands:
    void kickUser(User &user);
    void inviteUser(User &user);
    void changeTopic(const std::string &topic);
    void viewTopic(void);
    // void setMode(Mode mode);

    void toggleChannelFlag(const ChannelFlag flag);
    void resetChannelFlags(void);

    // FIXME: Mandatory: Use separate function for all (wrapper for
    // toggleChannelFlag()) or just use the toggleChannelFlag()?

    // TODO: i - toggle the invite - only channel                      flag;
    void toggleInviteOnly(void);

    // TODO: t - toggle the topic settable by channel operator only flag;
    void toggleTopicSettableByChanopOnly(void);

    // TODO: k - set / remove the channel       key(password);
    void toggleChannelKey(const std::string &key);

    // TODO: o - give / take channel   operator privilege;
    void toggleChannelOperatorPrivilege(User &user);

    // TODO: l - set / remove the user limit to channel;
    void setUserLimit(const unsigned int limit);

    /* WARN: Not defined in subject
     * a - toggle the anonymous channel flag;
     * m - toggle the moderated channel;
     * n - toggle the no messages to channel from clients on the outside;
     * q - toggle the quiet channel flag;
     * p - toggle the private channel flag;
     * s - toggle the secret channel flag;
     * r - toggle the server reop channel flag;
     * O - give "channel creator" status;
     * v - give / take the voice privilege;
     * b - set / remove ban mask to keep users out;
     * e - set / remove an exception mask to override a ban mask;
     * I - set / remove an invitation mask to automatically override the
     * invite-only flag; };
     */
};

// NOTE: Channel::ChannelFlag:
uint16_t operator<<(uint16_t shift, Channel::ChannelFlag flag);

// NOTE: Channel::User::Privilege:
uint16_t operator<<(uint16_t shift, Channel::User::Privilege privilege);
