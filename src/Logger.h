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
#include <string>
#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>


enum SharemindLogPriority {
    LOGPRIORITY_FATAL,
    LOGPRIORITY_ERROR,
    LOGPRIORITY_WARNING,
    LOGPRIORITY_NORMAL,
    LOGPRIORITY_DEBUG,
    LOGPRIORITY_FULLDEBUG
};

namespace sharemind {

typedef SharemindLogPriority LogPriority;

class ILogger {

public: /* Methods: */

    /**
     Logs a message with the specified priority.

     \param[in] priority the priority level.
     \param[in] message the message to log.
    */
    virtual void logMessage(LogPriority priority, const char * message) = 0;

    /**
     Logs a message with the specified priority.

     \param[in] priority the priority level.
     \param[in] message the message to log.
    */
    virtual void logMessage(LogPriority priority, const std::string & message) = 0;

};

#define WRITE_LOG(logger, priority, prefix, message) \
    do { \
        std::ostringstream oss; \
        oss << prefix << message; \
        (logger).logMessage((priority),oss.str()); \
    } while (false)

#ifdef SHAREMIND_LOGLEVEL_FULLDEBUG
#define WRITE_LOG_FULLDEBUG(logger, message) WRITE_LOG(logger, LOGPRIORITY_FULLDEBUG, "", message)
#define WRITE_LOG_FULLDEBUG_MINER(message) WRITE_LOG(m_platform.getLogger(), LOGPRIORITY_FULLDEBUG, "", message)
#define WRITE_LOG_FULLDEBUG_SESSION(message) WRITE_LOG(getPlatform().getLogger(), LOGPRIORITY_FULLDEBUG, "[S" << m_sessionNumber << "] ", message)
#define WRITE_LOG_FULLDEBUG_VM(message) WRITE_LOG(m_session->getPlatform().getLogger(), LOGPRIORITY_FULLDEBUG, "[S" << m_session->getSessionNumber () << "-VM] ", message)
/* #define WRITE_LOG_FULLDEBUG_PROTOCOL(message) WRITE_LOG(m_vm->getControllerSession()->getPlatform().getLogger(), LOGPRIORITY_FULLDEBUG, "[S" << m_vm->getControllerSession()->getSessionNumber () << "-" << m_protocolName << "] ", message) */
#define WRITE_LOG_FULLDEBUG_CONTROLLER(message) WRITE_LOG(m_logger, LOGPRIORITY_FULLDEBUG, "[Controller] ", message)
#define WRITE_LOG_FULLDEBUG_NET(message) WRITE_LOG(m_logger, LOGPRIORITY_FULLDEBUG, "[NET] ", message)
#define WRITE_LOG_FULLDEBUG_DB(message) WRITE_LOG(m_logger, LOGPRIORITY_FULLDEBUG, "[DB] ", message)
#define SHAREMIND_LOGLEVEL_DEBUG
#else
#define WRITE_LOG_FULLDEBUG(logger, message)
#define WRITE_LOG_FULLDEBUG_MINER(message)
#define WRITE_LOG_FULLDEBUG_SESSION(message)
#define WRITE_LOG_FULLDEBUG_VM(message)
/* #define WRITE_LOG_FULLDEBUG_PROTOCOL(message) */
#define WRITE_LOG_FULLDEBUG_CONTROLLER(message)
#define WRITE_LOG_FULLDEBUG_NET(message)
#define WRITE_LOG_FULLDEBUG_DB(message)
#endif

#ifdef SHAREMIND_LOGLEVEL_DEBUG
#define WRITE_LOG_DEBUG(logger, message) WRITE_LOG(logger, LOGPRIORITY_DEBUG, "", message)
#define WRITE_LOG_DEBUG_MINER(message) WRITE_LOG(m_platform.getLogger(), LOGPRIORITY_DEBUG, "", message)
#define WRITE_LOG_DEBUG_SESSION(message) WRITE_LOG(getPlatform().getLogger(), LOGPRIORITY_DEBUG, "[S" << m_sessionNumber << "] ", message)
#define WRITE_LOG_DEBUG_VM(message) WRITE_LOG(m_session->getPlatform().getLogger(), LOGPRIORITY_DEBUG, "[S" << m_session->getSessionNumber () << "-VM] ", message)
/* #define WRITE_LOG_DEBUG_PROTOCOL(message) WRITE_LOG(m_vm->getControllerSession()->getPlatform().getLogger(), LOGPRIORITY_DEBUG, "[S" << m_vm->getControllerSession()->getSessionNumber () << "-" << m_protocolName << "] ", message) */
#define WRITE_LOG_DEBUG_CONTROLLER(message) WRITE_LOG(m_logger, LOGPRIORITY_DEBUG, "[Controller] ", message)
#define WRITE_LOG_DEBUG_NET(message) WRITE_LOG(m_logger, LOGPRIORITY_DEBUG, "[NET] ", message)
#define WRITE_LOG_DEBUG_DB(message) WRITE_LOG(m_logger, LOGPRIORITY_DEBUG, "[DB] ", message)
#define SHAREMIND_LOGLEVEL_NORMAL
#else
#define WRITE_LOG_DEBUG(logger, message)
#define WRITE_LOG_DEBUG_MINER(message)
#define WRITE_LOG_DEBUG_SESSION(message)
#define WRITE_LOG_DEBUG_VM(message)
/* #define WRITE_LOG_DEBUG_PROTOCOL(message) */
#define WRITE_LOG_DEBUG_CONTROLLER(message)
#define WRITE_LOG_DEBUG_NET(message)
#define WRITE_LOG_DEBUG_DB(message)
#endif

#ifdef SHAREMIND_LOGLEVEL_NORMAL
#define WRITE_LOG_NORMAL(logger, message) WRITE_LOG(logger, LOGPRIORITY_NORMAL, "", message)
#define WRITE_LOG_NORMAL_MINER(message) WRITE_LOG(m_platform.getLogger(), LOGPRIORITY_NORMAL, "", message)
#define WRITE_LOG_NORMAL_SESSION(message) WRITE_LOG(getPlatform().getLogger(), LOGPRIORITY_NORMAL, "[S" << m_sessionNumber << "] ", message)
#define WRITE_LOG_NORMAL_VM(message) WRITE_LOG(m_session->getPlatform().getLogger(), LOGPRIORITY_NORMAL, "[S" << m_session->getSessionNumber () << "-VM] ", message)
/* #define WRITE_LOG_NORMAL_PROTOCOL(message) WRITE_LOG(m_vm->getControllerSession()->getPlatform().getLogger(), LOGPRIORITY_NORMAL, "[S" << m_vm->getControllerSession()->getSessionNumber () << "-" << m_protocolName << "] ", message) */
#define WRITE_LOG_NORMAL_CONTROLLER(message) WRITE_LOG(m_logger, LOGPRIORITY_NORMAL, "[Controller] ", message)
#define WRITE_LOG_NORMAL_NET(message) WRITE_LOG(m_logger, LOGPRIORITY_NORMAL, "[NET] ", message)
#define WRITE_LOG_NORMAL_DB(message) WRITE_LOG(m_logger, LOGPRIORITY_NORMAL, "[DB] ", message)
#define SHAREMIND_LOGLEVEL_WARNING
#else
#define WRITE_LOG_NORMAL(logger, message)
#define WRITE_LOG_NORMAL_MINER(message)
#define WRITE_LOG_NORMAL_SESSION(message)
#define WRITE_LOG_NORMAL_VM(message)
/* #define WRITE_LOG_NORMAL_PROTOCOL(message) */
#define WRITE_LOG_NORMAL_CONTROLLER(message)
#define WRITE_LOG_NORMAL_NET(message)
#define WRITE_LOG_NORMAL_DB(message)
#endif

#ifdef SHAREMIND_LOGLEVEL_WARNING
#define WRITE_LOG_WARNING(logger, message) WRITE_LOG(logger, LOGPRIORITY_WARNING, "", message)
#define WRITE_LOG_WARNING_MINER(message) WRITE_LOG(m_platform.getLogger(), LOGPRIORITY_WARNING, "", message)
#define WRITE_LOG_WARNING_SESSION(message) WRITE_LOG(getPlatform().getLogger(), LOGPRIORITY_WARNING, "[S" << m_sessionNumber << "] ", message)
#define WRITE_LOG_WARNING_VM(message) WRITE_LOG(m_session->getPlatform().getLogger(), LOGPRIORITY_WARNING, "[S" << m_session->getSessionNumber () << "-VM] ", message)
/* #define WRITE_LOG_WARNING_PROTOCOL(message) WRITE_LOG(m_vm->getControllerSession()->getPlatform().getLogger(), LOGPRIORITY_WARNING, "[S" << m_vm->getControllerSession()->getSessionNumber () << "-" << m_protocolName << "] ", message) */
#define WRITE_LOG_WARNING_CONTROLLER(message) WRITE_LOG(m_logger, LOGPRIORITY_WARNING, "[Controller] ", message)
#define WRITE_LOG_WARNING_NET(message) WRITE_LOG(m_logger, LOGPRIORITY_WARNING, "[NET] ", message)
#define WRITE_LOG_WARNING_DB(message) WRITE_LOG(m_logger, LOGPRIORITY_WARNING, "[DB] ", message)
#else
#define WRITE_LOG_WARNING(logger, message)
#define WRITE_LOG_WARNING_MINER(message)
#define WRITE_LOG_WARNING_SESSION(message)
#define WRITE_LOG_WARNING_VM(message)
/* #define WRITE_LOG_WARNING_PROTOCOL(message) */
#define WRITE_LOG_WARNING_CONTROLLER(message)
#define WRITE_LOG_WARNING_NET(message)
#define WRITE_LOG_WARNING_DB(message)
#endif

#define WRITE_LOG_ERROR(logger, message) WRITE_LOG(logger, LOGPRIORITY_ERROR, "", message)
#define WRITE_LOG_ERROR_MINER(message) WRITE_LOG(m_platform.getLogger(), LOGPRIORITY_ERROR, "", message)
#define WRITE_LOG_ERROR_SESSION(message) WRITE_LOG(getPlatform().getLogger(), LOGPRIORITY_ERROR, "[S" << m_sessionNumber << "] ", message)
#define WRITE_LOG_ERROR_VM(message) WRITE_LOG(m_session->getPlatform().getLogger(), LOGPRIORITY_ERROR, "[S" << m_session->getSessionNumber () << "-VM] ", message)
/* #define WRITE_LOG_ERROR_PROTOCOL(message) WRITE_LOG(m_vm->getControllerSession()->getPlatform().getLogger(), LOGPRIORITY_ERROR, "[S" << m_vm->getControllerSession()->getSessionNumber () << "-" << m_protocolName << "] ", message) */
#define WRITE_LOG_ERROR_CONTROLLER(message) WRITE_LOG(m_logger, LOGPRIORITY_ERROR, "[Controller] ", message)
#define WRITE_LOG_ERROR_NET(message) WRITE_LOG(m_logger, LOGPRIORITY_ERROR, "[NET] ", message)
#define WRITE_LOG_ERROR_DB(message) WRITE_LOG(m_logger, LOGPRIORITY_ERROR, "[DB] ", message)

#define WRITE_LOG_FATAL(logger, message) WRITE_LOG(logger, LOGPRIORITY_FATAL, "", message)
#define WRITE_LOG_FATAL_MINER(message) WRITE_LOG(m_platform.getLogger(), LOGPRIORITY_FATAL, "", message)
#define WRITE_LOG_FATAL_SESSION(message) WRITE_LOG(getPlatform().getLogger(), LOGPRIORITY_FATAL, "[S" << m_sessionNumber << "] ", message)
#define WRITE_LOG_FATAL_VM(message) WRITE_LOG(m_session->getPlatform().getLogger(), LOGPRIORITY_FATAL, "[S" << m_session->getSessionNumber () << "-VM] ", message)
/* #define WRITE_LOG_FATAL_PROTOCOL(message) WRITE_LOG(m_vm->getControllerSession()->getPlatform().getLogger(), LOGPRIORITY_FATAL, "[S" << m_vm->getControllerSession()->getSessionNumber () << "-" << m_protocolName << "] ", message) */
#define WRITE_LOG_FATAL_CONTROLLER(message) WRITE_LOG(m_logger, LOGPRIORITY_FATAL, "[Controller] ", message)
#define WRITE_LOG_FATAL_NET(message) WRITE_LOG(m_logger, LOGPRIORITY_FATAL, "[NET] ", message)
#define WRITE_LOG_FATAL_DB(message) WRITE_LOG(m_logger, LOGPRIORITY_FATAL, "[DB] ", message)


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

    void setMessagePrefix (std::string prefix) {
        m_prefix = prefix;
    }

    std::string getMessagePrefix () {
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
    bool openFile(const std::string& filename, const bool& append, int& fd);

}; /* class Logger { */

} /* namespace sharemind { */

#endif /* SHAREMINDCOMMON_LOGGER_H */
