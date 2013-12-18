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

#ifndef SHAREMIND_COMMON_INTERNAL__
#error including an internal header!
#endif

#include <log4cpp/Category.hh>
#include <log4cpp/LayoutAppender.hh>
#include <log4cpp/SimpleLayout.hh>

#include "MessageProcessor.h"

namespace sharemind {

class GenericAppender : public log4cpp::LayoutAppender {

public: /* Methods: */

    inline GenericAppender(const std::string &name, MessageProcessor *mp)
        : log4cpp::LayoutAppender(name)
        , m_mp(mp)
    {}

    virtual ~GenericAppender() { close(); }

    virtual inline bool reopen () { return true; }
    virtual inline void close () {}

protected: /* Methods: */

    virtual inline void _append(const log4cpp::LoggingEvent &event) {
        m_mp->processMessage( _getLayout().format(event) );
    }

protected: /* Fields: */

    MessageProcessor * m_mp;

}; /* class GenericAppender { */

} /* namespace sharemind { */

#endif /* SHAREMINDCOMMON_GENERICAPPENDER_H */
