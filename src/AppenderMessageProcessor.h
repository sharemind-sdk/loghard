/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SHAREMINDCOMMON_APPENDERMESSAGEPROCESSOR_H
#define SHAREMINDCOMMON_APPENDERMESSAGEPROCESSOR_H

#include "MessageProcessor.h"

#include <string>


namespace sharemind {

class GenericAppender;

/**
 This class is an interface for classes that process certain messages
*/
class AppenderMessageProcessor : public MessageProcessor {

public:

    AppenderMessageProcessor(const std::string& name);

    ~AppenderMessageProcessor();

    GenericAppender* getGenericAppender() {
        return m_ga;
    }

    /**
     Processes a single string message

     \param[in] msg A string containing the message
     */
    virtual void processMessage(const std::string& str) = 0;

private:

    GenericAppender *m_ga;
};

} // namespace sharemind

#endif // SHAREMINDCOMMON_APPENDERMESSAGEPROCESSOR_H
