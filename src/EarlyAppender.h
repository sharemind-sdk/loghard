/*
 * Copyright (C) 2015-2017 Cybernetica
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

#ifndef LOGHARD_EARLYAPPENDER_H
#define LOGHARD_EARLYAPPENDER_H

#include "Appender.h"

#include <cstddef>
#include <exception>
#include <sharemind/Exception.h>
#include <string>
#include <vector>


namespace LogHard {

class EarlyAppender: public Appender {

public: /* Types: */

    SHAREMIND_DECLARE_EXCEPTION_NOINLINE(std::exception, Exception);
    SHAREMIND_DECLARE_EXCEPTION_CONST_MSG_NOINLINE(Exception,
                                                   TooManyEntriesException);

    struct LogEntry {
        ::timeval time;
        Priority priority;
        std::string message;
    };
    using LogEntries = std::vector<LogEntry>;

public: /* Methods: */

    EarlyAppender(std::size_t const reserveEntries = 1024u,
                  std::size_t const maxMessageSize = 1024u);
    ~EarlyAppender() noexcept override;

    LogEntries const & entries() const noexcept { return m_entries; }

    void log(::timeval time,
             Priority const priority,
             char const * message) noexcept override;

    void logToAppender(Appender & appender) const noexcept;

    void clear() noexcept;

private: /* Fields: */

    LogEntries m_entries;
    std::vector<std::string> m_freeMessages;
    std::string m_oomMessage;
    bool m_oom = false;
    #ifndef NDEBUG
    std::size_t const m_maxMessageSize;
    #endif

}; /* class EarlyAppender */

} /* namespace LogHard { */

#endif /* LOGHARD_EARLYAPPENDER_H */
