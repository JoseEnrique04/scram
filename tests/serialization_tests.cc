/*
 * Copyright (C) 2017 Olzhas Rakhimov
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "serialization.h"

#include <sstream>

#include <gtest/gtest.h>
#include <libxml++/libxml++.h>

#include "env.h"
#include "initializer.h"
#include "settings.h"

namespace scram {
namespace mef {

TEST(SerializationTest, InputOutput) {
  static xmlpp::RelaxNGValidator validator(Env::install_dir() +
                                           "/share/scram/gui.rng");

  std::vector<std::vector<std::string>> inputs = {
      {"./share/scram/input/fta/correct_tree_input.xml"},
      {"./share/scram/input/fta/correct_tree_input_with_probs.xml"},
      {"./share/scram/input/fta/missing_bool_constant.xml"},
      {"./share/scram/input/fta/null_gate_with_label.xml"},
      {"./share/scram/input/fta/flavored_types.xml"},
      {"./share/scram/input/TwoTrain/two_train.xml"},
      {"./share/scram/input/fta/correct_formulas.xml"},
      {"./share/scram/input/Theatre/theatre.xml"},
      {"./share/scram/input/Baobab/baobab2.xml",
       "./share/scram/input/Baobab/baobab2-basic-events.xml"}};
  for (const auto& input : inputs) {
    std::shared_ptr<Model> model;
    ASSERT_NO_THROW(model = mef::Initializer(input, core::Settings{}).model());
    std::stringstream output;
    ASSERT_NO_THROW(Serialize(*model, output)) << input.front();

    xmlpp::DomParser parser;
    ASSERT_NO_THROW(parser.parse_stream(output)) << input.front();
    ASSERT_NO_THROW(validator.validate(parser.get_document())) << input.front();
  }
}

}  // namespace mef
}  // namespace scram
