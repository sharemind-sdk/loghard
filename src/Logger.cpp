/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#define SHAREMIND_COMMON_INTERNAL__

#include "Logger.h"

#include <cstdlib>
#include <ctime>
#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/RollingFileAppender.hh>
#include "../Abort.h"
#include "../SmartStringStream.h"
#include "Debug.h"
#include "GenericAppender.h"
#include "LogLayout.h"
#include "MessageProcessor.h"


namespace {

SHAREMIND_DEFINE_PREFIXED_LOGS("[Logger] ");

inline log4cpp::Priority::PriorityLevel prioToLog4cppPrio(SharemindLogPriority priority) {
    switch (priority) {
        case LOGPRIORITY_FATAL:     return log4cpp::Priority::FATAL;
        case LOGPRIORITY_ERROR:     return log4cpp::Priority::ERROR;
        case LOGPRIORITY_WARNING:   return log4cpp::Priority::WARN;
        case LOGPRIORITY_NORMAL:    return log4cpp::Priority::INFO;
        case LOGPRIORITY_DEBUG:     return log4cpp::Priority::DEBUG;
        case LOGPRIORITY_FULLDEBUG: return log4cpp::Priority::DEBUG;
        default:
            SHAREMIND_ABORT("pTLP: p=%d", static_cast<int>(priority));
    }
}

/**
 Opens a file for writing. Used for opening log files for the Appenders.

 \param[in] logger Used for logging messages
 \param[in] filename the name for the log file.
 \param[in] append indicates whether the logs will be appended to the end of the file or not.
 \param[out] fd the file descriptor for the log file.

 \retval true if opening the file was successful
 \retval false if opening the file failed
*/
bool openFile(sharemind::Logger & logger,
              const std::string & filename,
              bool append,
              int & fd)
{
    // Check if we have a filename
    if (filename.empty()) {
        LogError(logger) << "Empty log file name!";
        return false;
    }

    // Try to open the log file
    int flags = O_CREAT | O_APPEND | O_WRONLY;
    if (!append)
        flags |= O_TRUNC;

    fd = ::open(filename.c_str(), flags, 00644);
    if (fd < 0) {
        LogError(logger) << "Cannot open log file " << filename << "!";
        return false;
    }

    LogDebug(logger) << "Opened log file " << filename << ".";
    return true;
}

} // anonymous namespace

namespace sharemind {

/**
 * This class reimplements the OstreamAppender so that the stream is flushed each time something is logged.
 */
class FlushingOstreamAppender final: public log4cpp::OstreamAppender {

public: /* Methods: */

    FlushingOstreamAppender(const std::string& name, std::ostream* stream)
        : OstreamAppender(name, stream)
    {}

protected: /* Methods: */

