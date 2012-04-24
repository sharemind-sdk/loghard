/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include <sstream>

#include "Logger.h"
#include "LogLayout.h"

using std::ostringstream;
using namespace sharemind;

LogLayout::LogLayout(Logger* logger)
    : m_logger (logger)
{
}

LogLayout::~LogLayout() {
}

std::string LogLayout::format(const log4cpp::LoggingEvent& event) {
    ostringstream message;

    const std::string& priorityName = log4cpp::Priority::getPriorityName(event.priority);

    message << Logger::formatTime (event.timeStamp.getSeconds()) << " ";

    message.width(log4cpp::Priority::MESSAGE_SIZE);
    message.setf(std::ios::left);

    message << priorityName << ": " << m_logger->getMessagePrefix() << event.message << std::endl;

    return message.str();
}
