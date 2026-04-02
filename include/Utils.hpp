#pragma once
#include <string>

namespace Utils {
bool validateNickname(const std::string &nick);
}

namespace Numeric {
// handshake
constexpr int RPL_WELCOME = 1;
constexpr int RPL_YOURHOST = 2;
constexpr int RPL_CREATED = 3;
constexpr int RPL_MYINFO = 4;
// errors
constexpr int ERR_NOSUCHNICK = 401;
constexpr int ERR_NOSUCHCHANNEL = 403;
constexpr int ERR_CANNOTSENDTOCHAN = 404;
constexpr int ERR_INPUTTOOLONG = 417;
constexpr int ERR_UNKNOWNCOMMAND = 421;
constexpr int ERR_NONICKNAMEGIVEN = 431;
constexpr int ERR_ERRONEUSNICKNAME = 432;
constexpr int ERR_NICKNAMEINUSE = 433;
constexpr int ERR_NOTREGISTERED = 451;
constexpr int ERR_NEEDMOREPARAMS = 461;
constexpr int ERR_ALREADYREGISTRED = 462;
constexpr int ERR_PASSWDMISMATCH = 464;
}  // namespace Numeric
