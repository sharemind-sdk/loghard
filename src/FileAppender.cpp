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

#include "FileAppender.h"

#include "CFileAppender.h"


namespace LogHard {

SHAREMIND_DEFINE_EXCEPTION_NOINLINE(std::exception, FileAppender::, Exception);
SHAREMIND_DEFINE_EXCEPTION_CONST_MSG_NOINLINE(
        FileAppender::Exception,
        FileAppender::,
        FileOpenException,
        "Failed to open file for logging!");

FileAppender::FileAppender(std::string const & path,
                           OpenMode const openMode,
                           ::mode_t const flags)
    : FileAppender(path.c_str(),
                   openMode,
                   flags)
{}

FileAppender::FileAppender(char const * const path,
                           OpenMode const openMode,
                           ::mode_t const flags)
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

FileAppender::~FileAppender() noexcept { ::close(m_fd); }

void FileAppender::doLog(::timeval time,
                         Priority const priority,
                         char const * message) noexcept
{ CFileAppender::logToFile(m_fd, time, priority, message); }

} /* namespace LogHard { */
