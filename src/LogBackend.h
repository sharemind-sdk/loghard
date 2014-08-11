/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SHAREMINDCOMMON_LOGBACKEND_H
#define SHAREMINDCOMMON_LOGBACKEND_H

#include <cassert>
#include <fluffy/Exception.h>
#include <fluffy/QueueingMutex.h>
#include <fluffy/QueueingRwMutex.h>
#include <set>
#include <sys/time.h>
#include <syslog.h>
#include <system_error>
#include <utility>
#include "../Abort.h"
#include "LogPriority.h"


namespace sharemind {

class Logger;

class LogBackend {

    friend class Logger;

public: /* Types: */

    class Appender;
    typedef std::set<Appender *> Appenders;

    class Appender {

    public: /* Methods: */

        virtual ~Appender() noexcept {}

        virtual void activate(const Appenders & appenders) { (void) appenders; }

        virtual void log(timeval time,
                         LogPriority priority,
                         const char * message) noexcept = 0;

        inline static const char * priorityString(const LogPriority priority)
                noexcept
        {
            switch (priority) {
                case LogPriority::Fatal:     return "FATAL";
                case LogPriority::Error:     return "ERROR";
                case LogPriority::Warning:   return "WARNING";
                case LogPriority::Normal:    return "INFO";
                case LogPriority::Debug:     return "DEBUG";
                case LogPriority::FullDebug: return "DEBUG2";
                default:
                    SHAREMIND_ABORT("LLpS: p=%d", static_cast<int>(priority));
            }
        }

        inline static const char * priorityStringRightPadded(
                const LogPriority priority) noexcept
        {
            switch (priority) {
                case LogPriority::Fatal:     return "FATAL  ";
                case LogPriority::Error:     return "ERROR  ";
                case LogPriority::Warning:   return "WARNING";
                case LogPriority::Normal:    return "INFO   ";
                case LogPriority::Debug:     return "DEBUG  ";
                case LogPriority::FullDebug: return "DEBUG2 ";
                default:
                    SHAREMIND_ABORT("LLpSRP: p=%d", static_cast<int>(priority));
            }
        }

        static const char * timeStamp(char * const buffer,
                                      const size_t bufferSize,
                                      const char * const format,
                                      const timeval time,
                                      const char * const failStr) noexcept
        {
            tm eventTimeTm;
            if (!localtime_r(&time.tv_sec, &eventTimeTm))
                return failStr;

            #ifndef NDEBUG
            const size_t r =
            #endif
                strftime(buffer, bufferSize, format, &eventTimeTm);
            assert(r);
            return buffer;
        }

    }; /* class Appender { */

    class SyslogAppender: public Appender {

    public: /* Types: */

        FLUFFY_DEFINE_EXCEPTION_SUBCLASS(std::exception, Exception);
        FLUFFY_DEFINE_EXCEPTION_SUBCLASS_CONST_MSG(
                Exception,
                MultipleSyslogAppenderException,
                "Multiple Syslog appenders not allowed!");

    public: /* Methods: */

        inline SyslogAppender(const std::string & ident,
                              const int logopt,
                              const int facility)
            : m_ident(ident)
            , m_logopt(logopt)
            , m_facility(facility) {}

        inline ~SyslogAppender() noexcept override { closelog(); }

        void activate(const Appenders & appenders) override {
            for (Appender * const a : appenders)
                if (a != this && dynamic_cast<SyslogAppender *>(a) != nullptr)
                    throw MultipleSyslogAppenderException();
            openlog(m_ident.c_str(), m_logopt, m_facility);
        }

        inline void log(timeval,
                        const LogPriority priority,
                        const char * message) noexcept override
        {
            int p;
            switch (priority) {
                case LogPriority::Fatal:     p = LOG_EMERG;   break;
                case LogPriority::Error:     p = LOG_ERR;     break;
                case LogPriority::Warning:   p = LOG_WARNING; break;
                case LogPriority::Normal:    p = LOG_INFO;    break;
                case LogPriority::Debug:     p = LOG_DEBUG;   break;
                case LogPriority::FullDebug: p = LOG_DEBUG;   break;
                default:
                    SHAREMIND_ABORT("SAsP: p=%d", static_cast<int>(priority));
            }
            syslog(p, "%s", message);
        }

        const std::string m_ident;
        const int m_logopt;
        const int m_facility;

    }; /* class SyslogAppender { */

    class CFileAppender: public Appender {

    public: /* Methods: */

        CFileAppender(FILE * const file) noexcept : m_file(file) {}

        inline void log(timeval time,
                        const LogPriority priority,
                        const char * message) noexcept override
        { logToFile(m_file, time, priority, message, m_mutex); }

        static inline void logToFile(FILE * file,
                                     timeval time,
                                     const LogPriority priority,
                                     const char * const message,
                                     Fluffy::QueueingMutex & mutex) noexcept
        {

            constexpr const size_t bufSize = sizeof("HH:MM:SS");
            char timeStampBuf[bufSize];
            const char * const timeStr = timeStamp(timeStampBuf,
                                                   bufSize,
                                                   "%H:%M:%S",
                                                   time,
                                                   "--:--:--");
            const char * const priorityStr =
                    priorityStringRightPadded(priority);
            const Fluffy::QueueingMutex::Guard guard(mutex);
            fprintf(file, "%s %s %s\n", timeStr, priorityStr, message);
            fflush(file);
            const int fd = fileno(file);
            if (fd != -1)
                fsync(fd);
        }

