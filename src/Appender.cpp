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

#include "Appender.h"


namespace LogHard {

Appender::Appender() noexcept {}

Appender::Appender(Priority const priority) noexcept
    : m_priority(priority)
{}

Appender::~Appender() noexcept {}

void Appender::log(::timeval time,
                   Priority priority,
                   char const * message) noexcept
{
    if (priority <= m_priority.load(std::memory_order_relaxed))
        doLog(time, priority, message);
}

char const * Appender::priorityString(Priority const priority) noexcept
{
    static char const strings[][8u] =
            { "FATAL", "ERROR", "WARNING", "INFO", "DEBUG", "DEBUG2" };
    return &strings[static_cast<unsigned>(priority)][0u];
}

char const * Appender::priorityStringRightPadded(Priority const priority)
        noexcept
{
    static char const strings[][8u] = {
        "FATAL  ", "ERROR  ", "WARNING", "INFO   ", "DEBUG  ", "DEBUG2 "
    };
    return &strings[static_cast<unsigned>(priority)][0u];
}

} /* namespace LogHard { */
