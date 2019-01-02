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
#include <sharemind/Concepts.h>
#include <sharemind/DebugOnly.h>
#include <sharemind/Concat.h>
#include <sharemind/Uuid.h>
#include <sharemind/TemplateContainsType.h>
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

    class MessageBuilder {

    public: /* Methods: */

        MessageBuilder(MessageBuilder &&) noexcept = default;
        MessageBuilder(MessageBuilder const &) = delete;
        MessageBuilder & operator=(MessageBuilder &&) noexcept = delete;
        MessageBuilder & operator=(MessageBuilder const &) = delete;

        MessageBuilder(Priority priority, Logger const &) noexcept;
        MessageBuilder(::timeval, Priority priority, Logger const &) noexcept;


        ~MessageBuilder() noexcept;


        #define LOGHARD_LOGGER_H_(...) \
            MessageBuilder & operator<<(__VA_ARGS__) noexcept
        LOGHARD_LOGGER_H_(char);
        LOGHARD_LOGGER_H_(bool);
        LOGHARD_LOGGER_H_(signed char);
        LOGHARD_LOGGER_H_(unsigned char);
        LOGHARD_LOGGER_H_(short);
        LOGHARD_LOGGER_H_(unsigned short);
        LOGHARD_LOGGER_H_(int);
        LOGHARD_LOGGER_H_(unsigned int);
        LOGHARD_LOGGER_H_(long);
        LOGHARD_LOGGER_H_(unsigned long);
        LOGHARD_LOGGER_H_(long long);
        LOGHARD_LOGGER_H_(unsigned long long);
        LOGHARD_LOGGER_H_(Logger::Hex<unsigned char>);
        LOGHARD_LOGGER_H_(Logger::Hex<unsigned short>);
        LOGHARD_LOGGER_H_(Logger::Hex<unsigned int>);
        LOGHARD_LOGGER_H_(Logger::Hex<unsigned long>);
        LOGHARD_LOGGER_H_(Logger::Hex<unsigned long long>);
        LOGHARD_LOGGER_H_(Logger::HexByte);
        LOGHARD_LOGGER_H_(double);
        LOGHARD_LOGGER_H_(long double);
        LOGHARD_LOGGER_H_(float);
        LOGHARD_LOGGER_H_(char const *);
        LOGHARD_LOGGER_H_(std::string const &);
        LOGHARD_LOGGER_H_(void *);
        LOGHARD_LOGGER_H_(void const * const);
        LOGHARD_LOGGER_H_(sharemind::Uuid const &);
        #undef LOGHARD_LOGGER_H_

    private: /* Methods: */

        MessageBuilder & elide() noexcept;

    private: /* Fields: */

        std::shared_ptr<Backend> m_backend;
        Priority m_priority;

    }; /* struct MessageBuilder { */

    struct StandardExceptionFormatter {

        void operator()(std::size_t const exceptionNumber,
                        std::size_t const totalExceptions,
                        std::exception_ptr exception,
                        MessageBuilder mb) const noexcept
        {
            assert(exception);
            mb << "  * Exception " << exceptionNumber << " of "
               << totalExceptions;
            try {
                std::rethrow_exception(std::move(exception));
            } catch (std::exception const & e) {
                mb << ": " << e.what();
            } catch (...) {
                mb << " is not an std::exception!";
            }
        }

    }; /* StandardExceptionFormatter */

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

    MessageBuilder fatal() const noexcept;
    MessageBuilder error() const noexcept;
    MessageBuilder warning() const noexcept;
    MessageBuilder info() const noexcept;
    MessageBuilder debug() const noexcept;
    MessageBuilder fullDebug() const noexcept;

    MessageBuilder fatal(::timeval theTime) const noexcept;
    MessageBuilder error(::timeval theTime) const noexcept;
    MessageBuilder warning(::timeval theTime) const noexcept;
    MessageBuilder info(::timeval theTime) const noexcept;
    MessageBuilder debug(::timeval theTime) const noexcept;
    MessageBuilder fullDebug(::timeval theTime) const noexcept;

    template <typename T>
    static Hex<T> hex(T const value) noexcept { return {value}; }

    template <Priority PRIORITY = Priority::Error>
    void printCurrentException() const noexcept
    {
        printException<PRIORITY>(std::current_exception(),
                                 now(),
                                 StandardExceptionFormatter());
    }

    template <Priority PRIORITY = Priority::Error>
    void printCurrentException(::timeval theTime) const noexcept {
        printException<PRIORITY>(std::current_exception(),
                                 std::move(theTime),
                                 StandardExceptionFormatter());
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
    { printException<PRIORITY>(std::move(e), now(), StandardExceptionFormatter()); }

    template <Priority PRIORITY = Priority::Error>
    void printException(std::exception_ptr e, ::timeval theTime) const noexcept
    {
        printException<PRIORITY>(std::move(e),
                                 std::move(theTime),
                                 StandardExceptionFormatter());
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
                                     MessageBuilder(theTime, PRIORITY, *this));
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
#define LOGHARD_EXTERN(pri) \
    LOGHARD_ETCN(void Logger::printCurrentException<Priority::pri>()) \
    LOGHARD_ETCN(void Logger::printCurrentException<Priority::pri>(::timeval)) \
    LOGHARD_ETCN( \
        void Logger::printCurrentException<Priority::pri, \
                                           Logger::StandardExceptionFormatter>(\
                StandardExceptionFormatter &&)) \
    LOGHARD_ETCN( \
        void Logger::printCurrentException<Priority::pri, \
                                           Logger::StandardExceptionFormatter>(\
                ::timeval, StandardExceptionFormatter &&))

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
