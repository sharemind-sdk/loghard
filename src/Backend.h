/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef LOGHARD_BACKEND_H
#define LOGHARD_BACKEND_H

#include <cassert>
#include <cstdio>
#include <fcntl.h>
#include <fluffy/Exception.h>
#include <fluffy/QueueingMutex.h>
#include <fluffy/QueueingRwMutex.h>
#include <set>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <syslog.h>
#include <system_error>
#include <utility>
#include <unistd.h>
#include "Priority.h"


namespace LogHard {

class Logger;

class Backend {

    friend class Logger;

public: /* Types: */

    class Appender;
    typedef std::set<Appender *> Appenders;

    class Appender {

    public: /* Methods: */

        virtual ~Appender() noexcept {}

        virtual void activate(const Appenders & appenders) { (void) appenders; }

        virtual void log(timeval time,
                         Priority priority,
                         const char * message) noexcept = 0;

        inline static const char * priorityString(const Priority priority)
                noexcept
        {
            static const char strings[][8u] =
                    { "FATAL", "ERROR", "WARNING", "INFO", "DEBUG", "DEBUG2" };
            return &strings[static_cast<unsigned>(priority)][0u];
        }

        inline static const char * priorityStringRightPadded(
                const Priority priority) noexcept
        {
            static const char strings[][8u] = {
                "FATAL  ", "ERROR  ", "WARNING", "INFO   ", "DEBUG  ", "DEBUG2 "
            };
            return &strings[static_cast<unsigned>(priority)][0u];
        }

    }; /* class Appender { */

    class SyslogAppender: public Appender {

    public: /* Types: */

        FLUFFY_DEFINE_EXCEPTION(std::exception, Exception);
        FLUFFY_DEFINE_EXCEPTION_CONST_MSG(
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
                        const Priority priority,
                        const char * message) noexcept override
        {
            constexpr static const int priorities[] = {
                LOG_EMERG, LOG_ERR, LOG_WARNING, LOG_INFO, LOG_DEBUG, LOG_DEBUG
            };
            syslog(priorities[static_cast<unsigned>(priority)], "%s", message);
        }

        const std::string m_ident;
        const int m_logopt;
        const int m_facility;

    }; /* class SyslogAppender { */

    class CFileAppender: public Appender {

    public: /* Methods: */

        CFileAppender(FILE * const file) noexcept
            : m_fd(fileno(file))
        {
            if (m_fd == -1)
                throw std::system_error(errno, std::system_category());
        }

        inline void log(timeval time,
                        const Priority priority,
                        const char * message) noexcept override
        { logToFileSync(m_fd, time, priority, message, m_mutex); }

        static inline void logToFile(const int fd,
                                     timeval time,
                                     const Priority priority,
                                     const char * const message,
                                     Fluffy::QueueingMutex & mutex) noexcept
        { logToFile__(fd, time, priority, message, mutex, [](const int){}); }

        static inline void logToFileSync(const int fd,
                                         timeval time,
                                         const Priority priority,
                                         const char * const message,
                                         Fluffy::QueueingMutex & mutex) noexcept
        {
            logToFile__(fd, time, priority, message, mutex,
                        [](const int fd){ fsync(fd); });
        }

        static inline void logToFile(FILE * file,
                                     timeval time,
                                     const Priority priority,
                                     const char * const message,
                                     Fluffy::QueueingMutex & mutex) noexcept
        {
            const int fd = fileno(file);
            assert(fd != -1);
            logToFile(fd, time, priority, message, mutex);
        }

        static inline void logToFileSync(FILE * file,
                                         timeval time,
                                         const Priority priority,
                                         const char * const message,
                                         Fluffy::QueueingMutex & mutex) noexcept
        {
            const int fd = fileno(file);
            assert(fd != -1);
            logToFileSync(fd, time, priority, message, mutex);
        }

    private: /* Methods: */

        template <typename Sync>
        static inline void logToFile__(const int fd,
                                       timeval time,
                                       const Priority priority,
                                       const char * const message,
                                       Fluffy::QueueingMutex & mutex,
                                       Sync && sync) noexcept
        {
            assert(fd != -1);
            assert(message);
            constexpr const size_t bufSize = sizeof("HH:MM:SS");
            char timeStampBuf[bufSize];
            {
                tm eventTimeTm;
                {
                    #ifndef NDEBUG
                    tm * const r =
                    #endif
                            localtime_r(&time.tv_sec, &eventTimeTm);
                    assert(r);
                }
                {
                    #ifndef NDEBUG
                    const size_t r =
                    #endif
                        strftime(timeStampBuf,
                                 bufSize,
                                 "%H:%M:%S",
                                 &eventTimeTm);
                    assert(r == bufSize - 1u);
                }
            }
            const iovec iov[] = {
                { const_cast<char *>(timeStampBuf), bufSize - 1u },
                { const_cast<char *>(" "), 1u },
                { const_cast<char *>(priorityStringRightPadded(priority)), 7u },
                { const_cast<char *>(" "), 1u },
                { const_cast<char *>(message), strlen(message) },
                { const_cast<char *>("\n"), 1u }
            };
            const Fluffy::QueueingMutex::Guard guard(mutex);
            #ifdef __GNUC__
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wunused-result"
            #endif
            (void) writev(fd, iov, sizeof(iov) / sizeof(iovec));
            #ifdef __GNUC__
            #pragma GCC diagnostic pop
            #endif
            sync(fd);
        }

    private: /* Fields: */

        const int m_fd;
        Fluffy::QueueingMutex m_mutex;

    }; /* class CFileAppender { */

    class StdAppender: public Appender {

    public: /* Types: */

        FLUFFY_DEFINE_EXCEPTION(std::exception, Exception);
        FLUFFY_DEFINE_EXCEPTION_CONST_MSG(
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
                        const Priority priority,
                        const char * message) noexcept override
        {
            if (priority <= Priority::Warning) {
                CFileAppender::logToFile(STDERR_FILENO, time, priority, message,
                                         m_stderrMutex);
            } else {
                CFileAppender::logToFile(STDOUT_FILENO, time, priority, message,
                                         m_stdoutMutex);
            }
        }

        Fluffy::QueueingMutex m_stderrMutex;
        Fluffy::QueueingMutex m_stdoutMutex;

    };

    class FileAppender: public Appender {

    public: /* Types: */

        enum OpenMode { APPEND, OVERWRITE };

    public: /* Methods: */

        template <typename Path>
        FileAppender(Path && path,
                     const OpenMode openMode,
                     const mode_t flags = 0644)
            : m_path(std::forward<Path>(path))
            , m_fd(open(m_path.c_str(),
                        // No O_SYNC since it would hurt performance badly
                        O_WRONLY | O_CREAT | O_APPEND | O_NOCTTY
                        | ((openMode == OVERWRITE) ? O_TRUNC : 0u),
                        flags))
        {
            if (m_fd == -1)
                throw std::system_error(errno, std::system_category());
        }

        inline ~FileAppender() noexcept override { close(m_fd); }

        inline void log(timeval time,
                        const Priority priority,
                        const char * message) noexcept override
        { CFileAppender::logToFile(m_fd, time, priority, message, m_mutex); }

    private: /* Fields: */

        const std::string m_path;
        const int m_fd;
        Fluffy::QueueingMutex m_mutex;

    }; /* class FileAppender */

public: /* Methods: */

    virtual ~Backend() noexcept {
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

    template <Priority priority>
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

}; /* class Backend { */

} /* namespace LogHard { */

#endif /* LOGHARD_BACKEND_H */
