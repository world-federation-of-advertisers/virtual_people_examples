// Copyright 2021 The Cross-Media Measurement Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

#include "gtest/gtest.h"

namespace wfa_virtual_people {
namespace {

TEST(ModelApplierTest, AllPossibleToyModelBranches) {
    std::string cmpEvents = std::system("cmp ./textproto/expected/output_events.txt output_events.txt");
    std::string cmpReports = std::system("cmp ./textproto/expected/output_reports.txt output_reports.txt");

    EXPECT_EQ(cmpEvents, NULL) << cmpEvents << std::endl;
    EXPECT_EQ(cmpReports, NULL) << cmpReports << std::endl;
}

}   // namespace
}   // namespace wfa_virtual_people
