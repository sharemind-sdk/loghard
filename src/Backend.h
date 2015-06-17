/*
 * Copyright (C) 2015 Cybernetica
 *
 * Research/Commercial License Usage
 * Licensees holding a valid Research License or Commercial License
 * for the Software may use this file according to the written
 * agreement between you and Cybernetica.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl-3.0.html.
 *
 * For further information, please contact us at sharemind@cyber.ee.
 */

#ifndef LOGHARD_BACKEND_H
#define LOGHARD_BACKEND_H

#include <cassert>
#include <cstdio>
#include <fcntl.h>
#include <map>
#include <memory>
#include <mutex>
#include <sharemind/compiler-support/GccPR50025.h>
#include <sharemind/Exception.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <syslog.h>
#include <system_error>
#include <utility>
#include <unistd.h>
#include <vector>
#include "Priority.h"


namespace LogHard {

class Logger;

class Backend {

    friend class Logger;

private: /* Types: */

    using Mutex = std::recursive_mutex;
    using Guard = std::lock_guard<Mutex>;

public: /* Types: */

    using Lock = std::unique_lock<Mutex>;

    class Appender;
    using Appenders = std::map<Appender *, std::unique_ptr<Appender> >;

    class Appender {

    public: /* Methods: */

        virtual ~Appender() noexcept {}

        virtual void activate(Appenders const & appenders) { (void) appenders; }

        virtual void log(timeval time,
                         Priority priority,
                         char const * message) noexcept = 0;

        inline static char const * priorityString(Priority const priority)
                noexcept
        {
            static char const strings[][8u] =
                    { "FATAL", "ERROR", "WARNING", "INFO", "DEBUG", "DEBUG2" };
            return &strings[static_cast<unsigned>(priority)][0u];
        }

        inline static char const * priorityStringRightPadded(
                Priority const priority) noexcept
        {
            static char const strings[][8u] = {
                "FATAL  ", "ERROR  ", "WARNING", "INFO   ", "DEBUG  ", "DEBUG2 "
            };
            return &strings[static_cast<unsigned>(priority)][0u];
        }

    }; /* class Appender { */

    class BackendAppender: public Appender {

    public: /* Methods: */

        inline BackendAppender(Backend & backend)
            : m_backend SHAREMIND_GCCPR50025_WORKAROUND(backend)
        {}

        /// \todo Implement activate() to check for loops.

        inline void log(timeval time,
                        Priority const priority,
                        char const * message) noexcept override
        { m_backend.doLog(time, priority, message); }

    private: /* Fields: */

        Backend & m_backend;

    };

    class SyslogAppender: public Appender {

    public: /* Types: */

        SHAREMIND_DEFINE_EXCEPTION(std::exception, Exception);
        SHAREMIND_DEFINE_EXCEPTION_CONST_MSG(
                Exception,
                MultipleSyslogAppenderException,
                "Multiple Syslog appenders not allowed!");

    public: /* Methods: */

        inline SyslogAppender(std::string ident,
                              int const logopt,
                              int const facility)
            : m_ident{std::move(ident)}
            , m_logopt{logopt}
            , m_facility{facility}
        {}

        inline ~SyslogAppender() noexcept override { closelog(); }

        void activate(Appenders const & appenders) override {
            for (Appenders::value_type const & a : appenders)
                if (a.second.get() != this
                    && dynamic_cast<SyslogAppender *>(a.second.get()) != nullptr)
                    throw MultipleSyslogAppenderException{};
            openlog(m_ident.c_str(), m_logopt, m_facility);
        }

        inline void log(timeval,
                        Priority const priority,
                        char const * message) noexcept override
        {
            constexpr static int const priorities[] = {
                LOG_EMERG, LOG_ERR, LOG_WARNING, LOG_INFO, LOG_DEBUG, LOG_DEBUG
            };
            syslog(priorities[static_cast<unsigned>(priority)], "%s", message);
        }

    private: /* Fields: */

