#include "Logger.hpp"

#include <fcntl.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>

// static variable inits
std::mutex    Logger::logMutex_;
std::ofstream Logger::logFile_;
std::string   Logger::logPath_;
TimeStamp     Logger::lastCheck_;
bool          Logger::hasSpace_ = true;

Logger::Logger(const char *file, int line) : file_(file), line_(line) {}

std::string Logger::getTimestamp() {
  std::time_t now_time =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::ostringstream ss;
  ss << std::put_time(std::localtime(&now_time), "%Y-%m-%d %H:%M:%S");
  return ss.str();
}

Logger::~Logger() {
  std::string        fileStr(file_);
  size_t             lastSlash = fileStr.find_last_of("/\\");
  std::string        shortFile = (lastSlash == std::string::npos)
                                     ? fileStr
                                     : fileStr.substr(lastSlash + 1);
  std::ostringstream finalLog;
  finalLog << "[" << getTimestamp() << "] "
           << "[" << shortFile << ":" << line_ << "] " << oss_.str() << "\n";
  std::lock_guard<std::mutex> lock(logMutex_);
  if (logFile_.is_open()) {
    std::string line = finalLog.str();
    if (hasDiskSpace(line)) {
      logFile_ << line;
      logFile_.flush();
    } else {
      std::cerr << "Insufficent disk space for logging, nothing written"
                << std::endl;
    }
  }
}

void Logger::setLogFile(const std::string &filename) {
  logFile_.open(filename, std::ios_base::app);
  if (!logFile_.is_open()) {
    std::cerr << "Failed to open log file: " << filename << std::endl;
  } else {
    logPath_ = filename;
  }
  hasSpace_ = hasDiskSpace(filename);
}

bool Logger::hasDiskSpace(const std::string &line) {
  TimeStamp now = std::chrono::steady_clock::now();
  if (now - lastCheck_ < std::chrono::seconds(diskCheckInterval))
    return hasSpace_;
  try {
    lastCheck_ = now;
    std::filesystem::space_info si = std::filesystem::space(logPath_);
    hasSpace_ = ((si.available > line.length() + logBufferSize) ? true : false);
  } catch (std::filesystem::filesystem_error &err) {
    std::cerr << "Error checking disk space: " << err.what() << std::endl;
    hasSpace_ = false;
  }
  return hasSpace_;
}
