/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#define SHAREMIND_COMMON_INTERNAL__

#include "LogLayout.h"

#include <iomanip>
#include "../Concat.h"
#include "Logger.h"


namespace sharemind {

std::string LogLayout::format(const log4cpp::LoggingEvent & event) {
    return concat(Logger::formatTime(event.timeStamp.getSeconds()), ' ',
                  std::setw(log4cpp::Priority::MESSAGE_SIZE),
                  std::setiosflags(std::ios::left),
                  log4cpp::Priority::getPriorityName(event.priority),
                  event.message, concat_endl);
}

} // namespace sharemind {