        std::string const m_ident;
        int const m_logopt;
        int const m_facility;

    }; /* class SyslogAppender { */

    class CFileAppender: public Appender {

    public: /* Methods: */

        CFileAppender(FILE * const file)
            : m_fd(fileno(file))
        {
            if (m_fd == -1)
                throw std::system_error(errno, std::system_category());
        }

        inline void log(timeval time,
                        Priority const priority,
                        char const * message) noexcept override
        { logToFileSync(m_fd, time, priority, message); }

        static inline void logToFile(int const fd,
                                     timeval time,
                                     Priority const priority,
                                     char const * const message) noexcept
        { logToFile__(fd, time, priority, message, [](int const){}); }

        static inline void logToFileSync(
                int const fd,
                timeval time,
                Priority const priority,
                char const * const message) noexcept
        {
            logToFile__(fd, time, priority, message,
                        [](int const f){ fsync(f); });
        }

        static inline void logToFile(FILE * file,
                                     timeval time,
                                     Priority const priority,
                                     char const * const message) noexcept
        {
            int const fd = fileno(file);
            assert(fd != -1);
            logToFile(fd, time, priority, message);
        }

        static inline void logToFileSync(
                FILE * file,
                timeval time,
                Priority const priority,
                char const * const message) noexcept
        {
            int const fd = fileno(file);
            assert(fd != -1);
            logToFileSync(fd, time, priority, message);
        }

    private: /* Methods: */

        template <typename Sync>
        static inline void logToFile__(int const fd,
                                       timeval time,
                                       Priority const priority,
                                       char const * const message,
                                       Sync && sync) noexcept
        {
            assert(fd != -1);
            assert(message);
            constexpr size_t const bufSize = sizeof("YYYY.MM.DD HH:MM:SS");
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
                    size_t const r =
                    #endif
                        strftime(timeStampBuf,
                                 bufSize,
                                 "%Y.%m.%d %H:%M:%S",
                                 &eventTimeTm);
                    assert(r == bufSize - 1u);
                }
            }
            iovec const iov[] = {
                { const_cast<char *>(timeStampBuf), bufSize - 1u },
                { const_cast<char *>(" "), 1u },
                { const_cast<char *>(priorityStringRightPadded(priority)), 7u },
                { const_cast<char *>(" "), 1u },
                { const_cast<char *>(message), strlen(message) },
                { const_cast<char *>("\n"), 1u }
            };
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

        int const m_fd;

    }; /* class CFileAppender { */

    class StdAppender: public Appender {

    public: /* Types: */

        SHAREMIND_DEFINE_EXCEPTION(std::exception, Exception);
        SHAREMIND_DEFINE_EXCEPTION_CONST_MSG(
                Exception,
                MultipleStdAppenderException,
                "Multiple Std appenders not allowed!");

    public: /* Methods: */

        void activate(Appenders const & appenders) override {
            for (Appenders::value_type const & a : appenders)
                if (a.second.get() != this
                    && dynamic_cast<StdAppender *>(a.second.get()) != nullptr)
                    throw MultipleStdAppenderException{};
        }

        inline void log(timeval time,
                        Priority const priority,
                        char const * message) noexcept override
        {
            int const fn = (priority <= Priority::Warning)
                            ? STDERR_FILENO
                            : STDOUT_FILENO;
            CFileAppender::logToFile(fn, time, priority, message);
        }

    };

    class FileAppender: public Appender {

    public: /* Types: */

        enum OpenMode { APPEND, OVERWRITE };

    public: /* Methods: */

        FileAppender(std::string const path,
                     OpenMode const openMode,
                     mode_t const flags = 0644)
            : m_path{std::move(path)}
            , m_fd{open(m_path.c_str(),
                        // No O_SYNC since it would hurt performance badly
                        O_WRONLY | O_CREAT | O_APPEND | O_NOCTTY
                        | ((openMode == OVERWRITE) ? O_TRUNC : 0u),
                        flags)}
        {
            if (m_fd == -1)
                throw std::system_error{errno, std::system_category()};
        }

