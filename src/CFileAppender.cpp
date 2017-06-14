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

#include "CFileAppender.h"

#include <cassert>
#include <cstddef>
#include <cstring>
#include <ctime>
#include <exception>
#include <sharemind/DebugOnly.h>
#include <sys/uio.h>
#include <unistd.h>


namespace LogHard {

namespace {

void logToFile_(int const fd,
                ::timeval time,
                Priority const priority,
                char const * const message) noexcept
{
    assert(fd != -1);
    assert(message);
    constexpr std::size_t bufSize = sizeof("YYYY.MM.DD HH:MM:SS");
    char timeStampBuf[bufSize];
    {
        std::tm eventTimeTm;
        {
            SHAREMIND_DEBUG_ONLY(auto const r =)
                    ::localtime_r(&time.tv_sec, &eventTimeTm);
            assert(r);
        }
        {
            SHAREMIND_DEBUG_ONLY(auto const r =)
                    std::strftime(timeStampBuf,
                                  bufSize,
                                  "%Y.%m.%d %H:%M:%S",
                                  &eventTimeTm);
            assert(r == bufSize - 1u);
        }
    }
    iovec const iov[] = {
        { const_cast<char *>(timeStampBuf), bufSize - 1u },
        { const_cast<char *>(" "), 1u },
        { const_cast<char *>(Appender::priorityStringRightPadded(priority)),
          7u },
        { const_cast<char *>(" "), 1u },
        { const_cast<char *>(message), std::strlen(message) },
        { const_cast<char *>("\n"), 1u }
    };
    #ifdef __GNUC__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-result"
    #endif
    (void) writev(fd, iov, sizeof(iov) / sizeof(iovec));
    #ifdef __GNUC__
    #pragma GCC diagnostic pop
    #endif
}

} // anonymous namespace

CFileAppender::CFileAppender(std::FILE * const file)
    : m_fd(::fileno(file))
{
    try {
        if (m_fd == -1)
            throw sharemind::ErrnoException(errno);
    } catch (...) {
        std::throw_with_nested(InvalidFileException());
    }
}

void CFileAppender::log(::timeval time,
                        Priority const priority,
                        char const * message) noexcept
{ logToFileSync(m_fd, time, priority, message); }

void CFileAppender::logToFile(int const fd,
                              ::timeval time,
                              Priority const priority,
                              char const * const message) noexcept
{ logToFile_(fd, time, priority, message); }

void CFileAppender::logToFileSync(int const fd,
                                  ::timeval time,
                                  Priority const priority,
                                  char const * const message) noexcept
{
    logToFile_(fd, time, priority, message);
    ::fsync(fd);
}

void CFileAppender::logToFile(std::FILE * file,
                              ::timeval time,
                              Priority const priority,
                              char const * const message) noexcept
{
    int const fd = ::fileno(file);
    assert(fd != -1);
    logToFile(fd, time, priority, message);
}

void CFileAppender::logToFileSync(std::FILE * file,
                                  ::timeval time,
                                  Priority const priority,
                                  char const * const message) noexcept
{
    int const fd = ::fileno(file);
    assert(fd != -1);
    logToFileSync(fd, time, priority, message);
}

} /* namespace LogHard { */
