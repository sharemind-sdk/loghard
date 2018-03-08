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

#include "Priority.h"

#include <boost/program_options.hpp>
#include <exception>
#include "PriorityParser.h"

namespace boost {

namespace po = program_options;

// See http://www.boost.org/doc/libs/1_55_0/doc/html/program_options/howto.html:
void validate(boost::any & v,
              std::vector<std::string> const & values,
              LogHard::Priority * /* target_type */, int)
{
    po::validators::check_first_occurrence(v);
    try {
        v = LogHard::parsePriority(po::validators::get_single_string(values));
    } catch (LogHard::PriorityParseException const &) {
        std::throw_with_nested(
                    po::validation_error(
                        po::validation_error::invalid_option_value));
    }
}

} // namespace boost {
