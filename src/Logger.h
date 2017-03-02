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
#include <cstdint>
#include <cstring>
#include <memory>
#include <sharemind/compiler-support/GccVersion.h>
#include <sharemind/Concat.h>
#include <sharemind/DebugOnly.h>
#include <sharemind/Uuid.h>
#include <sstream>
#include <string>
#include <sys/time.h>
#include <type_traits>
#include <utility>
#include "Backend.h"
#include "Priority.h"


#ifndef LOGHARD_LOGLEVEL_MAXDEBUG
#define LOGHARD_LOGLEVEL_MAXDEBUG ::LogHard::Priority::Debug
#endif

/*
  TLS is Clang is only available when using Clang with GNU libstdc++ 4.8 or
  later. However there's no good way to determine the version of libstdc++ using
  preprocessor macros. The __GLIBCXX__ macro gives us a date, but unfortunately
  its value in libstdc++ 4.7.4 is greater than in 4.8.0, hence it is not
  entirely reliable.
*/
#if 1 || (defined(__clang__) && (!defined(__GLIBCXX__) \
                            || __GLIBCXX__ < 20130322)) || \
    (defined(SHAREMIND_GCC_VERSION) && SHAREMIND_GCC_VERSION < 40800)
#define LOGHARD_HAVE_TLS 0
#else
#define LOGHARD_HAVE_TLS 1
#endif

namespace LogHard {

namespace Detail {

static constexpr std::size_t MAX_MESSAGE_SIZE = 1024u * 16u;
static_assert(MAX_MESSAGE_SIZE >= 1u, "Invalid MAX_MESSAGE_SIZE");
static constexpr std::size_t STACK_BUFFER_SIZE = MAX_MESSAGE_SIZE + 4u;
static_assert(STACK_BUFFER_SIZE > MAX_MESSAGE_SIZE, "Overflow");

#if LOGHARD_HAVE_TLS
extern thread_local char tl_message[STACK_BUFFER_SIZE];
#endif

} /* namespace Detail { */

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
        inline NullLogHelperBase & operator<<(T &&) noexcept
        { return *this; }

    private: /* Methods: */

