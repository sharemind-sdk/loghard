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

#ifndef LOGHARD_PRIORITY_H
#define LOGHARD_PRIORITY_H

#include "PriorityC.h"

#include <boost/any.hpp>
#include <string>
#include <vector>


namespace LogHard {

/// \todo Use syslog levels
enum class Priority : unsigned {
    Fatal = LOGHARD_PRIORITY_FATAL,
    Error = LOGHARD_PRIORITY_ERROR,
    Warning = LOGHARD_PRIORITY_WARNING,
    Normal = LOGHARD_PRIORITY_NORMAL,
    Debug = LOGHARD_PRIORITY_DEBUG,
    FullDebug = LOGHARD_PRIORITY_FULLDEBUG
};

} /* namespace LogHard { */

namespace boost {

// For use with boost::program_options:
void validate(boost::any & v,
              std::vector<std::string> const & values,
              LogHard::Priority * target_type, int);

} /* namespace boost { */

#endif /* LOGHARD_PRIORITY_H */
