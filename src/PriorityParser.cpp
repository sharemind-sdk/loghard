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
#include <type_traits>
#include "PriorityParser.h"

namespace LogHard {

SHAREMIND_DEFINE_EXCEPTION_CONST_MSG_NOINLINE(
        std::exception,,
        PriorityParseException,
        "Failed to parse LogHard priority!");

Priority parsePriority(std::string s) {
    // check if it is a single digit
    if (s.size() == 1) {
        using U = std::underlying_type<Priority>::type;
        U input;
        switch (s[0u]) {
        case '0': input = 0u; break;
        case '1': input = 1u; break;
        case '2': input = 2u; break;
        case '3': input = 3u; break;
        case '4': input = 4u; break;
        case '5': input = 5u; break;
        case '6': input = 6u; break;
        case '7': input = 7u; break;
        case '8': input = 8u; break;
        case '9': input = 9u; break;
        default: throw PriorityParseException();
        }
        switch (input) {
        case static_cast<U>(Priority::Fatal): return Priority::Fatal;
        case static_cast<U>(Priority::Error): return Priority::Error;
        case static_cast<U>(Priority::Warning): return Priority::Warning;
        case static_cast<U>(Priority::Normal): return Priority::Normal;
        case static_cast<U>(Priority::Debug): return Priority::Debug;
        case static_cast<U>(Priority::FullDebug): return Priority::FullDebug;
        default: throw PriorityParseException();
        }
    }
    // convert to lowercase
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    if (s == "fatal") {
        return Priority::Fatal;
    } else if (s == "error") {
        return Priority::Error;
    } else if ((s == "warn") || (s == "warning")) {
        return Priority::Warning;
    } else if ((s == "normal") || (s == "info")) {
        return Priority::Normal;
    } else if (s == "debug") {
        return Priority::Debug;
    } else if ((s == "fulldebug") || (s == "full")) {
        return Priority::FullDebug;
    } else {
        throw PriorityParseException{};
    }
}

} /* namespace LogHard { */
