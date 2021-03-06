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

#include "Logger.h"

#include <cstring>
#include <sharemind/DebugOnly.h>
#include <type_traits>


namespace LogHard {

namespace {

static constexpr std::size_t MAX_MESSAGE_SIZE = 1024u * 16u;
static_assert(MAX_MESSAGE_SIZE >= 1u, "Invalid MAX_MESSAGE_SIZE");
static constexpr std::size_t STACK_BUFFER_SIZE = MAX_MESSAGE_SIZE + 4u;
static_assert(STACK_BUFFER_SIZE > MAX_MESSAGE_SIZE, "Overflow");

thread_local char tl_message[STACK_BUFFER_SIZE] = {};
thread_local ::timeval tl_time = {};
thread_local std::size_t tl_offset = 0u;

} // anonymous namespace

Logger::MessageBuilder::MessageBuilder(Priority priority, Logger const & logger)
        noexcept
    : MessageBuilder(Logger::now(), priority, logger)
{}

Logger::MessageBuilder::MessageBuilder(::timeval theTime,
                                       Priority priority,
                                       Logger const & logger) noexcept
    : m_backend(sharemind::assertReturn(logger.backend()))
    , m_priority(priority)
{
    tl_time = std::move(theTime);
    auto const & prefix = logger.prefix();
    if (!prefix.empty()) {
        tl_offset = std::min(MAX_MESSAGE_SIZE, prefix.size());
        std::memcpy(tl_message, prefix.c_str(), tl_offset);
    } else {
        tl_offset = 0u;
    }
}

Logger::MessageBuilder::~MessageBuilder() noexcept {
    if (!m_backend)
        return;
    assert(tl_offset <= STACK_BUFFER_SIZE);
    assert(tl_offset < STACK_BUFFER_SIZE
           || tl_message[STACK_BUFFER_SIZE - 1u] == '\0');
    if (tl_offset < STACK_BUFFER_SIZE)
        tl_message[tl_offset] = '\0';
    m_backend->doLog(std::move(tl_time), m_priority, tl_message);
}

Logger::MessageBuilder &
Logger::MessageBuilder::operator<<(char const v) noexcept {
    assert(m_backend);
    if (tl_offset <= MAX_MESSAGE_SIZE) {
        if (tl_offset == MAX_MESSAGE_SIZE)
            return elide();
        tl_message[tl_offset] = v;
        tl_offset++;
    }
    return *this;
}

Logger::MessageBuilder &
Logger::MessageBuilder::operator<<(bool const v) noexcept
{ return this->operator<<(v ? '1' : '0'); }

#define LOGHARD_LHC_OP(valueType,valueGetter,formatString) \
    Logger::MessageBuilder & \
    Logger::MessageBuilder::operator<<(valueType const v) noexcept { \
        assert(m_backend); \
        if (tl_offset > MAX_MESSAGE_SIZE) { \
            assert(tl_offset == STACK_BUFFER_SIZE); \
            return *this; \
        } \
        std::size_t const spaceLeft = MAX_MESSAGE_SIZE - tl_offset; \
        if (!spaceLeft) \
            return elide(); \
        int const r = snprintf(&tl_message[tl_offset], \
                               spaceLeft, \
                               (formatString), \
                               v valueGetter); \
        if (r < 0) \
            return elide(); \
        if (static_cast<std::size_t>(r) > spaceLeft) { \
            tl_offset = MAX_MESSAGE_SIZE; \
            return elide(); \
        } \
        tl_offset += static_cast<unsigned>(r); \
        return *this; \
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

Logger::MessageBuilder &
Logger::MessageBuilder::operator<<(float const v) noexcept
{ return this->operator<<(static_cast<double>(v)); }

Logger::MessageBuilder &
Logger::MessageBuilder::operator<<(char const * v) noexcept {
    assert(v);
    assert(m_backend);
    auto o = tl_offset;
    if (o > MAX_MESSAGE_SIZE) {
        assert(o == STACK_BUFFER_SIZE);
        return *this;
    }
    if (*v) {
        do {
            if (o == MAX_MESSAGE_SIZE) {
                tl_offset = o;
                return elide();
            }
            tl_message[o] = *v;
            ++o;
        } while (*++v);
        tl_offset = o;
    }
    return *this;
}

Logger::MessageBuilder &
Logger::MessageBuilder::operator<<(std::string const & v) noexcept {
    assert(m_backend);
    auto const s = v.size();
    if (s <= 0u)
        return *this;
    if (tl_offset <= MAX_MESSAGE_SIZE) {
        auto const freeSpace = MAX_MESSAGE_SIZE - tl_offset;
        if (freeSpace == 0u)
            return elide();
        if (s <= freeSpace) {
            std::memcpy(&tl_message[tl_offset], v.c_str(), s);
            tl_offset += s;
        } else {
            std::memcpy(&tl_message[tl_offset], v.c_str(), freeSpace);
            return elide();
        }
    }
    return *this;
}

LOGHARD_LHC_OP(void *,, "%p")

Logger::MessageBuilder &
Logger::MessageBuilder::operator<<(void const * const v) noexcept
{ return this->operator<<(const_cast<void *>(v)); }

Logger::MessageBuilder &
Logger::MessageBuilder::operator<<(sharemind::Uuid const & v) noexcept {
    #define LOGHARD_UUID_V(i) Logger::HexByte{v.data[i]}
    return this->operator<<(LOGHARD_UUID_V(0u))
           << LOGHARD_UUID_V(1u) << LOGHARD_UUID_V(2u) << LOGHARD_UUID_V(3u)
           << '-' << LOGHARD_UUID_V(4u) << LOGHARD_UUID_V(5u)
           << '-' << LOGHARD_UUID_V(6u) << LOGHARD_UUID_V(7u)
           << '-' << LOGHARD_UUID_V(8u) << LOGHARD_UUID_V(9u)
           << '-' << LOGHARD_UUID_V(10u) << LOGHARD_UUID_V(11u)
           << LOGHARD_UUID_V(12u) << LOGHARD_UUID_V(13u)
           << LOGHARD_UUID_V(14u) << LOGHARD_UUID_V(15u);
    #undef LOGHARD_UUID_V
}

Logger::MessageBuilder & Logger::MessageBuilder::elide() noexcept {
    assert(tl_offset <= MAX_MESSAGE_SIZE);
    assert(m_backend);
    std::memcpy(&tl_message[tl_offset], "...", 4u);
    tl_offset = STACK_BUFFER_SIZE;
    return *this;
}


void Logger::StandardExceptionFormatter::operator()(
                                      std::size_t const exceptionNumber,
                                      std::size_t const totalExceptions,
                                      std::exception_ptr exception,
                                      MessageBuilder mb) const noexcept
{
    assert(exception);
    for (std::size_t i = m_extraIndent; i > 0u; --i)
        mb << ' ';
    mb << "  * Exception " << exceptionNumber << " of " << totalExceptions;
    try {
        std::rethrow_exception(std::move(exception));
    } catch (std::exception const & e) {
        mb << ": " << e.what();
    } catch (...) {
        mb << " is not an std::exception!";
    }
}

Logger::Logger(std::shared_ptr<Backend> backend) noexcept
    : m_backend(sharemind::assertReturn(std::move(backend)))
{}

Logger::Logger(Logger && move) noexcept
    : m_backend(sharemind::assertReturn(std::move(move.m_backend)))
    , m_prefix(std::move(move.m_prefix))
    , m_basePrefix(std::move(move.m_prefix))
{}

Logger::Logger(Logger const & copy) noexcept
    : m_backend(sharemind::assertReturn(copy.m_backend))
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

Logger::MessageBuilder Logger::fatal() const noexcept
{ return MessageBuilder(Priority::Fatal, *this); }

Logger::MessageBuilder Logger::error() const noexcept
{ return MessageBuilder(Priority::Error, *this); }

Logger::MessageBuilder Logger::warning() const noexcept
{ return MessageBuilder(Priority::Warning, *this); }

Logger::MessageBuilder Logger::info() const noexcept
{ return MessageBuilder(Priority::Normal, *this); }

Logger::MessageBuilder Logger::debug() const noexcept
{ return MessageBuilder(Priority::Debug, *this); }

Logger::MessageBuilder Logger::fullDebug() const noexcept
{ return MessageBuilder(Priority::FullDebug, *this); }

Logger::MessageBuilder Logger::fatal(::timeval t) const noexcept
{ return MessageBuilder(std::move(t), Priority::Fatal, *this); }

Logger::MessageBuilder Logger::error(::timeval t) const noexcept
{ return MessageBuilder(std::move(t), Priority::Error, *this); }

Logger::MessageBuilder Logger::warning(::timeval t) const noexcept
{ return MessageBuilder(std::move(t), Priority::Warning, *this); }

Logger::MessageBuilder Logger::info(::timeval t) const noexcept
{ return MessageBuilder(std::move(t), Priority::Normal, *this); }

Logger::MessageBuilder Logger::debug(::timeval t) const noexcept
{ return MessageBuilder(std::move(t), Priority::Debug, *this); }

Logger::MessageBuilder Logger::fullDebug(::timeval t) const noexcept
{ return MessageBuilder(std::move(t), Priority::FullDebug, *this); }

// Extern template instantiations:

#define LOGHARD_TCN(...) template __VA_ARGS__ const noexcept;
#define LOGHARD_EXTERN(pri) \
    LOGHARD_TCN(void Logger::printCurrentException<Priority::pri>()) \
    LOGHARD_TCN(void Logger::printCurrentException<Priority::pri>(::timeval)) \
    LOGHARD_TCN( \
        void Logger::printCurrentException<Priority::pri, \
                                           Logger::StandardExceptionFormatter>(\
                StandardExceptionFormatter &&)) \
    LOGHARD_TCN( \
        void Logger::printCurrentException<Priority::pri, \
                                           Logger::StandardExceptionFormatter>(\
                ::timeval, StandardExceptionFormatter &&))

LOGHARD_EXTERN(Fatal)
LOGHARD_EXTERN(Error)
LOGHARD_EXTERN(Warning)
LOGHARD_EXTERN(Normal)
LOGHARD_EXTERN(Debug)
LOGHARD_EXTERN(FullDebug)

} // namespace LogHard {
