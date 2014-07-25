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

#include <cassert>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/Layout.hh>
#include <log4cpp/LayoutAppender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/RollingFileAppender.hh>
// #include <log4cpp/SimpleLayout.hh>
#include "../Abort.h"
#include "../Concat.h"


namespace {

/**
 * LogLayout is a fixed format log4cpp::Layout implementation for Sharemind.
 **/
class LogLayout: public log4cpp::Layout {

public: /* Methods: */

    /**
     * Formats the LoggingEvent in LogLayout style:<br>
     * "formattedTime priority ndc: message"
     **/
    std::string format(const log4cpp::LoggingEvent & event) final override {
        const time_t eventTime = event.timeStamp.getSeconds();
        constexpr const size_t bufSize = sizeof("HH:MM:SS ");
        char timeStampBuf[bufSize];
        tm eventTimeTm;
        const char * timeStampBufferToPrint;
        if (!localtime_r(&eventTime, &eventTimeTm)) {
            timeStampBufferToPrint = "--:--:-- ";
        } else {
            #ifndef NDEBUG
            const size_t r =
            #endif
                    strftime(timeStampBuf, bufSize, "%H:%M:%S ", &eventTimeTm);
            assert(r);
            timeStampBufferToPrint = timeStampBuf;
        }
        return sharemind::concat(
                    timeStampBufferToPrint,
                    std::setw(log4cpp::Priority::MESSAGE_SIZE),
                    std::setiosflags(std::ios::left),
                    log4cpp::Priority::getPriorityName(event.priority),
                    event.message,
                    sharemind::concat_endl);
    }

}; /* class LogLayout { */

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
    assert(!filename.empty());

    // Try to open the log file
    int flags = O_CREAT | O_APPEND | O_WRONLY;
    if (!append)
        flags |= O_TRUNC;

    fd = ::open(filename.c_str(), flags, 00644);
    if (fd < 0) {
        logger.error() << "Cannot open log file " << filename << "!";
        return false;
    }

    logger.debug() << "Opened log file " << filename << ".";
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
    m_logger.removeAllAppenders(); /// \bug Might throw
}

bool Logger::addFileAppender(const std::string & appenderName,
                             const std::string & filename,
                             bool append) noexcept
{
    try {
        int fd;
        if (!openFile(*this, filename, append, fd))
            return false;

        log4cpp::Appender * const appender = new log4cpp::FileAppender(appenderName, fd);
        try {
            LogLayout * const layout = new LogLayout;
            try {
                appender->setLayout(layout);
            } catch (...) {
                delete layout;
                throw;
            }
            std::lock_guard<std::mutex> guard(m_mutex);
            m_logger.addAppender(appender);
        } catch (...) {
            delete appender;
            throw;
        }
    } catch (...) {
        return false;
    }
    return true;
}

bool Logger::addRollingFileAppender(const std::string & name,
                                    const std::string & filename,
                                    bool append,
                                    const size_t maxFileSize,
                                    const unsigned int maxBackupFiles) noexcept
{
    try {
        log4cpp::Appender * const appender =
                new log4cpp::RollingFileAppender(name,
                                                 filename,
                                                 maxFileSize,
                                                 maxBackupFiles,
                                                 append);
        try {
            LogLayout * const layout = new LogLayout;
            try {
                appender->setLayout(layout);
            } catch (...) {
                delete layout;
                throw;
            }

            std::lock_guard<std::mutex> guard(m_mutex);
            /**
              \todo somehow need to check if the file was successfully
                    opened. Use reopen() function of the FileAppender for
                    that?
            */
            m_logger.addAppender(appender);
        } catch (...) {
            delete appender;
            throw;
        }
    } catch (...) {
        return false;
    }
    return true;
}

bool Logger::addOutputStreamAppender(const std::string & name,
                                     std::ostream & stream) noexcept
{
    try {
        log4cpp::Appender * const appender = new FlushingOstreamAppender(name, &stream);
        try {
            LogLayout * const layout = new LogLayout;
            try {
                appender->setLayout(layout);
            } catch (...) {
                delete layout;
                throw;
            }
            std::lock_guard<std::mutex> guard(m_mutex);
            m_logger.addAppender(appender);
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
    std::lock_guard<std::mutex> guard(m_mutex);
    /// \bug Might throw:
    m_logger.removeAppender(m_logger.getAppender(appenderName));
}

void Logger::removeAllAppenders() noexcept {
    std::lock_guard<std::mutex> guard(m_mutex);
    m_logger.removeAllAppenders(); /// \bug Might throw
}

void Logger::logMessage(LogPriority priority, const char * message) noexcept {
    std::lock_guard<std::mutex> guard(m_mutex);
    /// \bug Might throw:
    m_logger.getStream(prioToLog4cppPrio(priority)) << message;
}

void Logger::logMessage(LogPriority priority, const std::string & message) noexcept {
    std::lock_guard<std::mutex> guard(m_mutex);
    /// \bug Might throw:
    m_logger.getStream(prioToLog4cppPrio(priority)) << message;
}

void Logger::logMessage(LogPriority priority, std::string && message) noexcept {
    std::lock_guard<std::mutex> guard(m_mutex);
    /// \bug Might throw:
    m_logger.getStream(prioToLog4cppPrio(priority)) << std::move(message);
}

} /* namespace sharemind */
