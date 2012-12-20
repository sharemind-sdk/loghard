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

#include <sstream>
#include "Logger.h"


namespace sharemind {

std::string LogLayout::format(const log4cpp::LoggingEvent & event) {
    std::ostringstream oss;

    const std::string & priorityName = log4cpp::Priority::getPriorityName(event.priority);

    oss << Logger::formatTime(event.timeStamp.getSeconds()) << " ";
    oss.width(log4cpp::Priority::MESSAGE_SIZE);
    oss.setf(std::ios::left);
    oss << priorityName << ": " << m_logger.getMessagePrefix() << event.message << std::endl;

    return oss.str();
}

} // namespace sharemind {
