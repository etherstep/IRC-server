#ifndef Client_HPP
#define Client_HPP
#include <string>

#include "Socket.hpp"

class Client {
  public:
    enum class State {
      CONNECTED,
      NICK_RECEIVED,
      USER_RECEIVED,
      REGISTERED,
    };
    Client(Socket *soc);
    ~Client();

    /**
     * @brief This gets called when epoll() informs that the underlying FD has
     * data. Reads data from the underlying Socket, and appends it to the
     * socketBuffer. Calls checkBuffer to update status.
     */
    void readSocket();

    /**
     * @brief Extracts a message from the socketBuffer. Calls checkBuffer to
     * update status.
     * @return std::string The first full message contained in the socketBuffer.
     */
    std::string_view extractMessage();

    void eraseMessage();

    std::string getNickname();
    void        setNickname(std::string const &nick);

    State getState();
    void  setState(State s);

    std::string getUserName();
    void        setUserName(std::string const &name);

    std::string getRealName();
    void        setRealName(std::string const &name);

    bool isRegistered();

    void        fakeAppendToBuffer(std::string const &input);
    void        appendToOutgoing(std::string const &msg);
    std::string getOutgoingBuffer();

    void setPasswordOK(bool b);
    bool isPasswordOK();

    bool hasMessage();
    bool isOpen();

  private:
    Socket     *_socket;
    std::string _socketBuffer;
    std::string _outBuffer;
    bool        _hasMessage;
    bool        _isOpen;
    bool        _passwordOK;
    State       _state;
    std::string _nick;
    std::string _userName;
    std::string _realName;

    /**
     * @brief Checks if the socketBuffer contains a full message terminated by
     * "\r\n"
     * @return bool Is there a full message in buffer
     */
    bool checkBuffer();
};

#endif
