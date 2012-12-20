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

// Date and time manipulation
#include <ctime>
#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/RollingFileAppender.hh>

#include "../SmartStringStream.h"
#include "Debug.h"
#include "LogLayout.h"


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
    }
}

} // anonymous namespace

namespace sharemind {

/**
 * This class reimplements the OstreamAppender so that the stream is flushed each time something is logged.
 */
class FlushingOstreamAppender: public log4cpp::OstreamAppender {
public:
    FlushingOstreamAppender(const std::string& name, std::ostream* stream)
        : OstreamAppender(name, stream)
    {}

protected:
    virtual void _append(const log4cpp::LoggingEvent& event) {
        (*_stream) << _getLayout().format(event);
        (*_stream).flush();
        if (!_stream->good()) {
            // XXX help! help!
        }
    }
};

Logger::Logger(const std::string& name) :
    m_logger (log4cpp::Category::getInstance(name))
{
#if defined SHAREMIND_LOGLEVEL_FULLDEBUG
    m_logger.setPriority(log4cpp::Priority::DEBUG);
#elif defined SHAREMIND_LOGLEVEL_DEBUG
    m_logger.setPriority(log4cpp::Priority::DEBUG);
#elif defined SHAREMIND_LOGLEVEL_NORMAL
    m_logger.setPriority(log4cpp::Priority::INFO);
#else
    m_logger.setPriority(log4cpp::Priority::NOTICE);
#endif

}

Logger::~Logger() {
    removeAllAppenders();
}

bool Logger::addFileAppender(const std::string& appenderName, const std::string& filename, bool append) {
    removeAppender (appenderName);

    int fd;
    if (!openFile(filename, append, fd))
        return false;

    log4cpp::Appender *appender = new log4cpp::FileAppender(appenderName, fd);
    log4cpp::Layout * layout = new LogLayout(*this);
    appender->setLayout(layout);

    m_logger.addAppender(appender);

    return true;
}

bool Logger::addRollingFileAppender(const std::string& appenderName,
                                    const std::string& filename,
                                    bool append,
                                    const size_t& maxFileSize,
                                    const unsigned int& maxBackupFiles) {
    removeAppender (appenderName);

    log4cpp::Appender *appender = new log4cpp::RollingFileAppender(appenderName,
                                                                   filename,
                                                                   maxFileSize,
                                                                   maxBackupFiles,
                                                                   append);
    log4cpp::Layout * layout = new LogLayout(*this);
    appender->setLayout(layout);

    /**
      \todo somehow need to check if the file was successfully opened. Use
            reopen() function of the FileAppender for that?
    */

    m_logger.addAppender(appender);

    return true;
}

void Logger::addOutputStreamAppender(const std::string& appenderName, std::ostream& stream) {
    removeAppender (appenderName);

    log4cpp::Appender *appender = new FlushingOstreamAppender("OstreamAppender", &stream);
    log4cpp::Layout * layout = new LogLayout(*this);
    appender->setLayout(layout);

    m_logger.addAppender(appender);
}

void Logger::addAppender (log4cpp::Appender& appender) {
    // Take care we do not have the same appender added twice:
    removeAppender(appender);

    log4cpp::Layout * layout = new LogLayout(*this);
    appender.setLayout(layout);

    m_logger.addAppender(appender);
}

void Logger::removeAppender(log4cpp::Appender & appender) {
    m_logger.removeAppender(&appender);
}

void Logger::removeAppender(const std::string & appenderName) {
    m_logger.removeAppender(m_logger.getAppender(appenderName));
}

void Logger::removeAllAppenders() {
    m_logger.removeAllAppenders();
}

void Logger::logMessage(LogPriority priority, const char * message) {
    m_logger.getStream(prioToLog4cppPrio(priority)) << message;
}

void Logger::logMessage(LogPriority priority, const std::string & message) {
    m_logger.getStream(prioToLog4cppPrio(priority)) << message;
}

void Logger::logMessage(LogPriority priority, const SmartStringStream & message) {
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

bool Logger::openFile(const std::string & filename, bool append, int & fd) {
    // Check if we have a filename
    if (filename.length () > 0) {
        // Try to open the log file

        int flags = O_CREAT | O_APPEND | O_WRONLY;
        if (!append)
            flags |= O_TRUNC;

        fd = ::open(filename.c_str(), flags, 00644);
        if (fd < 0) {
            LogError(*this) << "Can't open logger log file " << filename << "!";
            return false;
        }

        LogDebug(*this) << "Opened logger log file " << filename << ".";

        return true;

    } else {

        // We didn't get a filename so spread the information about that.
        LogError(*this) << "Empty log file name!";
        return false;
    }
}

} // namespace sharemind {
