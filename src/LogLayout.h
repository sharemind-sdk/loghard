/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SHAREMINDCOMMON_LOGLAYOUT_H
#define SHAREMINDCOMMON_LOGLAYOUT_H

#include <log4cpp/Layout.hh>

namespace sharemind {

class Logger;

/**
 * LogLayout is a fixed format log4cpp::Layout implementation for Sharemind.
 **/
class LogLayout : public log4cpp::Layout {

public: /* Methods: */

    LogLayout(Logger* logger);

    virtual ~LogLayout();

    /**
     * Formats the LoggingEvent in LogLayout style:<br>
     * "formattedTime priority ndc: message"
     **/
    virtual std::string format(const log4cpp::LoggingEvent& event);

private: /* Methods: */

    LogLayout();

private: /* Fields: */

    /** \warning The ownership of m_logger is not ours, do not delete it by accident! */
    Logger* m_logger;

}; /* class LogLayout { */

} /* namespace sharemind { */

#endif /* SHAREMINDCOMMON_LOGLAYOUT_H */
