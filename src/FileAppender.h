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

#ifndef LOGHARD_FILEAPPENDER_H
#define LOGHARD_FILEAPPENDER_H

#include "Appender.h"

#include <exception>
#include <fcntl.h>
#include <sharemind/Exception.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include "CFileAppender.h"


namespace LogHard {

class FileAppender: public Appender {

public: /* Types: */

    enum OpenMode { APPEND, OVERWRITE };

    SHAREMIND_DEFINE_EXCEPTION(std::exception, Exception);
    SHAREMIND_DEFINE_EXCEPTION_CONST_MSG(
            Exception,
            FileOpenException,
            "Failed to open file for logging!");

public: /* Methods: */

    FileAppender(std::string const & path,
                 OpenMode const openMode,
                 ::mode_t const flags = 0644)
        : FileAppender(path.c_str(),
                       openMode,
                       flags)
    {}

    FileAppender(char const * const path,
                 OpenMode const openMode,
                 ::mode_t const flags = 0644)
        : m_fd(::open(path,
                    // No O_SYNC since it would hurt performance badly
                    O_WRONLY | O_CREAT | O_APPEND | O_NOCTTY
                    | ((openMode == OVERWRITE) ? O_TRUNC : 0u),
                    flags))
    {
        try {
            if (m_fd == -1)
                throw sharemind::ErrnoException(errno);
        } catch (...) {
            std::throw_with_nested(FileOpenException());
        }
    }

    ~FileAppender() noexcept override { ::close(m_fd); }

    void log(::timeval time,
             Priority const priority,
             char const * message) noexcept override
    { CFileAppender::logToFile(m_fd, time, priority, message); }

private: /* Fields: */

    int const m_fd;

}; /* class FileAppender */

} /* namespace LogHard { */

#endif /* LOGHARD_FILEAPPENDER_H */
