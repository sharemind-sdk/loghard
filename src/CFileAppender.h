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

#ifndef LOGHARD_CFILEAPPENDER_H
#define LOGHARD_CFILEAPPENDER_H

#include "Appender.h"

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <exception>
#include <sharemind/DebugOnly.h>
#include <sharemind/Exception.h>
#include <sys/uio.h>
#include <unistd.h>


namespace LogHard {

class CFileAppender: public Appender {

public: /* Types: */

    SHAREMIND_DEFINE_EXCEPTION(std::exception, Exception);
    SHAREMIND_DEFINE_EXCEPTION_CONST_MSG(
            Exception,
            InvalidFileException,
            "Invalid FILE handle given for logging!");

public: /* Methods: */

    CFileAppender(std::FILE * const file)
        : m_fd(::fileno(file))
    {
        try {
            if (m_fd == -1)
                throw sharemind::ErrnoException(errno);
        } catch (...) {
            std::throw_with_nested(InvalidFileException());
        }
    }

    void log(::timeval time,
             Priority const priority,
             char const * message) noexcept override
    { logToFileSync(m_fd, time, priority, message); }

    static void logToFile(int const fd,
                          ::timeval time,
                          Priority const priority,
                          char const * const message) noexcept
    { logToFile_(fd, time, priority, message, [](int const){}); }

    static void logToFileSync(int const fd,
                              ::timeval time,
                              Priority const priority,
                              char const * const message) noexcept
    {
        logToFile_(fd, time, priority, message,
                   [](int const f){ ::fsync(f); });
    }

    static void logToFile(std::FILE * file,
                          ::timeval time,
                          Priority const priority,
                          char const * const message) noexcept
    {
        int const fd = ::fileno(file);
        assert(fd != -1);
        logToFile(fd, time, priority, message);
    }

    static void logToFileSync(std::FILE * file,
                              ::timeval time,
                              Priority const priority,
                              char const * const message) noexcept
    {
        int const fd = ::fileno(file);
        assert(fd != -1);
        logToFileSync(fd, time, priority, message);
    }

private: /* Methods: */

    template <typename Sync>
    static void logToFile_(int const fd,
                           ::timeval time,
                           Priority const priority,
                           char const * const message,
                           Sync && sync) noexcept
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
            { const_cast<char *>(priorityStringRightPadded(priority)), 7u },
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
        sync(fd);
    }

private: /* Fields: */

    int const m_fd;

}; /* class CFileAppender { */

} /* namespace LogHard { */

#endif /* LOGHARD_CFILEAPPENDER_H */
