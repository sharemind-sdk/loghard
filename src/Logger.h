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
#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include "ILogger.h"


namespace sharemind {

/**
 This class provides logging services for the whole project.

 It currently supports five different levels of logging,
 described in the LogLevels enumeration. When sending messages
 to the log, the user selects, which level of logging it
 belongs to (it is not very smart to use LOG_NONE).

 If the message is LOG_DEBUG, but the logging level is set to
 LOG_MINIMAL, the message will not be displayed/written to the log.
*/
class Logger: public ILogger {

public:

    /**
     Initialises the Logger class.

     Creates the row buffer.
    */
    Logger(const std::string& name);

    virtual ~Logger();

    /**
     Adds a file appender to the Logger.

     \param[in] appenderName the name for the created appender.
     \param[in] filename the name for the log file.
     \param[in] append indicates whether the logs will be appended to the end of the file or not.

     \retval true if opening the file was successful
     \retval false if opening the file failed
    */
    bool addFileAppender(const std::string& appenderName, const std::string& filename, bool append);

    /**
     Adds a rolling file appender to the Logger.

     \param[in] appenderName the name for the created appender.
     \param[in] filename the name for the log file.
     \param[in] append indicates whether the logs will be appended to the end of the file or not.
     \param[in] maxFileSize the maximum size of a single log file.
     \param[in] maxBackupFiles the maximum number of backup log files to create before rollover occurs.

     \retval true if opening the file was successful
     \retval false if opening the file failed (currently no check is done, so false never returned)
    */
    bool addRollingFileAppender(const std::string& appenderName, const std::string& filename, bool append, const size_t& maxFileSize, const unsigned int& maxBackupFiles);

    /**
     Adds a output stream appender to the Logger.

     \param[in] appenderName the name for the created appender.
     \param[in] stream the output stream, where the logs will be sent.
    */
    void addOutputStreamAppender(const std::string& appenderName, std::ostream& stream);

    /**
     Adds a preconstructed appender to the Logger.

     \param[in] appender the preconstructed appender.
    */
    void addAppender (log4cpp::Appender& appender);

    /**
     Removes the specified appender from the Logger by pointer.

     \param[in] appender the pointer to a preconstructed appender.
    */
    inline void removeAppender (log4cpp::Appender& appender)  {
        m_logger.removeAppender(&appender);
    }

    /**
     Removes the specified appender from the Logger by name.

     \param[in] appenderName the name of the appender to be removed.
    */
    inline void removeAppender (const std::string& appenderName) {
        m_logger.removeAppender(m_logger.getAppender(appenderName));
    }

    /**
     Removes all the appenders and closes all opened log files
     */
    inline void removeAllAppenders() {
        m_logger.removeAllAppenders();
    }

    /**
     Returns a logging stream according to a given priority value.

     \param[in] priority the priority level from the log4cpp::Priority enumeration.
     \returns a logging stream according to a given priority value.
    */
    inline log4cpp::CategoryStream getStream(log4cpp::Priority::Value priority) {
        return m_logger.getStream(priority);
    }

    /* Inherited from ILogger: */
    virtual void logMessage(LogPriority priority, const char * message);
    virtual void logMessage(LogPriority priority, const std::string & message);
    virtual void logMessage(LogPriority priority, const SmartStringStream & message);

    /*inline boost::mutex& getStreamMutex() {
        return *m_streamMutex;
    }*/

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

    /* boost::mutex *m_streamMutex; */

    /**
     The main logger class of the log4cpp library. It does the actual logging.
     */
    log4cpp::Category& m_logger;

    std::string m_prefix;

    /**
     Opens a file for writing. Used by this class for opening files for the Appenders.

     \param[in] filename the name for the log file.
     \param[in] append indicates whether the logs will be appended to the end of the file or not.
     \param[out] fd the file descriptor for the log file.

     \retval true if opening the file was successful
     \retval false if opening the file failed
    */
    bool openFile(const std::string & filename, bool append, int & fd);

}; /* class Logger { */

} /* namespace sharemind { */

#endif /* SHAREMINDCOMMON_LOGGER_H */
