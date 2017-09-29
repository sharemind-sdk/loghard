/*
 * Copyright (C) 2017 Cybernetica
 *
 * Research/Commercial License Usage
 * Licensees holding a valid Research License or Commercial License
 * for the Software may use this file according to the written
 * agreement between you and Cybernetica.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl-3.0.html.
 *
 * For further information, please contact us at sharemind@cyber.ee.
 */

#include <algorithm>
#include <cctype>
#include <string>
#include "PriorityParser.h"

namespace LogHard {

SHAREMIND_DEFINE_EXCEPTION_CONST_MSG_NOINLINE(
        std::exception,,
        PriorityParseException,
        "Failed to parse LogHard priority!");

Priority parsePriority(std::string s) {
    // check if it is a single digit
    if (s.size() == 1 && std::isdigit(s[0])) {
        unsigned l = s[0] - '0';
        if (l > LOGHARD_PRIORITY_FULLDEBUG)
            throw PriorityParseException{};
        return static_cast<Priority>(l);
    }
    // convert to lowercase
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    if (!s.compare("fatal"))
        return Priority::Fatal;
    else if (!s.compare("error"))
        return Priority::Error;
    else if (!s.compare("warn") || !s.compare("warning"))
        return Priority::Warning;
    else if (!s.compare("normal") || !s.compare("info"))
        return Priority::Normal;
    else if (!s.compare("debug"))
        return Priority::Debug;
    else if (!s.compare("fulldebug") || !s.compare("full"))
        return Priority::FullDebug;

    throw PriorityParseException{};
}

} /* namespace LogHard { */
