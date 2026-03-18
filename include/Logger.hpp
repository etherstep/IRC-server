#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <sstream>
#include <string>
#include <mutex>
#include <fstream>


class Logger {
public:
    Logger(const char* file, int line);
    ~Logger();

    template <typename T>
    Logger& operator<<(const T& msg) {
        oss_ << msg;
        return *this;
    }
    static void setLogFile(const std::string& filename);

private:
    std::ostringstream oss_;
    const char* file_;
    int line_;

    static std::mutex logMutex;
    static std::ofstream logFile;

    std::string getTimestamp();
};

#define LOG Logger(__FILE__, __LINE__)

#endif // LOGGER_HPP
