#pragma once
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>

using TimeStamp = std::chrono::time_point<std::chrono::steady_clock>;

constexpr size_t logBufferSize = 1024 * 1024;
constexpr size_t diskCheckInterval = 5;

class Logger {
  public:
    Logger(const char *file, int line);
    ~Logger();

    template <typename T>
    Logger &operator<<(const T &msg) {
      oss_ << msg;
      return *this;
    }
    static void setLogFile(const std::string &filename);

  private:
    std::ostringstream oss_;
    const char        *file_;
    int                line_;

    static std::mutex    logMutex_;
    static std::ofstream logFile_;
    static std::string   logPath_;
    static TimeStamp     lastCheck_;
    static bool          hasSpace_;

    std::string getTimestamp();

    /**
     * @brief Checks if there is sufficient disk space: the length of the
     * message + logBufferSize (1MB)
     *
     * @param line The string to be logged
     * @return bool
     */
    static bool hasDiskSpace(const std::string &line);
};

#define LOG Logger(__FILE__, __LINE__)