    void _append(const log4cpp::LoggingEvent& event) final override {
        (*_stream) << _getLayout().format(event);
        (*_stream).flush();
        if (!_stream->good()) {
            /// \todo XXX help! help!
        }
    }

};

Logger::Logger(const std::string& name)
    : m_logger(log4cpp::Category::getInstance(name))
{
#if defined(SHAREMIND_LOGLEVEL_FULLDEBUG) || defined(SHAREMIND_LOGLEVEL_DEBUG)
    m_logger.setPriority(log4cpp::Priority::DEBUG);
#elif defined(SHAREMIND_LOGLEVEL_NORMAL)
    m_logger.setPriority(log4cpp::Priority::INFO);
#elif defined(SHAREMIND_LOGLEVEL_WARNING)
    m_logger.setPriority(log4cpp::Priority::WARN);
#elif defined(SHAREMIND_LOGLEVEL_ERROR)
    m_logger.setPriority(log4cpp::Priority::ERROR);
#else
    m_logger.setPriority(log4cpp::Priority::DEBUG);
#endif
}

Logger::~Logger() noexcept {
    removeAllAppenders();
}

bool Logger::addFileAppender(const std::string& appenderName,
                             const std::string& filename,
                             bool append)
{
    removeAppender (appenderName);

    int fd;
    if (!openFile(*this, filename, append, fd))
        return false;

    try {
        log4cpp::Appender *appender = new log4cpp::FileAppender(appenderName, fd);
        try {
            log4cpp::Layout * layout = new LogLayout(*this);
            try {
                appender->setLayout(layout);
                m_logger.addAppender(appender);
            } catch (...) {
                delete layout;
                throw;
            }
        } catch (...) {
            delete appender;
            throw;
        }
    } catch (...) {
        return false;
    }

    return true;
}

bool Logger::addRollingFileAppender(const std::string& name,
                                    const std::string& filename,
                                    bool append,
                                    const size_t& maxFileSize,
                                    const unsigned int& maxBackupFiles)
{
    removeAppender (name);

    try {
        log4cpp::Appender *appender = new log4cpp::RollingFileAppender(name,
                                                                       filename,
                                                                       maxFileSize,
                                                                       maxBackupFiles,
                                                                       append);
        try {
            log4cpp::Layout * layout = new LogLayout(*this);
            try {
                appender->setLayout(layout);

                /**
                  \todo somehow need to check if the file was successfully opened.
                        Use reopen() function of the FileAppender for that?
                */

                m_logger.addAppender(appender);
            } catch (...) {
                delete layout;
                throw;
            }
        } catch (...) {
            delete appender;
            throw;
        }
    } catch (...) {
        return false;
    }

    return true;
}

bool Logger::addOutputStreamAppender(const std::string& name, std::ostream& stream) {
    removeAppender (name);

    try {
        log4cpp::Appender *appender = new FlushingOstreamAppender("OstreamAppender", &stream);
        try {
            log4cpp::Layout * layout = new LogLayout(*this);
            try {
                appender->setLayout(layout);
                m_logger.addAppender(appender);
            } catch (...) {
                delete layout;
                throw;
            }
        } catch (...) {
            delete appender;
            throw;
        }
    } catch (...) {
        return false;
    }

    return true;
}

bool Logger::addCustomAppender (const std::string &name, MessageProcessor &processor) {
    removeAppender (name);

    try {
        log4cpp::Appender *appender = new GenericAppender(name, &processor);
        try {
            log4cpp::Layout * layout = new LogLayout(*this);
            try {
                appender->setLayout(layout);
                m_logger.addAppender(appender);
            } catch (...) {
                delete layout;
                throw;
            }
        } catch (...) {
            delete appender;
            throw;
        }
    } catch (...) {
        return false;
    }

    return true;
}

void Logger::removeAppender(const std::string & appenderName) noexcept {
    /// \bug Might throw:
    m_logger.removeAppender(m_logger.getAppender(appenderName));
}

void Logger::removeAllAppenders() noexcept {
    m_logger.removeAllAppenders(); /// \bug Might throw
}

void Logger::logMessage(LogPriority priority, const char * message) noexcept {
    /// \bug Might throw:
    m_logger.getStream(prioToLog4cppPrio(priority)) << message;
}

void Logger::logMessage(LogPriority priority, const std::string & message) noexcept {
    /// \bug Might throw:
    m_logger.getStream(prioToLog4cppPrio(priority)) << message;
}

void Logger::logMessage(LogPriority priority, std::string && message) noexcept {
    /// \bug Might throw:
    m_logger.getStream(prioToLog4cppPrio(priority)) << std::move(message);
}

void Logger::logMessage(LogPriority priority, const SmartStringStream & message) noexcept {
    /// \bug Might throw:
    m_logger.getStream(prioToLog4cppPrio(priority)) << message;
}

std::string Logger::formatDate(time_t timestamp, bool reverse) {
    std::ostringstream date;

    const tm* dest = localtime (&timestamp);
    if (reverse) {
        date <<
            (dest->tm_year + 1900) << "-" <<
            (dest->tm_mon < 9 ? "0" : "") << (dest->tm_mon + 1) << "-" <<
            (dest->tm_mday < 10 ? "0" : "") << dest->tm_mday;
    } else {
        date <<
            (dest->tm_mday < 10 ? "0" : "") << dest->tm_mday << "-" <<
            (dest->tm_mon < 9 ? "0" : "") << (dest->tm_mon + 1) << "-" <<
            (dest->tm_year + 1900);
    }

    return date.str ();
}

std::string Logger::formatTime(time_t timestamp) {
    std::ostringstream time;

    const tm* dest = localtime (&timestamp);
    time <<
    (dest->tm_hour < 10 ? "0" : "") << dest->tm_hour << "-" <<
    (dest->tm_min < 10 ? "0" : "") << dest->tm_min << "-" <<
    (dest->tm_sec < 10 ? "0" : "") << dest->tm_sec;

    return time.str ();
}

} /* namespace sharemind */
