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

#ifndef LOGHARD_SYSLOGAPPENDER_H
#define LOGHARD_SYSLOGAPPENDER_H

#include "Appender.h"

#include <cassert>
#include <exception>
#include <sharemind/Exception.h>
#include <string>
#include <syslog.h>
#include <utility>


namespace LogHard {

class SyslogAppender: public Appender {

public: /* Types: */

    SHAREMIND_DEFINE_EXCEPTION(std::exception, Exception);
    SHAREMIND_DEFINE_EXCEPTION_CONST_MSG(
            Exception,
            MultipleSyslogAppenderException,
            "Multiple Syslog active appenders per process not allowed!");

public: /* Methods: */

    template <typename Ident>
    SyslogAppender(Ident && ident,
                   int const logopt,
                   int const facility)
        : m_ident{std::forward<Ident>(ident)}
        , m_logopt{logopt}
        , m_facility{facility}
    { setEnabled_(true); }

    ~SyslogAppender() noexcept override { setEnabled_(false); }

    void log(::timeval,
             Priority const priority,
             char const * message) noexcept override
    {
        constexpr static int priorities[] =
            { LOG_EMERG, LOG_ERR, LOG_WARNING, LOG_INFO, LOG_DEBUG, LOG_DEBUG };
        ::syslog(priorities[static_cast<unsigned>(priority)], "%s", message);
    }

private: /* Methods: */

    void setEnabled_(bool const enable) const {
        static SyslogAppender const * singleInstance = nullptr;
        if (enable) {
            if (singleInstance)
                throw MultipleSyslogAppenderException{};
            singleInstance = this;
            ::openlog(m_ident.c_str(), m_logopt, m_facility);
        } else {
            assert(singleInstance == this);
            ::closelog();
            singleInstance = nullptr;
        }
    }

private: /* Fields: */

    std::string const m_ident;
    int const m_logopt;
    int const m_facility;

}; /* class SyslogAppender { */

} /* namespace LogHard { */

#endif /* LOGHARD_SYSLOGAPPENDER_H */
