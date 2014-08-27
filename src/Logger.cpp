/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "Logger.h"


#if !defined(SHAREMIND_GCC_VERSION) || SHAREMIND_GCC_VERSION >= 40800
namespace LogHard {
namespace Detail {
thread_local timeval tl_time = { 0u, 0u };
thread_local Backend * tl_backend = nullptr;
thread_local size_t tl_offset = 0u;
thread_local char tl_message[STACK_BUFFER_SIZE] = {};
} // namespace Detail {
} // namespace LogHard {
#endif
