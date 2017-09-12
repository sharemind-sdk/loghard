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

#ifndef LOGHARD_SYSLOGAPPENDER_H
#define LOGHARD_SYSLOGAPPENDER_H

#include "Appender.h"

#include <exception>
#include <sharemind/Exception.h>
#include <string>


namespace LogHard {

class SyslogAppender: public Appender {

public: /* Types: */

    SHAREMIND_DECLARE_EXCEPTION_NOINLINE(std::exception, Exception);
    SHAREMIND_DECLARE_EXCEPTION_CONST_MSG_NOINLINE(
            Exception,
            MultipleSyslogAppenderException);

public: /* Methods: */

    SyslogAppender(std::string ident, int const logopt, int const facility);

    ~SyslogAppender() noexcept override;

private: /* Methods: */

    void doLog(::timeval,
               Priority const priority,
               char const * message) noexcept override;

    void setEnabled_(bool const enable) const;

private: /* Fields: */

    std::string const m_ident;
    int const m_logopt;
    int const m_facility;
    static SyslogAppender const * m_singleInstance;

}; /* class SyslogAppender { */

} /* namespace LogHard { */

#endif /* LOGHARD_SYSLOGAPPENDER_H */
