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

#ifndef LOGHARD_LOGGER_H
#define LOGHARD_LOGGER_H

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <sharemind/Concat.h>
#include <sharemind/Uuid.h>
#include <string>
#include <sys/time.h>
#include <type_traits>
#include <utility>
#include "Backend.h"
#include "Priority.h"


#ifndef LOGHARD_LOGLEVEL_MAXDEBUG
#define LOGHARD_LOGLEVEL_MAXDEBUG ::LogHard::Priority::Debug
#endif

namespace LogHard {

class Logger {

public: /* Types: */

    template <typename T> struct Hex {
        static_assert(std::is_arithmetic<T>::value, "T is not arithmetic!");
        static_assert(std::is_unsigned<T>::value, "T is not unsigned!");
        T const value;
    };

    struct HexByte { uint8_t const value; };

    template <Priority> class LogHelper;

    class NullLogHelperBase {

        template <Priority> friend class LogHelper;

    public : /* Methods: */

        template <class T>
        NullLogHelperBase & operator<<(T &&) noexcept
        { return *this; }

    private: /* Methods: */

        template <typename ... Args>
        NullLogHelperBase(Args && ...) noexcept {}

    }; /* class NullLogHelperBase { */

private: /* Types: */

    struct LogHelperContents {

    /* Methods: */

        LogHelperContents(LogHelperContents const &) = delete;
        LogHelperContents & operator=(LogHelperContents const &) = delete;

        LogHelperContents(LogHelperContents &&) noexcept = default;
        LogHelperContents & operator=(LogHelperContents &&) noexcept = default;

        LogHelperContents(::timeval, std::shared_ptr<Backend>) noexcept;
        LogHelperContents(::timeval,
                          std::shared_ptr<Backend>,
                          std::string const &) noexcept;
        LogHelperContents(::timeval, std::shared_ptr<Backend>, char const *)
                noexcept;

        void finish(Priority priority) noexcept;

        void log(char) noexcept;
        void log(bool) noexcept;
        void log(signed char) noexcept;
        void log(unsigned char) noexcept;
        void log(short) noexcept;
        void log(unsigned short) noexcept;
        void log(int) noexcept;
        void log(unsigned int) noexcept;
        void log(long) noexcept;
        void log(unsigned long) noexcept;
        void log(long long) noexcept;
        void log(unsigned long long) noexcept;
        void log(Logger::Hex<unsigned char>) noexcept;
        void log(Logger::Hex<unsigned short>) noexcept;
        void log(Logger::Hex<unsigned int>) noexcept;
        void log(Logger::Hex<unsigned long>) noexcept;
        void log(Logger::Hex<unsigned long long>) noexcept;
        void log(Logger::HexByte) noexcept;
        void log(double) noexcept;
        void log(long double) noexcept;
        void log(float) noexcept;
        void log(char const *) noexcept;
        void log(std::string const &) noexcept;
        void log(void *) noexcept;
        void log(void const * const) noexcept;
        void log(sharemind::Uuid const &) noexcept;

        void elide() noexcept;

    /* Fields: */

        std::shared_ptr<Backend> m_backend;

    }; /* struct LogHelperContents { */
    static_assert(std::is_nothrow_move_assignable<LogHelperContents>::value,
                  "");
    static_assert(std::is_nothrow_move_constructible<LogHelperContents>::value,
                  "");

public: /* Types: */

    template <Priority priority>
    class LogHelperBase {

        template <Priority> friend class LogHelper;

    public: /* Methods: */

        template <typename ... Args>
        LogHelperBase(Args && ... args)
            : m_contents(std::forward<Args>(args)...)
        {}

        LogHelperBase(LogHelperBase<priority> const &) = delete;
        LogHelperBase<priority> & operator=(LogHelperBase<priority> const &) =
                delete;

        LogHelperBase(LogHelperBase<priority> && move) noexcept
            : m_contents(std::move(move.m_contents))
        {}

        LogHelperBase<priority> & operator=(LogHelperBase<priority> && move)
                noexcept
        {
            m_contents = std::move(move.m_contents);
            return *this;
        }

        ~LogHelperBase() noexcept { m_contents.finish(priority); }

        template <typename T>
        LogHelperBase & operator<<(T && v) noexcept {
            m_contents.log(std::forward<T>(v));
            return *this;
        }

    private: /* Fields: */

        LogHelperContents m_contents;

    }; /* class LogHelperBase { */

    template <Priority priority = Priority::Debug>
    class LogHelper
            : public std::conditional<priority <= LOGHARD_LOGLEVEL_MAXDEBUG,
                                      LogHelperBase<priority>,
                                      NullLogHelperBase>::type
    {

        friend class Logger;

    private: /* Methods: */

        template <typename BackendPtr, typename ... Args>
        LogHelper(::timeval theTime, BackendPtr && backend, Args && ... args)
                noexcept
            : std::conditional<priority <= LOGHARD_LOGLEVEL_MAXDEBUG,
                               LogHelperBase<priority>,
                               NullLogHelperBase>::type(
                                    std::move(theTime),
                                    std::forward<BackendPtr>(backend),
                                    std::forward<Args>(args)...)
        {}

    }; /* class LogHelper */

    template <Priority> friend class LogHelper;

    struct StandardFormatter {

        template <typename OutStream>
        void operator()(std::size_t const exceptionNumber,
                        std::size_t const totalExceptions,
                        std::exception_ptr exception,
                        OutStream out) const noexcept
        {
            assert(exception);
            out << "  * Exception " << exceptionNumber << " of "
                << totalExceptions;
            try {
                std::rethrow_exception(std::move(exception));
            } catch (std::exception const & e) {
                out << ": " << e.what();
            } catch (...) {
                out << " is not an std::exception!";
            }
        }

    };

public: /* Methods: */

