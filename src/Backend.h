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
#include <memory>
#include <mutex>
#include <set>
#include <utility>
#include "Appender.h"
#include "Priority.h"


namespace LogHard {

class Logger;

class Backend {

    friend class Logger;

public: /* Types: */

    using Lock = std::unique_lock<std::recursive_mutex>;

    class Appender: public LogHard::Appender {

    public: /* Methods: */

        inline Appender(std::shared_ptr<Backend> backend)
            : m_backend(std::move(backend))
        {}

        /// \todo Check for backend loops.

        inline void log(::timeval time,
                        Priority const priority,
                        char const * message) noexcept override
        { m_backend->doLog(time, priority, message); }

    private: /* Fields: */

        std::shared_ptr<Backend> m_backend;

    }; /* class Appender */

public: /* Methods: */

    inline void addAppender(std::shared_ptr<LogHard::Appender> appenderPtr) {
        assert(appenderPtr);
        std::lock_guard<std::recursive_mutex> const guard(m_mutex);
        m_appenders.insert(appenderPtr);
    }

    inline void removeAppender(std::shared_ptr<LogHard::Appender> appenderPtr)
            noexcept
    {
        std::lock_guard<std::recursive_mutex> const guard(m_mutex);
        m_appenders.erase(appenderPtr);
    }

private: /* Methods: */

    inline Lock retrieveLock() noexcept { return Lock(m_mutex); }

    inline void doLog(::timeval const time,
                      Priority const priority,
                      char const * const message) noexcept
    {
        std::lock_guard<std::recursive_mutex> const guard(m_mutex);
        for (auto const & a : m_appenders)
            a->log(time, priority, message);
    }

private: /* Fields: */

    std::recursive_mutex m_mutex;
    std::set<std::shared_ptr<LogHard::Appender> > m_appenders;

}; /* class Backend { */

} /* namespace LogHard { */

#endif /* LOGHARD_BACKEND_H */
