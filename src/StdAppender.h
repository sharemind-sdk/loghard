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

#ifndef LOGHARD_STDAPPENDER_H
#define LOGHARD_STDAPPENDER_H

#include "Appender.h"

#include <unistd.h>
#include "CFileAppender.h"


namespace LogHard {

class StdAppender: public Appender {

public: /* Methods: */

    void log(::timeval time,
             Priority const priority,
             char const * message) noexcept override
    {
        int const fn = (priority <= Priority::Warning)
                        ? STDERR_FILENO
                        : STDOUT_FILENO;
        CFileAppender::logToFile(fn, time, priority, message);
    }

}; /* class StdAppender */

} /* namespace LogHard { */

#endif /* LOGHARD_STDAPPENDER_H */