        template <typename ... Args>
        inline NullLogHelperBase(Args && ...) noexcept {}

    }; /* class NullLogHelperBase { */

    template <Priority priority>
    class LogHelperBase {

        template <Priority> friend class LogHelper;

    public: /* Methods: */

        LogHelperBase(LogHelperBase<priority> const &) = delete;
        LogHelperBase<priority> & operator=(LogHelperBase<priority> const &) =
                delete;

        inline LogHelperBase(LogHelperBase<priority> && move) noexcept
            : m_time(std::move(move.m_time))
            , m_backend(std::move(move.m_backend))
            , m_offset(std::move(move.m_offset))
        {
            #if ! LOGHARD_HAVE_TLS
            using namespace ::LogHard::Detail;
            std::memcpy(tl_message, move.tl_message, STACK_BUFFER_SIZE);
            #endif
        }

        inline LogHelperBase<priority> & operator=(
                LogHelperBase<priority> const && move)
        {
            m_time = std::move(move.m_time);
            m_backend = std::move(move.m_backend);
            m_offset = std::move(move.m_offset);
            #if ! LOGHARD_HAVE_TLS
            using namespace ::LogHard::Detail;
            std::memcpy(tl_message, move.tl_message, STACK_BUFFER_SIZE);
            #endif
        }

        inline ~LogHelperBase() noexcept {
            if (!m_backend)
                return;
            using namespace ::LogHard::Detail;
            assert(m_offset <= STACK_BUFFER_SIZE);
            assert(m_offset < STACK_BUFFER_SIZE
                   || tl_message[STACK_BUFFER_SIZE - 1u] == '\0');
            if (m_offset < STACK_BUFFER_SIZE)
                tl_message[m_offset] = '\0';
            m_backend->doLog(std::move(m_time), priority, tl_message);
        }

        inline LogHelperBase & operator<<(char const v) noexcept {
            assert(m_backend);
            using namespace ::LogHard::Detail;
            if (m_offset <= MAX_MESSAGE_SIZE) {
                if (m_offset == MAX_MESSAGE_SIZE)
                    return elide();
                tl_message[m_offset] = v;
                m_offset++;
            }
            return *this;
        }

        inline LogHelperBase & operator<<(bool const v) noexcept
        { return this->operator<<(v ? '1' : '0'); }

#define LOGHARD_LHB_OP(valueType,valueGetter,formatString) \
    inline LogHelperBase & operator<<(valueType const v) noexcept { \
        assert(m_backend); \
        using namespace ::LogHard::Detail; \
        if (m_offset > MAX_MESSAGE_SIZE) { \
            assert(m_offset == STACK_BUFFER_SIZE); \
            return *this; \
        } \
        std::size_t const spaceLeft = MAX_MESSAGE_SIZE - m_offset; \
        if (!spaceLeft) \
            return elide(); \
        int const r = snprintf(&tl_message[m_offset], \
                               spaceLeft, \
                               (formatString), \
                               v valueGetter); \
        if (r < 0) \
            return elide(); \
        if (static_cast<std::size_t>(r) > spaceLeft) { \
            m_offset = MAX_MESSAGE_SIZE; \
            return elide(); \
        } \
        m_offset += static_cast<unsigned>(r); \
        return *this; \
    }

        LOGHARD_LHB_OP(signed char,, "%hhd")
        LOGHARD_LHB_OP(unsigned char,, "%hhu")
        LOGHARD_LHB_OP(short,, "%hd")
        LOGHARD_LHB_OP(unsigned short,, "%hu")
        LOGHARD_LHB_OP(int,, "%d")
        LOGHARD_LHB_OP(unsigned int,, "%u")
        LOGHARD_LHB_OP(long,, "%ld")
        LOGHARD_LHB_OP(unsigned long,, "%lu")
        LOGHARD_LHB_OP(long long,, "%lld")
        LOGHARD_LHB_OP(unsigned long long,, "%llu")

        LOGHARD_LHB_OP(Logger::Hex<unsigned char>,.value,"%hhx")
        LOGHARD_LHB_OP(Logger::Hex<unsigned short>,.value, "%hx")
        LOGHARD_LHB_OP(Logger::Hex<unsigned int>,.value, "%x")
        LOGHARD_LHB_OP(Logger::Hex<unsigned long>,.value, "%lx")
        LOGHARD_LHB_OP(Logger::Hex<unsigned long long>,.value,"%llx")

        LOGHARD_LHB_OP(Logger::HexByte,.value,"%02hhx")

        LOGHARD_LHB_OP(double,, "%f")
        LOGHARD_LHB_OP(long double,, "%Lf")

        inline LogHelperBase & operator<<(float const v) noexcept
        { return this->operator<<(static_cast<double const>(v)); }

        inline LogHelperBase & operator<<(char const * v) noexcept {
            assert(v);
            assert(m_backend);
            using namespace ::LogHard::Detail;
            auto o = m_offset;
            if (o > MAX_MESSAGE_SIZE) {
                assert(o == STACK_BUFFER_SIZE);
                return *this;
            }
            if (*v) {
                do {
                    if (o == MAX_MESSAGE_SIZE) {
                        m_offset = o;
                        return elide();
                    }
                    tl_message[o] = *v;
                } while ((++o, *++v));
                m_offset = o;
            }
            return *this;
        }

        inline LogHelperBase & operator<<(std::string const & v) noexcept
        { return this->operator<<(v.c_str()); }

        LOGHARD_LHB_OP(void *,, "%p")

        inline LogHelperBase & operator<<(void const * const v) noexcept
        { return this->operator<<(const_cast<void *>(v)); }

        inline LogHelperBase & operator<<(sharemind::Uuid const & v) noexcept {
#define LOGHARD_UUID_V(i) Logger::HexByte(v.data[i])
            return (*this)
                << LOGHARD_UUID_V(0u)  << LOGHARD_UUID_V(1u)
                << LOGHARD_UUID_V(2u)  << LOGHARD_UUID_V(3u) << '-'
                << LOGHARD_UUID_V(4u)  << LOGHARD_UUID_V(5u) << '-'
                << LOGHARD_UUID_V(6u)  << LOGHARD_UUID_V(7u) << '-'
                << LOGHARD_UUID_V(8u)  << LOGHARD_UUID_V(9u) << '-'
                << LOGHARD_UUID_V(10u) << LOGHARD_UUID_V(11u)
                << LOGHARD_UUID_V(12u) << LOGHARD_UUID_V(13u)
                << LOGHARD_UUID_V(14u) << LOGHARD_UUID_V(15u);
#undef LOGHARD_UUID_V
        }

#undef LOGHARD_LHB_OP

    private: /* Methods: */

        template <typename BackendPtr>
        inline LogHelperBase(::timeval theTime,
                             BackendPtr && backendPtr) noexcept
            : m_time(std::move(theTime))
            , m_backend(std::forward<BackendPtr>(backendPtr))
            , m_offset(0u)
        {}

        template <typename BackendPtr>
        inline LogHelperBase(::timeval theTime,
                             BackendPtr && backendPtr,
                             std::string const & prefix) noexcept
            : LogHelperBase(std::move(theTime),
                            std::forward<BackendPtr>(backendPtr),
                            prefix.c_str())
        {}

        template <typename BackendPtr>
        inline LogHelperBase(::timeval theTime,
                             BackendPtr && backendPtr,
                             char const * prefix) noexcept
            : m_time(std::move(theTime))
            , m_backend(std::forward<BackendPtr>(backendPtr))
        {
            using namespace ::LogHard::Detail;
            if (prefix && *prefix) {
                std::size_t o = 0u;
                do {
                    if (o == MAX_MESSAGE_SIZE)
                        break;
                    tl_message[o] = *prefix;
                } while ((++o, *++prefix));
                m_offset = o;
            } else {
                m_offset = 0u;
            }
        }

        inline LogHelperBase & elide() noexcept {
            using namespace ::LogHard::Detail;
            assert(m_offset <= MAX_MESSAGE_SIZE);
            assert(m_backend);
            std::memcpy(&tl_message[m_offset], "...", 4u);
            m_offset = STACK_BUFFER_SIZE;
            return *this;
        }

    private: /* Fields: */

        ::timeval m_time;
        std::shared_ptr<Backend> m_backend;
        std::size_t m_offset;
        #if ! LOGHARD_HAVE_TLS
        char tl_message[::LogHard::Detail::STACK_BUFFER_SIZE];
        #endif

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
        inline LogHelper(
                ::timeval theTime,
                BackendPtr && backend,
                Args && ... args) noexcept
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
                        std::exception_ptr e,
                        OutStream out) noexcept
        {
            assert(e);
            out << "  * Exception " << exceptionNumber << " of "
                << totalExceptions;
            try {
                std::rethrow_exception(e);
            } catch (std::exception const & e) {
                out << ": " << e.what();
            } catch (...) {
                out << " is not an std::exception!";
            }
        }

    };