        inline ~FileAppender() noexcept override { close(m_fd); }

        inline void log(timeval time,
                        Priority const priority,
                        char const * message) noexcept override
        { CFileAppender::logToFile(m_fd, time, priority, message); }

    private: /* Fields: */

        std::string const m_path;
        int const m_fd;

    }; /* class FileAppender */

    class EarlyAppender: public Appender {

    public: /* Types: */

        SHAREMIND_DEFINE_EXCEPTION(std::exception, Exception);
        SHAREMIND_DEFINE_EXCEPTION_CONST_MSG(
                Exception,
                MultipleEarlyAppenderException,
                "Multiple early appenders not allowed!");
        SHAREMIND_DEFINE_EXCEPTION_CONST_MSG(
                Exception,
                TooManyEntriesException,
                "Maximum log entry reservation size exceeded!");

        struct LogEntry {
            timeval time;
            Priority priority;
            std::string message;
        };
        using LogEntries = std::vector<LogEntry>;

    public: /* Methods: */

        EarlyAppender(size_t const reserveEntries = 1024u,
                      size_t const maxMessageSize = 1024u)
            : m_oomMessage{"Early log buffer full, messages skipped!"}
            , m_maxMessageSize{(assert(maxMessageSize >= 5u), maxMessageSize)}
        {
            size_t const size = reserveEntries + 1u;
            if (size < reserveEntries)
                throw TooManyEntriesException{};
            m_entries.reserve(size);
            m_freeMessages.resize(reserveEntries);
            for (std::string & msg : m_freeMessages)
                msg.reserve(maxMessageSize);
        }

        LogEntries const & entries() const noexcept { return m_entries; }

        void activate(Appenders const & appenders) override {
            for (Appenders::value_type const & a : appenders)
                if (a.second.get() != this
                    && dynamic_cast<EarlyAppender *>(a.second.get()) != nullptr)
                    throw MultipleEarlyAppenderException{};
        }

        inline void log(timeval time,
                        Priority const priority,
                        char const * message) noexcept override
        {
            if (m_freeMessages.empty()) {
                if (!m_oom) {
                    assert(m_entries.size() < m_entries.capacity());
                    m_entries.emplace_back(LogEntry{time,
                                                    Priority::Error,
                                                    std::move(m_oomMessage)});
                    m_oom = true;
                }
            } else {
                assert(m_entries.size() < m_entries.capacity());
                std::string & s = m_freeMessages.back();
                assert(s.empty());
                assert(s.capacity() >= m_maxMessageSize);
                try {
                    try {
                        s.assign(message);
                    } catch (...) {
                        assert(message);
                        assert(s.empty());
                        assert(s.capacity() >= m_maxMessageSize);
                        do {
                            s.append(message, 1u);
                        } while (*++message);
                    }
                } catch (...) {
                    size_t const size = s.size();
                    assert(size > m_maxMessageSize);
                    static char const elide[] = "[...]";
                    s.replace(size - 5u, 5u, elide, 5u);
                }
                m_entries.emplace_back(LogEntry{time, priority, std::move(s)});
                m_freeMessages.pop_back();
            }
        }

        inline void logToAppender(Appender & appender) const noexcept {
            for (LogEntry const & entry : m_entries)
                appender.log(entry.time, entry.priority, entry.message.c_str());
        }

        inline void clear() noexcept {
            if (m_oom)
                m_oomMessage = std::move(m_entries.back().message);
            m_entries.pop_back();
            for (LogEntry & entry : m_entries)
                m_freeMessages.emplace_back(std::move(entry.message));
            m_entries.clear();
        }

    private: /* Fields: */

        LogEntries m_entries;
        std::vector<std::string> m_freeMessages;
        std::string m_oomMessage;
        bool m_oom = false;
        size_t const m_maxMessageSize;

    }; /* class EarlyAppender */

public: /* Methods: */

