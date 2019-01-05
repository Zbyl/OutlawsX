#pragma once

#include "zpreprocessor.h"

#include <iostream>
#include <sstream>

/// Usage:
/// LOG() << "Memory usage: " << 5;         - equivalent to LOG(INFO)
///
/// LOG(ERROR) << "Memory usage: " << 5;
/// LOG(WARNING) << "Memory usage: " << 5;
/// LOG(INFO) << "Memory usage: " << 5;
/// LOG(DEBUG) << "Memory usage: " << 5;


#if ZVA_OPT_SUPPORTED

#define LOG(...) (terminal_editor::LogHelper( (LogLevel::INFO __VA_OPT__(, LogLevel::) __VA_ARGS__),  __FILE__ "(" ZTOKEN_STRINGIZE(__LINE__) "): ").message)

#else

#define LOGHACK_        LogLevel::INFO
#define LOGHACK_ERROR   LogLevel::ERROR
#define LOGHACK_WARNING LogLevel::WARNING
#define LOGHACK_INFO    LogLevel::INFO
#define LOGHACK_DEBUG   LogLevel::DEBUG

#define LOG(...) (terminal_editor::LogHelper(LOGHACK_ ## __VA_ARGS__,  __FILE__ "(" ZTOKEN_STRINGIZE(__LINE__) "): ").message)

#endif


namespace terminal_editor {

enum class LogLevel {
    ERROR,
    WARNING,
    INFO,
    DEBUG,
};

class LogHelper {
public:
    LogLevel logLevel;
    std::stringstream message;

    LogHelper(LogLevel logLevel, const char* messageBase)
        : logLevel(logLevel)
    {
        switch (logLevel) {
            case LogLevel::ERROR:   message << "ERROR: "; break;
            case LogLevel::WARNING: message << "WARNING: "; break;
            case LogLevel::INFO:    message << "INFO: "; break;
            case LogLevel::DEBUG:   message << "DEBUG: "; break;
        }
        message << messageBase;
    }

    ~LogHelper() noexcept(false) {
        auto messageStr = message.str();
        if ( (logLevel == LogLevel::ERROR) || (logLevel == LogLevel::WARNING) )
            std::cerr << messageStr << std::endl;
        else
            std::cout << messageStr << std::endl;
    }
};

} // namespace terminal_editor
