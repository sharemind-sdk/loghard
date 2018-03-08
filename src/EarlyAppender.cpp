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


#include "EarlyAppender.h"

#include <cassert>


namespace LogHard {

SHAREMIND_DEFINE_EXCEPTION_NOINLINE(LogHard::Exception,
                                    EarlyAppender::,
                                    Exception);
SHAREMIND_DEFINE_EXCEPTION_CONST_MSG_NOINLINE(
        EarlyAppender::Exception,
        EarlyAppender::,
        TooManyEntriesException,
        "Maximum log entry reservation size exceeded!");

EarlyAppender::EarlyAppender(std::size_t const reserveEntries,
                             std::size_t const maxMessageSize)
    : m_oomMessage("Early log buffer full, messages skipped!")
    #ifndef NDEBUG
    , m_maxMessageSize((assert(maxMessageSize >= 5u), maxMessageSize))
    #endif
{
    std::size_t const size = reserveEntries + 1u;
    if (size < reserveEntries)
        throw TooManyEntriesException();
    m_entries.reserve(size);
    m_freeMessages.resize(reserveEntries);
    for (std::string & msg : m_freeMessages)
        msg.reserve(maxMessageSize);
}

EarlyAppender::~EarlyAppender() noexcept {}

void EarlyAppender::logToAppender(Appender & appender) const noexcept {
    for (LogEntry const & entry : m_entries)
        appender.log(entry.time, entry.priority, entry.message.c_str());
}

void EarlyAppender::logToAppender(Appender & appender,
                                  Priority const priority) const noexcept
{
    for (LogEntry const & entry : m_entries)
        if (entry.priority <= priority)
            appender.log(entry.time, entry.priority, entry.message.c_str());
}

void EarlyAppender::clear() noexcept {
    if (m_oom)
        m_oomMessage = std::move(m_entries.back().message);
    m_entries.pop_back();
    for (LogEntry & entry : m_entries)
        m_freeMessages.emplace_back(std::move(entry.message));
    m_entries.clear();
}

void EarlyAppender::doLog(::timeval time,
                          Priority const priority,
                          char const * message) noexcept
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
            std::size_t const size = s.size();
            assert(size > m_maxMessageSize);
            static char const elide[] = "[...]";
            s.replace(size - 5u, 5u, elide, 5u);
        }
        m_entries.emplace_back(LogEntry{time, priority, std::move(s)});
        m_freeMessages.pop_back();
    }
}

} /* namespace LogHard { */