public: /* Methods: */

    inline Logger(std::shared_ptr<Backend> backend)
            noexcept
        : m_backend(std::move(backend))
    {}

    template <typename Arg, typename ... Args>
    inline Logger(std::shared_ptr<Backend> backend,
                  Arg && arg,
                  Args && ... args) noexcept
        : m_backend(std::move(backend))
        , m_prefix(sharemind::concat(std::forward<Arg>(arg),
                                     std::forward<Args>(args)...,
                                     ' '))
    {}

    inline Logger(Logger const & copy) noexcept
        : m_backend(copy.m_backend)
        , m_prefix(copy.m_prefix)
        , m_basePrefix(copy.m_prefix)
    {}

    template <typename Arg, typename ... Args>
    inline Logger(Logger const & logger, Arg && arg, Args && ... args) noexcept
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

    inline std::shared_ptr<Backend> backend() const noexcept
    { return m_backend; }

    inline std::string const & prefix() const noexcept { return m_prefix; }

    inline std::string const & basePrefix() const noexcept { return m_prefix; }

    inline Backend::Lock retrieveBackendLock() const noexcept
    { return m_backend->retrieveLock(); }

    template <typename ... Args> void setPrefix(Args && ... args) {
        m_prefix = sharemind::concat(m_basePrefix,
                                     std::forward<Args>(args)...,
                                     ' ');
    }

    template <Priority PRIORITY, bool usePrefix = true>
    inline LogHelper<PRIORITY> logHelper() const noexcept
    { return logHelper<PRIORITY, usePrefix>(now()); }

    template <Priority PRIORITY, bool usePrefix = true>
    inline LogHelper<PRIORITY> logHelper(::timeval theTime) const noexcept {
        return usePrefix
               ? LogHelper<PRIORITY>(std::move(theTime), m_backend, m_prefix)
               : LogHelper<PRIORITY>(std::move(theTime), m_backend);
    }

    inline LogHelper<Priority::Fatal> fatal() const noexcept
    { return logHelper<Priority::Fatal>(); }

    inline LogHelper<Priority::Error> error() const noexcept
    { return logHelper<Priority::Error>(); }

    inline LogHelper<Priority::Warning> warning() const noexcept
    { return logHelper<Priority::Warning>(); }

    inline LogHelper<Priority::Normal> info() const noexcept
    { return logHelper<Priority::Normal>(); }

    inline LogHelper<Priority::Debug> debug() const noexcept
    { return logHelper<Priority::Debug>(); }

    inline LogHelper<Priority::FullDebug> fullDebug() const noexcept
    { return logHelper<Priority::FullDebug>(); }

    template <typename T>
    inline static Hex<T> hex(T const value) noexcept { return {value}; }

    template <Priority PRIORITY = Priority::Error>
    inline void printCurrentException() const noexcept
    { printCurrentException<PRIORITY>(now(), StandardFormatter()); }

    template <Priority PRIORITY = Priority::Error>
    inline void printCurrentException(::timeval theTime) const noexcept {
        printCurrentException<PRIORITY>(std::move(theTime),
                                        StandardFormatter());
    }

    template <Priority PRIORITY = Priority::Error, typename Formatter>
    inline void printCurrentException(Formatter && formatter) const noexcept {
        return printCurrentException<PRIORITY, Formatter>(
                       now(),
                       std::forward<Formatter>(formatter));
    }

    template <Priority PRIORITY = Priority::Error, typename Formatter>
    inline void printCurrentException(::timeval theTime, Formatter && formatter)
            const noexcept
    {
        auto const e(std::current_exception());
        if (!e)
            return;
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
        printException_(e, 1u, levels, printer);
    }

    inline static ::timeval now() noexcept {
        ::timeval theTime;
        SHAREMIND_DEBUG_ONLY(auto const r =) ::gettimeofday(&theTime, nullptr);
        assert(r == 0);
        return theTime;
    }

private: /* Methods: */

    template <typename Printer>
    inline void printException_(std::exception_ptr const e,
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
                       e);
    }

private: /* Fields: */

    std::shared_ptr<Backend> m_backend;
    std::string m_prefix;
    std::string const m_basePrefix;

}; /* class Logger { */

} /* namespace LogHard { */

#endif /* LOGHARD_LOGGER_H */
