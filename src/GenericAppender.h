/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SHAREMINDCOMMON_GENERICAPPENDER_H
#define SHAREMINDCOMMON_GENERICAPPENDER_H

#include <log4cpp/Category.hh>
#include <log4cpp/LayoutAppender.hh>
#include <log4cpp/SimpleLayout.hh>

#include "AppenderMessageProcessor.h"

namespace sharemind {

class GenericAppender : public log4cpp::LayoutAppender {

public: /* Methods: */

    inline GenericAppender(const std::string& name, AppenderMessageProcessor *mp) :
        log4cpp::LayoutAppender(name),
        m_mp(mp) {}

    virtual ~GenericAppender() {
        close();
    }

    virtual inline bool reopen () {
        return true;
    }

    virtual inline void close () {}

protected: /* Methods: */

    virtual inline void _append(const log4cpp::LoggingEvent& event) {
        m_mp->processMessage( _getLayout().format(event) );
    }

protected: /* Fields: */

    AppenderMessageProcessor *m_mp;

}; /* class GenericAppender { */

} /* namespace sharemind { */

#endif /* SHAREMINDCOMMON_GENERICAPPENDER_H */
