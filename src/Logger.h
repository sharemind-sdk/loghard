/*
 * Copyright (C) Cybernetica
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
#include <sharemind/AssertReturn.h>
#include <sharemind/DebugOnly.h>
#include <sharemind/Concat.h>
#include <sharemind/Uuid.h>
#include <string>
#include <sys/time.h>
#include <type_traits>
#include <utility>
#include "Backend.h"
#include "Priority.h"


/*
    LOGHARD_DEVMSG is a macro which may be used for debug-build-only messages
    with hope that the compiler will optimize to remove the whole statement if
    the NDEBUG macro is defined. Usage example:

        m_logger.LOGHARD_DEVMSG() << "Here";

    Note that any side-effects of expressions passed to it will still execute.
*/
#ifndef LOGHARD_DEVMSG
#ifndef NDEBUG
#define LOGHARD_DEVMSG fullDebug
#else
#define LOGHARD_DEVMSG discard
#endif
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

    class NullLogHelper {

    public : /* Methods: */

        NullLogHelper() noexcept {}

        NullLogHelper(NullLogHelper &&) noexcept = default;
        NullLogHelper(NullLogHelper const &) = delete;
        NullLogHelper & operator=(NullLogHelper &&) noexcept = default;
        NullLogHelper & operator=(NullLogHelper const &) = delete;

        NullLogHelper(Logger const & SHAREMIND_DEBUG_ONLY(logger)) noexcept
        { assert(logger.backend()); }

        NullLogHelper(::timeval,
                      Logger const & SHAREMIND_DEBUG_ONLY(logger)) noexcept
        { assert(logger.backend()); }

        template <class T>
        NullLogHelper & operator<<(T &&) noexcept
        { return *this; }

    }; /* class NullLogHelper { */

private: /* Types: */

    struct LogHelperBase {

    /* Methods: */

        LogHelperBase(LogHelperBase &&) noexcept = default;
        LogHelperBase(LogHelperBase const &) = delete;
        LogHelperBase & operator=(LogHelperBase &&) noexcept = default;
        LogHelperBase & operator=(LogHelperBase const &) = delete;

        LogHelperBase(Logger const &) noexcept;
        LogHelperBase(::timeval, Logger const &) noexcept;

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

    }; /* struct LogHelperBase { */
    static_assert(std::is_nothrow_move_assignable<LogHelperBase>::value, "");
    static_assert(std::is_nothrow_move_constructible<LogHelperBase>::value, "");

