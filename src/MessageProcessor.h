/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SHAREMINDCOMMON_MESSAGEPROCESSOR_H
#define SHAREMINDCOMMON_MESSAGEPROCESSOR_H

#include <string>

namespace sharemind {

/**
 This class is an interface for classes that process certain messages
*/
class MessageProcessor {

public:

	/**
	 Processes a single string message

	 \param[in] msg A string containing the message
	 */
	virtual void processMessage(const std::string& str) = 0;

private:

};

} // namespace sharemind

#endif // SHAREMINDCOMMON_MESSAGEPROCESSOR_H
