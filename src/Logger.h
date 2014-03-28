/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SHAREMINDCOMMON_LOGGER_H
#define SHAREMINDCOMMON_LOGGER_H

#include <ctime>
#include <sstream>
#include "ILogger.h"

namespace log4cpp {
class Category;
}

namespace sharemind {

class MessageProcessor;

/**
 This class provides logging services for the whole project.

 It currently supports five different levels of logging,
 described in the LogLevels enumeration. When sending messages
 to the log, the user selects, which level of logging it
 belongs to (it is not very smart to use LOG_NONE).

 If the message is LOG_DEBUG, but the logging level is set to
 LOG_MINIMAL, the message will not be displayed/written to the log.
*/
class Logger final: public ILogger {

public:

    /**
     Initialises the Logger class.

     Creates the row buffer.
    */
    Logger(const std::string& name);

    ~Logger() noexcept;

    /**
     Adds a file appender to the Logger.

     \param[in] appenderName The unique name of the created appender.
     \param[in] filename The name of the output log file.
     \param[in] append Indicates whether the logs will be appended to the end of the file or not.
     \returns Whether or not adding the file appender succeeded.
    */
    bool addFileAppender(const std::string& appenderName,
                         const std::string& filename,
                         bool append);

    /**
     Adds a rolling file appender to the Logger.

     \param[in] appenderName The unique name of the created appender.
     \param[in] filename The name for the output log file.
     \param[in] append Indicates whether the logs will be appended to the end of the file or not.
     \param[in] maxFileSize The maximum size of a single log file.
     \param[in] maxBackupFiles The maximum number of backup log files to create before rollover occurs.
     \returns Whether or not adding the rolling file appender succeeded.
    */
    bool addRollingFileAppender(const std::string& appenderName,
                                const std::string& filename,
                                bool append,
                                const size_t& maxFileSize,
                                const unsigned int& maxBackupFiles);

    /**
     Adds an output stream appender to the Logger.

     \param[in] name The unique name for the created appender.
     \param[in] stream The output stream, where the logs will be sent.
     \returns Whether or not adding the stream appender succeeded.
    */
    bool addOutputStreamAppender(const std::string& name, std::ostream& stream);

    /**
     Adds a custom MessageProcessor based appender to the Logger.

     \param[in] name The unique name of the created appender.
     \param[in] processor The processor of the logged messages.
     \returns Whether or not adding the custom appender succeeded.
    */
    bool addCustomAppender (const std::string &name, MessageProcessor &processor);

    /**
     Removes the specified appender from the Logger by name.

     \param[in] appenderName the name of the appender to be removed.
    */
    void removeAppender(const std::string & appenderName);

    /**
     Removes all the appenders and closes all opened log files
     */
    void removeAllAppenders() noexcept;

    /* Inherited from ILogger: */
    void logMessage(LogPriority priority,
                    const char * message) noexcept final override;
    void logMessage(LogPriority priority,
                    const std::string & message) noexcept final override;
    void logMessage(LogPriority priority,
                    std::string && message) noexcept final override;
    void logMessage(LogPriority priority,
                    const SmartStringStream & message) noexcept final override;

    /**
     Returns a formatted date for the given timestamp.

     \param[in] timestamp the C timestamp to format
     \returns a string which contains the date in the timestamp
     */
    static std::string formatDate(time_t timestamp, bool reverse = false);

    /**
     Returns a formatted time for the given timestamp.

     \param[in] timestamp the C timestamp to format
     \returns a string which contains the time of day in the timestamp
     */
    static std::string formatTime(time_t timestamp);

    void setMessagePrefix(const std::string & prefix) {
        m_prefix = prefix;
    }

    const std::string & getMessagePrefix() const {
        return m_prefix;
    }

private:

    /**
     The main logger class of the log4cpp library. It does the actual logging.
     */
    log4cpp::Category& m_logger;

    std::string m_prefix;

}; /* class Logger { */

} /* namespace sharemind { */

#endif /* SHAREMINDCOMMON_LOGGER_H */
