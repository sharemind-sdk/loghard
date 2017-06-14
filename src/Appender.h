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

#ifndef LOGHARD_APPENDER_H
#define LOGHARD_APPENDER_H

#include <sys/time.h>
#include "Priority.h"


namespace LogHard {

class Appender {

public: /* Methods: */

    virtual ~Appender() noexcept {}

    virtual void log(::timeval time,
                     Priority priority,
                     char const * message) noexcept = 0;

    static char const * priorityString(Priority const priority)
            noexcept
    {
        static char const strings[][8u] =
                { "FATAL", "ERROR", "WARNING", "INFO", "DEBUG", "DEBUG2" };
        return &strings[static_cast<unsigned>(priority)][0u];
    }

    static char const * priorityStringRightPadded(Priority const priority)
            noexcept
    {
        static char const strings[][8u] = {
            "FATAL  ", "ERROR  ", "WARNING", "INFO   ", "DEBUG  ", "DEBUG2 "
        };
        return &strings[static_cast<unsigned>(priority)][0u];
    }

}; /* class Appender { */

} /* namespace LogHard { */

#endif /* LOGHARD_APPENDER_H */