    Logger(std::shared_ptr<Backend> backend) noexcept;

    template <typename Arg, typename ... Args>
    Logger(std::shared_ptr<Backend> backend, Arg && arg, Args && ... args)
            noexcept
        : m_backend(std::move(backend))
        , m_prefix(sharemind::concat(std::forward<Arg>(arg),
                                     std::forward<Args>(args)...,
                                     ' '))
    {}

    Logger(Logger && move) noexcept;

    Logger(Logger const & copy) noexcept;

    template <typename Arg, typename ... Args>
    Logger(Logger const & logger, Arg && arg, Args && ... args) noexcept
        : m_backend(logger.m_backend)
        , m_prefix(logger.m_prefix.empty()
                   ? sharemind::concat(std::forward<Arg>(arg),
                                       std::forward<Args>(args)...,
                                       ' ')
                   : (*(logger.m_prefix.crbegin()) != ' ')
                     ? sharemind::concat(logger.m_prefix,
                                         std::forward<Arg>(arg),
                                         std::forward<Args>(args)...,
                                         ' ')
                     : sharemind::concat(
                           std::string(logger.m_prefix.cbegin(),
                                       logger.m_prefix.cend() - 1),
                           std::forward<Arg>(arg),
                           std::forward<Args>(args)...,
                           ' '))
        , m_basePrefix(logger.m_prefix)
    {}

    Logger & operator=(Logger &&) = delete;
    Logger & operator=(Logger const &) = delete;

    std::shared_ptr<Backend> backend() const noexcept { return m_backend; }

    std::string const & prefix() const noexcept { return m_prefix; }

    std::string const & basePrefix() const noexcept { return m_prefix; }

    Backend::Lock retrieveBackendLock() const noexcept
    { return m_backend->retrieveLock(); }

    template <typename ... Args> void setPrefix(Args && ... args) {
        m_prefix = sharemind::concat(m_basePrefix,
                                     std::forward<Args>(args)...,
                                     ' ');
    }

    template <Priority PRIORITY, bool usePrefix = true>
    LogHelper<PRIORITY> logHelper() const noexcept
    { return logHelper<PRIORITY, usePrefix>(now()); }

    template <Priority PRIORITY, bool usePrefix = true>
    LogHelper<PRIORITY> logHelper(::timeval theTime) const noexcept {
        return usePrefix
               ? LogHelper<PRIORITY>(std::move(theTime), m_backend, m_prefix)
               : LogHelper<PRIORITY>(std::move(theTime), m_backend);
    }

    LogHelper<Priority::Fatal> fatal() const noexcept
    { return logHelper<Priority::Fatal>(); }

    LogHelper<Priority::Error> error() const noexcept
    { return logHelper<Priority::Error>(); }

    LogHelper<Priority::Warning> warning() const noexcept
    { return logHelper<Priority::Warning>(); }

    LogHelper<Priority::Normal> info() const noexcept
    { return logHelper<Priority::Normal>(); }

    LogHelper<Priority::Debug> debug() const noexcept
    { return logHelper<Priority::Debug>(); }

    LogHelper<Priority::FullDebug> fullDebug() const noexcept
    { return logHelper<Priority::FullDebug>(); }

    template <typename T>
    static Hex<T> hex(T const value) noexcept { return {value}; }

    template <Priority PRIORITY = Priority::Error>
    void printCurrentException() const noexcept
    { printCurrentException<PRIORITY>(now(), StandardFormatter()); }

    template <Priority PRIORITY = Priority::Error>
    void printCurrentException(::timeval theTime) const noexcept {
        printCurrentException<PRIORITY>(std::move(theTime),
                                        StandardFormatter());
    }

    template <Priority PRIORITY = Priority::Error, typename Formatter>
    void printCurrentException(Formatter && formatter) const noexcept {
        return printCurrentException<PRIORITY, Formatter>(
                       now(),
                       std::forward<Formatter>(formatter));
    }

    template <Priority PRIORITY = Priority::Error, typename Formatter>
    void printCurrentException(::timeval theTime, Formatter && formatter)
            const noexcept
    {
        if (auto e = std::current_exception()) {
            auto printer =
                [this, &formatter, theTime](std::size_t const exceptionNumber,
                                            std::size_t const totalExceptions,
                                            std::exception_ptr e_)
                {
                    return formatter(std::move(exceptionNumber),
                                     std::move(totalExceptions),
                                     std::move(e_),
                                     logHelper<PRIORITY>(theTime));
                };

            std::size_t levels = 1u;
            printException_(std::move(e), 1u, levels, printer);
        }
    }

    static ::timeval now() noexcept;

private: /* Methods: */

    template <typename Printer>
    void printException_(std::exception_ptr e,
                         std::size_t const levelNow,
                         std::size_t & totalLevels,
                         Printer && printer) const noexcept
    {
        assert(e);
        try {
            std::rethrow_exception(e);
        } catch (std::nested_exception const & e2) {
            auto const ne(e2.nested_ptr());
            if (ne)
                printException_(ne, levelNow + 1u, ++totalLevels, printer);
        } catch (...) {}
        std::forward<Printer>(printer)(
                       levelNow,
                       static_cast<std::size_t const>(totalLevels),
                       std::move(e));
    }

private: /* Fields: */

    std::shared_ptr<Backend> m_backend;
    std::string m_prefix;
    std::string m_basePrefix;

}; /* class Logger { */

} /* namespace LogHard { */

#endif /* LOGHARD_LOGGER_H */
