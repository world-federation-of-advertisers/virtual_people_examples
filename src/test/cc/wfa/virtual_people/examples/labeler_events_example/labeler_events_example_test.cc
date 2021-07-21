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

#include <fcntl.h>

#include "absl/strings/str_cat.h"
#include "gmock/gmock.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/text_format.h"
#include "gtest/gtest.h"
#include "src/main/proto/wfa/virtual_people/common/model.pb.h"

namespace wfa_virtual_people {
namespace {

const char* kTestRelativeDir =
    "/src/main/textproto/wfa/virtual_people/examples/labeler_events_example/";
const char* kTestFiles[] = {
    "labeler_events_01.textproto",
    "labeler_events_02.textproto",
    "labeler_events_03.textproto",
    "labeler_events_04.textproto",
    "labeler_events_05.textproto",
    "labeler_events_06.textproto",
    "labeler_events_07.textproto",
    "labeler_events_08.textproto",
    "labeler_events_09.textproto",
    "labeler_events_10.textproto"
};

inline std::string GetTestSrcDir() {
  return testing::UnitTest::GetInstance()->original_working_dir();
}

TEST(LabelerEventsExampleTest, LoadEvents) {
  for (const char* filename : kTestFiles) {
    LabelerEvent event;
    int fd = open(
        absl::StrCat(GetTestSrcDir(), kTestRelativeDir, filename).c_str(),
        O_RDONLY);
    EXPECT_GT(fd, 0);
    google::protobuf::io::FileInputStream file_input(fd);
    EXPECT_TRUE(google::protobuf::TextFormat::Parse(&file_input, &event));
    close(fd);
  }
}

}  // namespace
}  // namespace wfa_virtual_people
