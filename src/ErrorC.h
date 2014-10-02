/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
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