    /**
      \brief Adds a BackendAppender to the Logger.
      \param[in] args Arguments to the BackendAppender constructor.
    */
    template <typename ... Args>
    inline BackendAppender & addBackendAppender(Args && ... args) {
        return constructAndAddAppender<BackendAppender,
                                       Args...>(std::forward<Args>(args)...);
    }

    /**
      \brief Adds a BackendAppender to the Logger.
      \param[in] backend The LogHard backend to pass messages to.
    */
    inline BackendAppender & addAppender(Backend & backend)
    { return addBackendAppender(backend); }

    /**
      \brief Adds a SyslogAppender to the Logger.
      \param[in] args Arguments to the SyslogAppender constructor.
    */
    template <typename ... Args>
    inline SyslogAppender & addSyslogAppender(Args && ... args) {
        return constructAndAddAppender<SyslogAppender,
                                       Args...>(std::forward<Args>(args)...);
    }

    /**
      \brief Adds a FileAppender to the Logger.
      \param[in] args Arguments to the FileAppender constructor.
    */
    template <typename ... Args>
    inline FileAppender & addFileAppender(Args && ... args) {
        return constructAndAddAppender<FileAppender,
                                       Args...>(std::forward<Args>(args)...);
    }

    /**
      \brief Adds a CFileAppender to the Logger.
      \param[in] args Arguments to the CFileAppender constructor.
    */
    template <typename ... Args>
    inline CFileAppender & addCFileAppender(Args && ... args) {
        return constructAndAddAppender<CFileAppender,
                                       Args...>(std::forward<Args>(args)...);
    }

    /**
      \brief Adds a StdAppender to the Logger.
      \param[in] args Arguments to the StdAppender constructor.
    */
    template <typename ... Args>
    inline StdAppender & addStdAppender(Args && ... args) {
        return constructAndAddAppender<StdAppender,
                                       Args...>(std::forward<Args>(args)...);
    }

    inline Appender & addAppender(std::unique_ptr<Appender> appender) {
        assert(appender);
        Appender & a = *appender;
        Guard const guard{m_mutex};
        auto const r =
                m_appenders.insert(Appenders::value_type{&a,
                                                         std::move(appender)});
        assert(r.second);
        assert(r.first->second.get() == &a);
        try {
            a.activate(m_appenders);
            return a;
        } catch (...) {
            m_appenders.erase(r.first);
            throw;
        }
    }

    /**
      \brief Adds an EarlyAppender to the Logger.
      \param[in] args Arguments to the EarlyAppender constructor.
    */
    template <typename ... Args>
    inline EarlyAppender & addEarlyAppender(Args && ... args) {
        return constructAndAddAppender<EarlyAppender,
                                       Args...>(std::forward<Args>(args)...);
    }

    inline std::unique_ptr<Appender> takeAppender(Appender & appender) noexcept
    {
        Guard const guard{m_mutex};
        Appenders::iterator it{m_appenders.find(&appender)};
        assert(it != m_appenders.end());
        std::unique_ptr<Appender> r{std::move(it->second)};
        m_appenders.erase(it);
        return r;
    }

private: /* Methods: */

    inline Lock retrieveLock() noexcept { return Lock{m_mutex}; }

    inline void doLog(timeval const time,
                      Priority const priority,
                      char const * const message) noexcept
    {
        Guard const guard{m_mutex};
        for (Appenders::value_type const & a : m_appenders)
            a.second->log(time, priority, message);
    }

    template <typename AppenderType, typename ... Args>
    inline AppenderType & constructAndAddAppender(Args && ... args) {
        return static_cast<AppenderType &>(
                    addAppender(
                        std::unique_ptr<Appender>{
                            new AppenderType{std::forward<Args>(args)...}}));
    }

private: /* Fields: */

    std::recursive_mutex m_mutex;
    Appenders m_appenders;

}; /* class Backend { */

} /* namespace LogHard { */

#endif /* LOGHARD_BACKEND_H */
