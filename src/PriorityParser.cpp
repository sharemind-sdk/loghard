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

#include <cassert>
#include <string>
#include <type_traits>
#include "PriorityParser.h"

namespace LogHard {

namespace {

constexpr char asciiToLower(char const v) noexcept {
    return (v == 'A') ? 'a' : (v == 'B') ? 'b' : (v == 'C') ? 'c'
         : (v == 'D') ? 'd' : (v == 'E') ? 'e' : (v == 'F') ? 'f'
         : (v == 'G') ? 'g' : (v == 'H') ? 'h' : (v == 'I') ? 'i'
         : (v == 'J') ? 'j' : (v == 'K') ? 'k' : (v == 'L') ? 'l'
         : (v == 'M') ? 'm' : (v == 'N') ? 'n' : (v == 'O') ? 'o'
         : (v == 'P') ? 'p' : (v == 'Q') ? 'q' : (v == 'R') ? 'r'
         : (v == 'S') ? 's' : (v == 'T') ? 't' : (v == 'U') ? 'u'
         : (v == 'V') ? 'v' : (v == 'W') ? 'w' : (v == 'X') ? 'x'
         : (v == 'Y') ? 'y' : (v == 'Z') ? 'z' : v;
};

constexpr bool charIs(char const v, char const a) noexcept
{ return (v == a) || (v == asciiToLower(a)); };

} // anonymous namespace

SHAREMIND_DEFINE_EXCEPTION_CONST_MSG_NOINLINE(
        std::exception,,
        PriorityParseException,
        "Failed to parse LogHard priority!");

Priority parsePriority(char const * c) {
    assert(c);
    if (!*c)
        throw PriorityParseException();
    // check if it is a single digit
    if (c[1u] == '\0') {
        using U = std::underlying_type<Priority>::type;
        U input;
        switch (*c) {
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
    switch (*c) {
    case 'F': case 'f': // "fatal" or "fulldebug" or "full"
        switch (*++c) {
        case 'A': case 'a': // "fatal"
            if (charIs(*++c, 'T') && charIs(*++c, 'A') && charIs(*++c, 'L')
                && ((*++c) == '\0'))
                return Priority::Fatal;
            break;
        case 'U': case 'u': // "fulldebug" or "full"
            if (charIs(*++c, 'L') && charIs(*++c, 'L')
                && (((*++c) == '\0')
                    || (charIs(*c, 'D') && charIs(*++c, 'E')
                        && charIs(*++c, 'B') && charIs(*++c, 'U')
                        && charIs(*++c, 'G') && ((*++c) == '\0'))))
                return Priority::FullDebug;
            break;
        default: break;
        }
        break;
    case 'E': case 'e': // "error"
        if (charIs(*++c, 'R') && charIs(*++c, 'R') && charIs(*++c, 'O')
            && charIs(*++c, 'R') && ((*++c) == '\0'))
            return Priority::Error;
        break;
    case 'W': case 'w': // "warn" or "warning"
        if (charIs(*++c, 'A') && charIs(*++c, 'R') && charIs(*++c, 'N')
            && (((*++c) == '\0')
                || (charIs(*c, 'I') && charIs(*++c, 'N')
                    && charIs(*++c, 'G') && ((*++c) == '\0'))))
            return Priority::Warning;
        break;
    case 'N': case 'n': // "normal"
        if (charIs(*++c, 'O') && charIs(*++c, 'R') && charIs(*++c, 'M')
            && charIs(*++c, 'A') && charIs(*++c, 'L') && ((*++c) == '\0'))
            return Priority::Normal;
        break;
    case 'I': case 'i': // "info"
        if (charIs(*++c, 'N') && charIs(*++c, 'F') && charIs(*++c, 'O')
            && ((*++c) == '\0'))
            return Priority::Normal;
        break;
    case 'D': case 'd': // "debug"
        if (charIs(*++c, 'E') && charIs(*++c, 'B') && charIs(*++c, 'U')
            && charIs(*++c, 'G') && ((*++c) == '\0'))
            return Priority::Debug;
        break;
    default: break;
    }
    throw PriorityParseException();
}

Priority parsePriority(std::string const & s)
{ return parsePriority(s.c_str()); }

} /* namespace LogHard { */