    private: /* Fields: */

        FILE * const m_file;
        mutable Fluffy::QueueingMutex m_mutex;

    }; /* class CFileAppender { */

    class StdAppender: public Appender {

    public: /* Types: */

        FLUFFY_DEFINE_EXCEPTION_SUBCLASS(std::exception, Exception);
        FLUFFY_DEFINE_EXCEPTION_SUBCLASS_CONST_MSG(
                Exception,
                MultipleStdAppenderException,
                "Multiple Std appenders not allowed!");

    public: /* Methods: */

        void activate(const Appenders & appenders) override {
            for (Appender * const a : appenders)
                if (a != this && dynamic_cast<StdAppender *>(a) != nullptr)
                    throw MultipleStdAppenderException();
        }

        inline void log(timeval time,
                        const LogPriority priority,
                        const char * message) noexcept override
        {
            if (priority <= LogPriority::Warning) {
                CFileAppender::logToFile(stderr, time, priority, message,
                                         m_stderrMutex);
            } else {
                CFileAppender::logToFile(stdout, time, priority, message,
                                         m_stdoutMutex);
            }
        }

    private: /* Fields: */

        Fluffy::QueueingMutex m_stderrMutex;
        Fluffy::QueueingMutex m_stdoutMutex;

    };

    class FileAppender: public Appender {

    public: /* Types: */

        enum OpenMode { APPEND, OVERWRITE };

    public: /* Methods: */

        template <typename Path>
        FileAppender(Path && path, const OpenMode openMode)
            : m_path(std::forward<Path>(path))
            , m_file(fopen(m_path.c_str(), openModeString(openMode)))
        {
            if (!m_file)
                throw std::system_error(errno, std::system_category());
        }

        inline ~FileAppender() noexcept override { fclose(m_file); }

        inline void log(timeval time,
                        const LogPriority priority,
                        const char * message) noexcept override
        { CFileAppender::logToFile(m_file, time, priority, message, m_mutex); }

        inline static const char * openModeString(const OpenMode openMode)
                     noexcept
        {
            assert(openMode == OVERWRITE || openMode == APPEND);
            return (openMode == OVERWRITE) ? "w+b" : "a+b";
        }

    private: /* Fields: */

        const std::string m_path;
        FILE * const m_file;
        mutable Fluffy::QueueingMutex m_mutex;

    }; /* class FileAppender */

public: /* Methods: */

    virtual ~LogBackend() noexcept {
        for (Appender * const appender: m_appenders)
            delete appender;
    }

    /**
      \brief Adds a SyslogAppender to the Logger.
      \param[in] args Arguments to the SyslogAppender constructor.
    */
    template <typename ... Args>
    inline SyslogAppender & addSyslogAppender(Args && ... args) {
        return addAppender__<SyslogAppender,
                             Args...>(std::forward<Args>(args)...);
    }

    /**
      \brief Adds a FileAppender to the Logger.
      \param[in] args Arguments to the FileAppender constructor.
    */
    template <typename ... Args>
    inline FileAppender & addFileAppender(Args && ... args) {
        return addAppender__<FileAppender,
                             Args...>(std::forward<Args>(args)...);
    }

    /**
      \brief Adds a CFileAppender to the Logger.
      \param[in] args Arguments to the CFileAppender constructor.
    */
    template <typename ... Args>
    inline CFileAppender & addCFileAppender(Args && ... args) {
        return addAppender__<CFileAppender,
                             Args...>(std::forward<Args>(args)...);
    }

    /** \brief Adds a StdAppender to the Logger. */
    inline StdAppender & addStdAppender()
    { return addAppender__<StdAppender>(); }

    inline void addAppender(Appender * const appender) {
        assert(appender);
        const Fluffy::QueueingRwMutex::UniqueGuard uniqueGuard(m_mutex);
        m_appenders.insert(appender);
        try {
            appender->activate(m_appenders);
        } catch (...) {
            m_appenders.erase(appender);
            throw;
        }
    }

private: /* Methods: */

    template <LogPriority priority>
    inline void doLog(const timeval time, const char * const message) {
        const Fluffy::QueueingRwMutex::SharedGuard sharedGuard(m_mutex);
        for (Appender * const appender : m_appenders)
            appender->log(time, priority, message);
    }

    template <typename AppenderType, typename ... Args>
    inline AppenderType & addAppender__(Args && ... args) {
        AppenderType * const appender =
                new AppenderType(std::forward<Args>(args)...);
        try {
            addAppender(appender);
            return *appender;
        } catch (...) {
            delete appender;
            throw;
        }
    }

private: /* Fields: */

    mutable Fluffy::QueueingRwMutex m_mutex;
    Appenders m_appenders;

}; /* class LogBackend { */

} /* namespace sharemind { */

#endif /* SHAREMINDCOMMON_LOGBACKEND_H */
