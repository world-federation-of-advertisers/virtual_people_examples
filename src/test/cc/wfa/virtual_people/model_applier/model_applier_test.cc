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

#include <cstdlib>
#include <fcntl.h>
#include <string>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "google/protobuf/text_format.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/util/message_differencer.h"

using namespace ::google::protobuf;
using namespace ::google::protobuf::io;
using namespace ::google::protobuf::util;

namespace wfa_virtual_people {
namespace {

void ReadTextProtoFile(std::string path, Message* message) {
    CHECK(!path.empty()) << "No path set";
    int fd = open(path.c_str(), O_RDONLY);
    CHECK(fd > 0) << "Unable to open file: " << path;
    FileInputStream fstream(fd);
    CHECK(TextFormat::Parse(&fstream, message)) 
        << "Unable to parse textproto file: " << path;
}

TEST(ModelApplierTest, AllPossibleToyModelBranches) {
    std::string eventsExpectedPath = "//src/test/cc/wfa/virtual_people/model_applier/textproto/expected/output_events.txt";
    std::string eventsOutputPath = "//bazel-out/k8-fastbuild/bin/src/test/cc/wfa/virtual_people/model_applier/output_events.txt";

    std::string reportsExpectedPath = "//src/test/cc/wfa/virtual_people/model_applier/textproto/expected/output_reports.txt";
    std::string reportsOutputPath = "//bazel-out/k8-fastbuild/bin/src/test/cc/wfa/virtual_people/model_applier/output_reports.txt";
    
    Message* eventsExpected = nullptr;
    ReadTextProtoFile(eventsExpectedPath, eventsExpected);

    // Message* eventsOutput = nullptr;
    // ReadTextProtoFile(eventsOutputPath, eventsOutput);

    // Message* reportsExpected = nullptr;
    // ReadTextProtoFile(reportsExpectedPath, reportsExpected);

    // Message* reportsOutput = nullptr;
    // ReadTextProtoFile(reportsOutputPath, reportsOutput);

    // MessageDifferencer diff;
    // EXPECT_TRUE(diff.Equals(*eventsExpected, *eventsOutput));
    // EXPECT_TRUE(diff.Equals(*reportsExpected, *reportsOutput));
}

}   // namespace
}   // namespace wfa_virtual_people
