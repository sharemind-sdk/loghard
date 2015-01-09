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

#ifndef LOGHARD_PRIORITYC_H
#define LOGHARD_PRIORITYC_H

#include <sharemind/extern_c.h>


SHAREMIND_EXTERN_C_BEGIN

/// \todo Use syslog levels
typedef enum LogHardPriority_ {
    LOGHARD_PRIORITY_FATAL = 0,
    LOGHARD_PRIORITY_ERROR = 1,
    LOGHARD_PRIORITY_WARNING = 2,
    LOGHARD_PRIORITY_NORMAL = 3,
    LOGHARD_PRIORITY_DEBUG = 4,
    LOGHARD_PRIORITY_FULLDEBUG = 5
} LogHardPriority;

SHAREMIND_EXTERN_C_END

#endif /* LOGHARD_PRIORITYC_H */
