/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

// Date and time manipulation
#include <ctime>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/RollingFileAppender.hh>
#include <log4cpp/OstreamAppender.hh>

#include "Logger.h"
#include "LogLayout.h"

using std::ostringstream;
using std::string;
using namespace sharemind;

Logger::Logger(const std::string& name) :
    m_logger (log4cpp::Category::getInstance(name))
{
#if defined SHAREMIND_LOGLEVEL_FULLDEBUG
    m_logger.setPriority(LOGPRIORITY_FULLDEBUG);
#elif defined SHAREMIND_LOGLEVEL_DEBUG
    m_logger.setPriority(LOGPRIORITY_DEBUG);
#elif defined SHAREMIND_LOGLEVEL_NORMAL
    m_logger.setPriority(LOGPRIORITY_NORMAL);
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
    log4cpp::Layout *layout = new LogLayout(this);
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
    log4cpp::Layout *layout = new LogLayout(this);
    appender->setLayout(layout);

    // \todo somehow need to check if the file was successfully opened.
    // Use reopen() function of the FileAppender for that?

    m_logger.addAppender(appender);

    return true;
}

void Logger::addOutputStreamAppender(const std::string& appenderName, std::ostream& stream) {
    removeAppender (appenderName);

    log4cpp::Appender *appender = new log4cpp::OstreamAppender("OstreamAppender", &stream);
    log4cpp::Layout *layout = new LogLayout(this);
    appender->setLayout(layout);

    m_logger.addAppender(appender);
}

void Logger::addAppender (log4cpp::Appender& appender) {
    //first remove it, then readd
    removeAppender(appender);

    log4cpp::Layout *layout = new LogLayout(this);
    appender.setLayout(layout);

    m_logger.addAppender(appender);
}


void Logger::logToStream(log4cpp::Priority::Value priority, const std::string &message) {
    m_logger.getStream(priority) << message;
}

string Logger::formatDate(time_t timestamp, bool reverse) {
    ostringstream date;

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

string Logger::formatTime(time_t timestamp) {
    ostringstream time;

    const tm* dest = localtime (&timestamp);
    time <<
    (dest->tm_hour < 10 ? "0" : "") << dest->tm_hour << "-" <<
    (dest->tm_min < 10 ? "0" : "") << dest->tm_min << "-" <<
    (dest->tm_sec < 10 ? "0" : "") << dest->tm_sec;

    return time.str ();
}

bool Logger::openFile(const string& filename, const bool& append, int& fd) {
    // Check if we have a filename
    if (filename.length () > 0) {
        // Try to open the log file

        int flags = O_CREAT | O_APPEND | O_WRONLY;
        if (!append)
            flags |= O_TRUNC;

        fd = ::open(filename.c_str(), flags, 00644);
        if (fd < 0) {
            WRITE_LOG_ERROR (*this, "[Logger] Can't open logger log file " << filename << "!");
            return false;
        }

        WRITE_LOG_DEBUG (*this, "[Logger] Opened logger log file " << filename << ".");

        return true;

    } else {

        // We didn't get a filename so spread the information about that.
        WRITE_LOG_ERROR (*this, "[Logger] Empty log file name!");
        return false;
    }
}

