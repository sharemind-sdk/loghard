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
thread_local timeval sharemind::Detail::Logger::tl_time = { 0u, 0u };
thread_local sharemind::LogBackend * sharemind::Detail::Logger::tl_backend =
        nullptr;
thread_local size_t sharemind::Detail::Logger::tl_offset = 0u;
thread_local char sharemind::Detail::Logger::tl_message[STACK_BUFFER_SIZE] = {};
#endif