public: /* Types: */

    template <Priority priority>
    class LogHelper: private LogHelperBase {

    public: /* Methods: */

        LogHelper(LogHelper &&) noexcept = default;
        LogHelper(LogHelper const &) = delete;
        LogHelper & operator=(LogHelper &&) noexcept = default;
        LogHelper & operator=(LogHelper const &) = delete;

        using LogHelperBase::LogHelperBase;

        ~LogHelper() noexcept { finish(priority); }

        template <typename T>
        LogHelper & operator<<(T && v) noexcept {
            log(std::forward<T>(v));
            return *this;
        }

    }; /* class LogHelper { */

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
        : m_backend(sharemind::assertReturn(std::move(backend)))
        , m_prefix(sharemind::concat(std::forward<Arg>(arg),
                                     std::forward<Args>(args)...,
                                     ' '))
    {}

    Logger(Logger && move) noexcept;

    Logger(Logger const & copy) noexcept;

    template <typename Arg, typename ... Args>
    Logger(Logger const & logger, Arg && arg, Args && ... args) noexcept
        : m_backend(sharemind::assertReturn(logger.m_backend))
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

    ~Logger() noexcept;

    Logger & operator=(Logger &&) = delete;
    Logger & operator=(Logger const &) = delete;

    std::shared_ptr<Backend> const & backend() const noexcept
    { return m_backend; }

    std::string const & prefix() const noexcept { return m_prefix; }

    std::string const & basePrefix() const noexcept { return m_prefix; }

    Backend::Lock retrieveBackendLock() const noexcept
    { return m_backend->retrieveLock(); }

    template <typename ... Args> void setPrefix(Args && ... args) {
        m_prefix = sharemind::concat(m_basePrefix,
                                     std::forward<Args>(args)...,
                                     ' ');
    }

    template <Priority PRIORITY>
    LogHelper<PRIORITY> logHelper() const noexcept {
        using R = LogHelper<PRIORITY>;
        static_assert(!std::is_nothrow_copy_assignable<R>::value, "");
        static_assert(!std::is_nothrow_copy_constructible<R>::value, "");
        static_assert(std::is_nothrow_move_assignable<R>::value, "");
        static_assert(std::is_nothrow_move_constructible<R>::value, "");
        static_assert(std::is_nothrow_constructible<R, decltype(*this)>::value,
                      "");
        static_assert(std::is_nothrow_destructible<R>::value, "");
        return R(*this);
    }

    template <Priority PRIORITY>
    LogHelper<PRIORITY> logHelper(::timeval theTime) const noexcept {
        using R = LogHelper<PRIORITY>;
        static_assert(!std::is_nothrow_copy_assignable<R>::value, "");
        static_assert(!std::is_nothrow_copy_constructible<R>::value, "");
        static_assert(std::is_nothrow_move_assignable<R>::value, "");
        static_assert(std::is_nothrow_move_constructible<R>::value, "");
        static_assert(
                    std::is_nothrow_constructible<
                        R,
                        decltype(std::move(theTime)),
                        decltype(*this)>::value, "");
        static_assert(std::is_nothrow_destructible<R>::value, "");
        return R(std::move(theTime), *this);
    }

    LogHelper<Priority::Fatal> fatal() const noexcept;
    LogHelper<Priority::Error> error() const noexcept;
    LogHelper<Priority::Warning> warning() const noexcept;
    LogHelper<Priority::Normal> info() const noexcept;
    LogHelper<Priority::Debug> debug() const noexcept;
    LogHelper<Priority::FullDebug> fullDebug() const noexcept;

    NullLogHelper discard() const noexcept
    { return NullLogHelper(*this); }

    template <typename T>
    static Hex<T> hex(T const value) noexcept { return {value}; }

    template <Priority PRIORITY = Priority::Error>
    void printCurrentException() const noexcept
    {
        printException<PRIORITY>(std::current_exception(),
                                 now(),
                                 StandardFormatter());
    }

    template <Priority PRIORITY = Priority::Error>
    void printCurrentException(::timeval theTime) const noexcept {
        printException<PRIORITY>(std::current_exception(),
                                 std::move(theTime),
                                 StandardFormatter());
    }

    template <Priority PRIORITY = Priority::Error, typename Formatter>
    void printCurrentException(Formatter && formatter) const noexcept {
        return printException<PRIORITY, Formatter>(
                    std::current_exception(),
                    now(),
                    std::forward<Formatter>(formatter));
    }

    template <Priority PRIORITY = Priority::Error, typename Formatter>
    void printCurrentException(::timeval theTime, Formatter && formatter)
            const noexcept
    {
        return printException<PRIORITY>(std::current_exception(),
                                        std::move(theTime),
                                        std::move(formatter));
    }

    template <Priority PRIORITY = Priority::Error>
    void printException(std::exception_ptr e) const noexcept
    { printException<PRIORITY>(std::move(e), now(), StandardFormatter()); }

    template <Priority PRIORITY = Priority::Error>
    void printException(std::exception_ptr e, ::timeval theTime) const noexcept
    {
        printException<PRIORITY>(std::move(e),
                                 std::move(theTime),
                                 StandardFormatter());
    }

    template <Priority PRIORITY = Priority::Error, typename Formatter>
    void printException(std::exception_ptr e, Formatter && formatter)
            const noexcept
    {
        return printCurrentException<PRIORITY, Formatter>(
                    std::move(e),
                    now(),
                    std::forward<Formatter>(formatter));
    }

    template <Priority PRIORITY = Priority::Error, typename Formatter>
    void printException(std::exception_ptr e,
                        ::timeval theTime,
                        Formatter && formatter) const noexcept
    {
        if (e) {
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
        std::forward<Printer>(printer)(levelNow,
                                       std::size_t(totalLevels),
                                       std::move(e));
    }

private: /* Fields: */

    std::shared_ptr<Backend> m_backend;
    std::string m_prefix;
    std::string m_basePrefix;

}; /* class Logger { */

// Extern template declarations:

#define LOGHARD_ETCN(...) extern template __VA_ARGS__ const noexcept;
#define LOGHARD_EXTERN_LH(pri,...) \
    LOGHARD_ETCN(Logger::LogHelper<Priority::pri> \
                 Logger::logHelper<Priority::pri>(__VA_ARGS__))
#define LOGHARD_EXTERN(pri) \
    extern template class Logger::LogHelper<Priority::pri>; \
    LOGHARD_ETCN( \
        void Logger::StandardFormatter::operator()( \
                std::size_t const, \
                std::size_t const, \
                std::exception_ptr, \
                Logger::LogHelper<Priority::pri>)) \
    LOGHARD_EXTERN_LH(pri,) \
    LOGHARD_EXTERN_LH(pri, ::timeval) \
    LOGHARD_ETCN(void Logger::printCurrentException<Priority::pri>()) \
    LOGHARD_ETCN(void Logger::printCurrentException<Priority::pri>(::timeval)) \
    LOGHARD_ETCN( \
        void Logger::printCurrentException<Priority::pri, \
                                           Logger::StandardFormatter>( \
                StandardFormatter &&)) \
    LOGHARD_ETCN( \
        void Logger::printCurrentException<Priority::pri, \
                                           Logger::StandardFormatter>( \
                ::timeval, StandardFormatter &&))

LOGHARD_EXTERN(Fatal)
LOGHARD_EXTERN(Error)
LOGHARD_EXTERN(Warning)
LOGHARD_EXTERN(Normal)
LOGHARD_EXTERN(Debug)
LOGHARD_EXTERN(FullDebug)

#undef LOGHARD_EXTERN
#undef LOGHARD_EXTERN_LH
#undef LOGHARD_ETCN

} /* namespace LogHard { */

#endif /* LOGHARD_LOGGER_H */
