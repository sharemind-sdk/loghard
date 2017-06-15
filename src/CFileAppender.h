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

#ifndef LOGHARD_CFILEAPPENDER_H
#define LOGHARD_CFILEAPPENDER_H

#include "Appender.h"

#include <cstdio>
#include <sharemind/Exception.h>


namespace LogHard {

class CFileAppender: public Appender {

public: /* Types: */

    SHAREMIND_DECLARE_EXCEPTION_NOINLINE(std::exception, Exception);
    SHAREMIND_DECLARE_EXCEPTION_CONST_MSG_NOINLINE(Exception,
                                                   InvalidFileException);

public: /* Methods: */

    CFileAppender(std::FILE * const file);
    ~CFileAppender() noexcept override;

    void log(::timeval time,
             Priority const priority,
             char const * message) noexcept override;

    static void logToFile(int const fd,
                          ::timeval time,
                          Priority const priority,
                          char const * const message) noexcept;

    static void logToFileSync(int const fd,
                              ::timeval time,
                              Priority const priority,
                              char const * const message) noexcept;

    static void logToFile(std::FILE * file,
                          ::timeval time,
                          Priority const priority,
                          char const * const message) noexcept;

    static void logToFileSync(std::FILE * file,
                              ::timeval time,
                              Priority const priority,
                              char const * const message) noexcept;

private: /* Fields: */

    int const m_fd;

}; /* class CFileAppender { */

} /* namespace LogHard { */

#endif /* LOGHARD_CFILEAPPENDER_H */
