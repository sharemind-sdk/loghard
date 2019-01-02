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

#include "../src/Logger.h"

#include <string>
#include <type_traits>
#include <utility>
#include "../src/Priority.h"


using L = LogHard::Logger;
using MB = L::MessageBuilder;
using P = LogHard::Priority;

#define D(...) std::declval<__VA_ARGS__>()

#define SA(...) static_assert(__VA_ARGS__, "")

#define CAN_STREAM(...) \
    SA(std::is_same<decltype(D(MB &) << D(__VA_ARGS__ &)), MB &>::value); \
    SA(std::is_same<decltype(D(MB &) << D(__VA_ARGS__ &&)), MB &>::value); \
    SA(std::is_same<decltype(D(MB &) << D(__VA_ARGS__ const &)), MB &>::value);

CAN_STREAM(char);
CAN_STREAM(bool);
CAN_STREAM(signed char);
CAN_STREAM(unsigned char);
CAN_STREAM(short);
CAN_STREAM(unsigned short);
CAN_STREAM(int);
CAN_STREAM(unsigned int);
CAN_STREAM(long);
CAN_STREAM(unsigned long);
CAN_STREAM(long long);
CAN_STREAM(unsigned long long);
CAN_STREAM(L::Hex<unsigned char>);
CAN_STREAM(L::Hex<unsigned short>);
CAN_STREAM(L::Hex<unsigned int>);
CAN_STREAM(L::Hex<unsigned long>);
CAN_STREAM(L::Hex<unsigned long long>);
CAN_STREAM(L::HexByte);
CAN_STREAM(double);
CAN_STREAM(long double);
CAN_STREAM(float);
CAN_STREAM(char const *);
CAN_STREAM(std::string);
CAN_STREAM(void *);
CAN_STREAM(void const *);
CAN_STREAM(sharemind::Uuid)


SA(!std::is_default_constructible<MB >::value);
SA(!std::is_copy_constructible<MB >::value);
SA(!std::is_copy_assignable<MB >::value);
SA(!std::is_move_assignable<MB >::value);
SA(std::is_nothrow_move_constructible<MB >::value);
SA(std::is_nothrow_constructible<MB, P, L const &>::value);
SA(std::is_nothrow_constructible<MB, ::timeval, P, L const &>::value);
SA(std::is_nothrow_destructible<MB >::value);

using SEF = L::StandardExceptionFormatter;
SA(std::is_nothrow_default_constructible<SEF>::value);
SA(std::is_nothrow_constructible<SEF, std::size_t>::value);
SA(std::is_nothrow_copy_constructible<SEF>::value);
SA(std::is_nothrow_move_constructible<SEF>::value);
SA(std::is_nothrow_copy_assignable<SEF>::value);
SA(std::is_nothrow_move_assignable<SEF>::value);
SA(std::is_nothrow_destructible<SEF>::value);

int main() {}
