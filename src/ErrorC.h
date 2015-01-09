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

#ifndef LOGHARD_ERRORC_H
#define LOGHARD_ERRORC_H

#include <sharemind/extern_c.h>
#include <sharemind/preprocessor.h>


SHAREMIND_EXTERN_C_BEGIN

#define LOGHARD_ERROR_ENUM \
    ((LOGHARD_OK, = 0)) \
    ((LOGHARD_OUT_OF_MEMORY,)) \
    ((LOGHARD_IMPLEMENTATION_LIMITS_REACHED,)) \
    ((LOGHARD_MUTEX_ERROR,)) \
    ((LOGHARD_UNKNOWN_ERROR,)) \
    ((LOGHARD_ERROR_COUNT,))
SHAREMIND_ENUM_CUSTOM_DEFINE(LogHardError, LOGHARD_ERROR_ENUM);
SHAREMIND_ENUM_DECLARE_TOSTRING(LogHardError);

SHAREMIND_EXTERN_C_END

#endif /* LOGHARD_ERRORC_H */
