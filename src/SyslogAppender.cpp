/*
 * Copyright (C) Cybernetica
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

#include "SyslogAppender.h"

#include <cassert>
#include <syslog.h>
#include <utility>


namespace LogHard {

SHAREMIND_DEFINE_EXCEPTION_NOINLINE(LogHard::Exception,
                                    SyslogAppender::,
                                    Exception);
SHAREMIND_DEFINE_EXCEPTION_CONST_MSG_NOINLINE(
        SyslogAppender::Exception,
        SyslogAppender::,
        MultipleSyslogAppenderException,
        "Multiple Syslog active appenders per process not allowed!");

SyslogAppender const * SyslogAppender::m_singleInstance = nullptr;

SyslogAppender::SyslogAppender(std::string ident,
                               int const logopt,
                               int const facility)
    : m_ident(std::move(ident))
    , m_logopt(logopt)
    , m_facility(facility)
{ setEnabled_(true); }

SyslogAppender::~SyslogAppender() noexcept { setEnabled_(false); }

void SyslogAppender::doLog(::timeval,
                           Priority const priority,
                           char const * message) noexcept
{
    constexpr static int priorities[] =
        { LOG_EMERG, LOG_ERR, LOG_WARNING, LOG_INFO, LOG_DEBUG, LOG_DEBUG };
    ::syslog(priorities[static_cast<unsigned>(priority)], "%s", message);
}


void SyslogAppender::setEnabled_(bool const enable) const {
    if (enable) {
        if (m_singleInstance)
            throw MultipleSyslogAppenderException{};
        m_singleInstance = this;
        ::openlog(m_ident.c_str(), m_logopt, m_facility);
    } else {
        assert(m_singleInstance == this);
        ::closelog();
        m_singleInstance = nullptr;
    }
}

} /* namespace LogHard { */
