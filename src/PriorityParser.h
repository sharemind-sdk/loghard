/*
 * Copyright (C) 2017 Cybernetica
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

#ifndef LOGHARD_PRIORITY_PARSER_H
#define LOGHARD_PRIORITY_PARSER_H

#include <exception>
#include <sharemind/ExceptionMacros.h>
#include <string>
#include "Priority.h"


namespace LogHard {

    SHAREMIND_DECLARE_EXCEPTION_CONST_MSG_NOINLINE(
            std::exception,
            PriorityParseException);

    Priority parsePriority(std::string const & s);

} /* namespace LogHard { */

#endif /* LOGHARD_PRIORITY_PARSER_H */
