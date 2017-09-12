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

#ifndef LOGHARD_FILEAPPENDER_H
#define LOGHARD_FILEAPPENDER_H

#include "Appender.h"

#include <exception>
#include <fcntl.h>
#include <sharemind/Exception.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


namespace LogHard {

class FileAppender: public Appender {

public: /* Types: */

    enum OpenMode { APPEND, OVERWRITE };

    SHAREMIND_DECLARE_EXCEPTION_NOINLINE(std::exception, Exception);
    SHAREMIND_DECLARE_EXCEPTION_CONST_MSG_NOINLINE(Exception,
                                                   FileOpenException);

public: /* Methods: */

    FileAppender(std::string const & path,
                 OpenMode const openMode,
                 ::mode_t const flags = 0644);

    FileAppender(char const * const path,
                 OpenMode const openMode,
                 ::mode_t const flags = 0644);

    ~FileAppender() noexcept override;

private: /* Methods: */

    void doLog(::timeval time,
               Priority const priority,
               char const * message) noexcept override;

private: /* Fields: */

    int const m_fd;

}; /* class FileAppender */

} /* namespace LogHard { */

#endif /* LOGHARD_FILEAPPENDER_H */
