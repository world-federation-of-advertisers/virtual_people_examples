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
#include "tools/cpp/runfiles/runfiles.h"

namespace wfa_virtual_people {
namespace {

using bazel::tools::cpp::runfiles::Runfiles;

// TEST(ModelApplierTest, EmptyModelInput) {}

// TEST(ModelApplierTest, EmptyEventInput) {}

TEST(ModelApplierTest, AllPossibleToyModelBranches) {
    std::string error;
    std::unique_ptr<Runfiles> runfiles(Runfiles::CreateForTest(&error));

    std::string rlocation_path = "virtual_people_examples/src/main/cc/wfa/virtual_people/model_applier/model_applier";
    std::string path = runfiles->Rlocation(rlocation_path);

    if (path.empty()) { 
        FAIL() << "failed to lookup runfile for: " << rlocation_path << std::endl; 
    }

    std::string model_node_path="//src/test/cc/wfa/virtual_people/model_applier/textproto/toy_model.textproto";
    std::string input_path="//src/test/cc/wfa/virtual_people/model_applier/textproto/input_events.textproto";
    std::string output_dir=".";

    std::string cmd = path + 
        " --model_node_path=" + model_node_path + 
        " --input_path=" + input_path + 
        " --output_dir=" + output_dir;

    std::system(cmd.c_str());
}

}   // namespace
}   // namespace wfa_virtual_people
