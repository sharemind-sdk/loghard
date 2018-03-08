/*
 * Copyright (C) Cybernetica
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

#include "../src/PriorityParser.h"

#include <sharemind/Concat.h>
#include <sharemind/TestAssert.h>
#include <vector>


using LogHard::Priority;
using sharemind::concat;
using U = std::underlying_type<Priority>::type;

void test(std::string const & str, Priority const match) noexcept {
    SHAREMIND_TESTASSERT(LogHard::parsePriority(str) == match);
    SHAREMIND_TESTASSERT(LogHard::parsePriority(str.c_str()) == match);
}

void testNegative(std::string const & str) noexcept {
    try {
        LogHard::parsePriority(str);
        SHAREMIND_TEST_UNREACHABLE;
    } catch (LogHard::PriorityParseException const &) {}
    try {
        LogHard::parsePriority(str.c_str());
        SHAREMIND_TEST_UNREACHABLE;
    } catch (LogHard::PriorityParseException const &) {}
}

void testWithEnd(std::string const & str,
                 Priority const match,
                 std::string const & ends) noexcept
{
    SHAREMIND_TESTASSERT(!str.empty());
    test(str, match);
    testNegative(str[0] + str);
    for (auto const & end : ends)
        testNegative(str + end);
}

void testNumber(Priority const priority)
{ return test(concat(static_cast<U>(priority)), priority); }

int main() {
    testNegative(""); testNegative("asdf");

    testNegative("F"); testNegative("f");
    testNegative("E"); testNegative("e");
    testNegative("W"); testNegative("w");
    testNegative("N"); testNegative("n");
    testNegative("I"); testNegative("i");
    testNegative("D"); testNegative("d");

    testNumber(Priority::Fatal);
    testNumber(Priority::Error);
    testNumber(Priority::Warning);
    testNumber(Priority::Normal);
    testNumber(Priority::Debug);
    testNumber(Priority::FullDebug);

    {
        constexpr int limit = 10;
        constexpr auto ufatal = static_cast<U>(Priority::Fatal);
        constexpr auto uerror = static_cast<U>(Priority::Error);
        constexpr auto uwarn  = static_cast<U>(Priority::Warning);
        constexpr auto unorm  = static_cast<U>(Priority::Normal);
        constexpr auto udebug = static_cast<U>(Priority::Debug);
        constexpr auto ufull  = static_cast<U>(Priority::FullDebug);

        static_assert(ufatal >= 0, "");
        static_assert(uerror >= 0, "");
        static_assert(uwarn >= 0, "");
        static_assert(unorm >= 0, "");
        static_assert(udebug >= 0, "");
        static_assert(ufull >= 0, "");

        static_assert(ufatal < limit, "");
        static_assert(uerror < limit, "");
        static_assert(uwarn < limit, "");
        static_assert(unorm < limit, "");
        static_assert(udebug < limit, "");
        static_assert(ufull < limit, "");

        std::vector<std::string> v;
        for (int i = -limit; i < limit; i++) {
            if (i == ufatal) {
                test(concat(i), Priority::Fatal);
            } else if (i == uerror) {
                test(concat(i), Priority::Error);
            } else if (i == uwarn) {
                test(concat(i), Priority::Warning);
            } else if (i == unorm) {
                test(concat(i), Priority::Normal);
            } else if (i == udebug) {
                test(concat(i), Priority::Debug);
            } else if (i == ufull) {
                test(concat(i), Priority::FullDebug);
            } else {
                testNegative(concat(i));
            }
            testNegative(concat('0', i));
            testNegative(concat(i, '0'));
            testNegative(concat("0x", i));
            testNegative(concat(i, "0x"));
            testNegative(concat(i, '%'));
            testNegative(concat('%', i));
        }
    }


    testWithEnd("fatal", Priority::Fatal, "FATALfatalXx");
    testWithEnd("Fatal", Priority::Fatal, "FATALfatalXx");
    testWithEnd("FATAL", Priority::Fatal, "FATALfatalXx");
    testWithEnd("FaTaL", Priority::Fatal, "FATALfatalXx");
    testWithEnd("fAtAl", Priority::Fatal, "FATALfatalXx");

    testWithEnd("error", Priority::Error, "ERRORerrorXx");
    testWithEnd("Error", Priority::Error, "ERRORerrorXx");
    testWithEnd("ERROR", Priority::Error, "ERRORerrorXx");
    testWithEnd("ErRoR", Priority::Error, "ERRORerrorXx");
    testWithEnd("eRrOr", Priority::Error, "ERRORerrorXx");

    testWithEnd("warn", Priority::Warning, "WARNwarnIiXx");
    testWithEnd("Warn", Priority::Warning, "WARNwarnIiXx");
    testWithEnd("WARN", Priority::Warning, "WARNwarnIiXx");
    testWithEnd("WaRn", Priority::Warning, "WARNwarnIiXx");
    testWithEnd("wArN", Priority::Warning, "WARNwarnIiXx");
    testWithEnd("warning", Priority::Warning, "WARNINGwarningXx");
    testWithEnd("Warning", Priority::Warning, "WARNINGwarningXx");
    testWithEnd("WARNING", Priority::Warning, "WARNINGwarningXx");
    testWithEnd("WaRnInG", Priority::Warning, "WARNINGwarningXx");
    testWithEnd("wArNiNg", Priority::Warning, "WARNINGwarningXx");

    testWithEnd("normal", Priority::Normal, "NORMALnormalXx");
    testWithEnd("Normal", Priority::Normal, "NORMALnormalXx");
    testWithEnd("NORMAL", Priority::Normal, "NORMALnormalXx");
    testWithEnd("NoRmAl", Priority::Normal, "NORMALnormalXx");
    testWithEnd("nOrMaL", Priority::Normal, "NORMALnormalXx");
    testWithEnd("info", Priority::Normal, "INFOinfoXx");
    testWithEnd("Info", Priority::Normal, "INFOinfoXx");
    testWithEnd("INFO", Priority::Normal, "INFOinfoXx");
    testWithEnd("InFo", Priority::Normal, "INFOinfoXx");
    testWithEnd("iNfO", Priority::Normal, "INFOinfoXx");

    testWithEnd("debug", Priority::Debug, "DEBUGdebugXx");
    testWithEnd("Debug", Priority::Debug, "DEBUGdebugXx");
    testWithEnd("DEBUG", Priority::Debug, "DEBUGdebugXx");
    testWithEnd("DeBuG", Priority::Debug, "DEBUGdebugXx");
    testWithEnd("dEbUg", Priority::Debug, "DEBUGdebugXx");

    testWithEnd("full", Priority::FullDebug, "FULLfullXxDd");
    testWithEnd("Full", Priority::FullDebug, "FULLfullXxDd");
    testWithEnd("FULL", Priority::FullDebug, "FULLfullXxDd");
    testWithEnd("fUlL", Priority::FullDebug, "FULLfullXxDd");
    testWithEnd("FuLl", Priority::FullDebug, "FULLfullXxDd");
    testWithEnd("fulldebug", Priority::FullDebug, "FULLDEBUGfulldebugXx");
    testWithEnd("Fulldebug", Priority::FullDebug, "FULLDEBUGfulldebugXx");
    testWithEnd("FullDebug", Priority::FullDebug, "FULLDEBUGfulldebugXx");
    testWithEnd("fullDebug", Priority::FullDebug, "FULLDEBUGfulldebugXx");
    testWithEnd("FULLDEBUG", Priority::FullDebug, "FULLDEBUGfulldebugXx");
    testWithEnd("FuLlDeBuG", Priority::FullDebug, "FULLDEBUGfulldebugXx");
    testWithEnd("fUlLdEbUg", Priority::FullDebug, "FULLDEBUGfulldebugXx");
}
