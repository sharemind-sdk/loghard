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

#include "Logger.h"

#include <cstring>
#include <sharemind/DebugOnly.h>


namespace LogHard {


namespace Detail {

static constexpr std::size_t MAX_MESSAGE_SIZE = 1024u * 16u;
static_assert(MAX_MESSAGE_SIZE >= 1u, "Invalid MAX_MESSAGE_SIZE");
static constexpr std::size_t STACK_BUFFER_SIZE = MAX_MESSAGE_SIZE + 4u;
static_assert(STACK_BUFFER_SIZE > MAX_MESSAGE_SIZE, "Overflow");

thread_local char tl_message[STACK_BUFFER_SIZE] = {};
thread_local ::timeval tl_time = {};
thread_local std::size_t tl_offset = 0u;

} // namespace Detail {

Logger::LogHelperContents::LogHelperContents(
        ::timeval theTime,
        std::shared_ptr<Backend> backendPtr) noexcept
    : m_backend(std::move(backendPtr))
{
    Detail::tl_time = std::move(theTime);
    Detail::tl_offset = 0u;
}

Logger::LogHelperContents::LogHelperContents(
        ::timeval theTime,
        std::shared_ptr<Backend> backendPtr,
        std::string const & prefix) noexcept
    : LogHelperContents(std::move(theTime),
                    std::move(backendPtr),
                    prefix.c_str())
{}

Logger::LogHelperContents::LogHelperContents(
        ::timeval theTime,
        std::shared_ptr<Backend> backendPtr,
        char const * prefix) noexcept
    : m_backend(std::move(backendPtr))
{
    Detail::tl_time = std::move(theTime);
    using namespace ::LogHard::Detail;
    if (prefix && *prefix) {
        std::size_t o = 0u;
        do {
            Detail::tl_message[o] = *prefix;
        } while ((++o < MAX_MESSAGE_SIZE) && (*++prefix));
        Detail::tl_offset = o;
    } else {
        Detail::tl_offset = 0u;
    }
}

void Logger::LogHelperContents::finish(Priority priority) noexcept {
    if (!m_backend)
        return;
    using namespace ::LogHard::Detail;
    assert(Detail::tl_offset <= STACK_BUFFER_SIZE);
    assert(Detail::tl_offset < STACK_BUFFER_SIZE
           || Detail::tl_message[STACK_BUFFER_SIZE - 1u] == '\0');
    if (Detail::tl_offset < STACK_BUFFER_SIZE)
        Detail::tl_message[Detail::tl_offset] = '\0';
    m_backend->doLog(std::move(Detail::tl_time), priority, Detail::tl_message);
}

void Logger::LogHelperContents::log(char const v) noexcept {
    assert(m_backend);
    using namespace ::LogHard::Detail;
    if (Detail::tl_offset <= MAX_MESSAGE_SIZE) {
        if (Detail::tl_offset == MAX_MESSAGE_SIZE)
            return elide();
        Detail::tl_message[Detail::tl_offset] = v;
        Detail::tl_offset++;
    }
}

void Logger::LogHelperContents::log(bool const v) noexcept
{ return log(v ? '1' : '0'); }

#define LOGHARD_LHC_OP(valueType,valueGetter,formatString) \
    void Logger::LogHelperContents::log(valueType const v) noexcept { \
        assert(m_backend); \
        using namespace ::LogHard::Detail; \
        if (Detail::tl_offset > MAX_MESSAGE_SIZE) { \
            assert(Detail::tl_offset == STACK_BUFFER_SIZE); \
            return; \
        } \
        std::size_t const spaceLeft = MAX_MESSAGE_SIZE - Detail::tl_offset; \
        if (!spaceLeft) \
            return elide(); \
        int const r = snprintf(&Detail::tl_message[Detail::tl_offset], \
                               spaceLeft, \
                               (formatString), \
                               v valueGetter); \
        if (r < 0) \
            return elide(); \
        if (static_cast<std::size_t>(r) > spaceLeft) { \
            Detail::tl_offset = MAX_MESSAGE_SIZE; \
            return elide(); \
        } \
        Detail::tl_offset += static_cast<unsigned>(r); \
    }

LOGHARD_LHC_OP(signed char,, "%hhd")
LOGHARD_LHC_OP(unsigned char,, "%hhu")
LOGHARD_LHC_OP(short,, "%hd")
LOGHARD_LHC_OP(unsigned short,, "%hu")
LOGHARD_LHC_OP(int,, "%d")
LOGHARD_LHC_OP(unsigned int,, "%u")
LOGHARD_LHC_OP(long,, "%ld")
LOGHARD_LHC_OP(unsigned long,, "%lu")
LOGHARD_LHC_OP(long long,, "%lld")
LOGHARD_LHC_OP(unsigned long long,, "%llu")

LOGHARD_LHC_OP(Logger::Hex<unsigned char>,.value,"%hhx")
LOGHARD_LHC_OP(Logger::Hex<unsigned short>,.value, "%hx")
LOGHARD_LHC_OP(Logger::Hex<unsigned int>,.value, "%x")
LOGHARD_LHC_OP(Logger::Hex<unsigned long>,.value, "%lx")
LOGHARD_LHC_OP(Logger::Hex<unsigned long long>,.value,"%llx")

LOGHARD_LHC_OP(Logger::HexByte,.value,"%02hhx")

LOGHARD_LHC_OP(double,, "%f")
LOGHARD_LHC_OP(long double,, "%Lf")

void Logger::LogHelperContents::log(float const v) noexcept
{ return log(static_cast<double const>(v)); }

void Logger::LogHelperContents::log(char const * v) noexcept {
    assert(v);
    assert(m_backend);
    using namespace ::LogHard::Detail;
    auto o = Detail::tl_offset;
    if (o > MAX_MESSAGE_SIZE) {
        assert(o == STACK_BUFFER_SIZE);
        return;
    }
    if (*v) {
        do {
            if (o == MAX_MESSAGE_SIZE) {
                Detail::tl_offset = o;
                return elide();
            }
            Detail::tl_message[o] = *v;
        } while ((++o, *++v));
        Detail::tl_offset = o;
    }
}

void Logger::LogHelperContents::log(std::string const & v) noexcept
{ return log(v.c_str()); }

LOGHARD_LHC_OP(void *,, "%p")

void Logger::LogHelperContents::log(void const * const v) noexcept
{ return log(const_cast<void *>(v)); }

void Logger::LogHelperContents::log(sharemind::Uuid const & v) noexcept {
#define LOGHARD_UUID_V(i) Logger::HexByte{v.data[i]}
    log(LOGHARD_UUID_V(0u)); log(LOGHARD_UUID_V(1u));
    log(LOGHARD_UUID_V(2u)); log(LOGHARD_UUID_V(3u)); log('-');
    log(LOGHARD_UUID_V(4u)); log(LOGHARD_UUID_V(5u)); log('-');
    log(LOGHARD_UUID_V(6u)); log(LOGHARD_UUID_V(7u)); log('-');
    log(LOGHARD_UUID_V(8u)); log(LOGHARD_UUID_V(9u)); log('-');
    log(LOGHARD_UUID_V(10u)); log(LOGHARD_UUID_V(11u));
    log(LOGHARD_UUID_V(12u)); log(LOGHARD_UUID_V(13u));
    log(LOGHARD_UUID_V(14u)); log(LOGHARD_UUID_V(15u));
}

void Logger::LogHelperContents::elide() noexcept {
    using namespace ::LogHard::Detail;
    assert(Detail::tl_offset <= MAX_MESSAGE_SIZE);
    assert(m_backend);
    std::memcpy(&Detail::tl_message[Detail::tl_offset], "...", 4u);
    Detail::tl_offset = STACK_BUFFER_SIZE;
}

Logger::Logger(std::shared_ptr<Backend> backend) noexcept
    : m_backend(std::move(backend))
{}

Logger::Logger(Logger && move) noexcept
    : m_backend(std::move(move.m_backend))
    , m_prefix(std::move(move.m_prefix))
    , m_basePrefix(std::move(move.m_prefix))
{}

Logger::Logger(Logger const & copy) noexcept
    : m_backend(copy.m_backend)
    , m_prefix(copy.m_prefix)
    , m_basePrefix(copy.m_prefix)
{}

Logger::~Logger() noexcept {}

::timeval Logger::now() noexcept {
    ::timeval theTime;
    SHAREMIND_DEBUG_ONLY(auto const r =) ::gettimeofday(&theTime, nullptr);
    assert(r == 0);
    return theTime;
}

Logger::LogHelper<Priority::Fatal> Logger::fatal() const noexcept
{ return logHelper<Priority::Fatal>(); }

Logger::LogHelper<Priority::Error> Logger::error() const noexcept
{ return logHelper<Priority::Error>(); }

Logger::LogHelper<Priority::Warning> Logger::warning() const noexcept
{ return logHelper<Priority::Warning>(); }

Logger::LogHelper<Priority::Normal> Logger::info() const noexcept
{ return logHelper<Priority::Normal>(); }

Logger::LogHelper<Priority::Debug> Logger::debug() const noexcept
{ return logHelper<Priority::Debug>(); }

Logger::LogHelper<Priority::FullDebug> Logger::fullDebug() const noexcept
{ return logHelper<Priority::FullDebug>(); }

// Extern template instantiations:

#define LOGHARD_TCN(...) template __VA_ARGS__ const noexcept;
#define LOGHARD_EXTERN_LH(pri,usePrefix,...) \
    LOGHARD_TCN(Logger::LogHelper<Priority::pri> \
                Logger::logHelper<Priority::pri, usePrefix>(__VA_ARGS__))
#define LOGHARD_EXTERN(pri) \
    template class Logger::LogHelperBase<Priority::pri>; \
    template class Logger::LogHelper<Priority::pri>; \
    LOGHARD_TCN( \
        void Logger::StandardFormatter::operator()( \
                std::size_t const, \
                std::size_t const, \
                std::exception_ptr, \
                Logger::LogHelper<Priority::pri>)) \
    LOGHARD_EXTERN_LH(pri, true,) \
    LOGHARD_EXTERN_LH(pri, false,) \
    LOGHARD_EXTERN_LH(pri, true, ::timeval) \
    LOGHARD_EXTERN_LH(pri, false, ::timeval) \
    LOGHARD_TCN(void Logger::printCurrentException<Priority::pri>()) \
    LOGHARD_TCN(void Logger::printCurrentException<Priority::pri>(::timeval)) \
    LOGHARD_TCN( \
        void Logger::printCurrentException<Priority::pri, Logger::StandardFormatter>( \
                StandardFormatter &&)) \
    LOGHARD_TCN( \
        void Logger::printCurrentException<Priority::pri, Logger::StandardFormatter>( \
                ::timeval, StandardFormatter &&))

LOGHARD_EXTERN(Fatal)
LOGHARD_EXTERN(Error)
LOGHARD_EXTERN(Warning)
LOGHARD_EXTERN(Normal)
LOGHARD_EXTERN(Debug)
LOGHARD_EXTERN(FullDebug)

} // namespace LogHard {
