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

#include "Backend.h"

#include <cassert>


namespace LogHard {

void Backend::addAppender(std::shared_ptr<LogHard::Appender> appenderPtr) {
    assert(appenderPtr);
    std::lock_guard<std::recursive_mutex> const guard(m_mutex);
    m_appenders.insert(appenderPtr);
}

void Backend::removeAppender(std::shared_ptr<LogHard::Appender> appenderPtr) noexcept
{
    std::lock_guard<std::recursive_mutex> const guard(m_mutex);
    m_appenders.erase(appenderPtr);
}

void Backend::doLog(::timeval const time,
                    Priority const priority,
                    char const * const message) noexcept
{
    std::lock_guard<std::recursive_mutex> const guard(m_mutex);
    for (auto const & a : m_appenders)
        a->log(time, priority, message);
}

} /* namespace LogHard { */