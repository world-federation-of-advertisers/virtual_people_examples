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

// This is a tool to generate a list of events in DataProviderEvent protobuf.
// Example usage:
// bazel build -c opt \
// //src/main/cc/wfa/virtual_people/events_generator:events_generator_main
// bazel-bin/src/main/cc/wfa/virtual_people/events_generator/\
// events_generator_main \
// --output_dir=/tmp/events_generator

#include <fcntl.h>

#include <filesystem>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_cat.h"
#include "absl/time/time.h"
#include "glog/logging.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/message.h"
#include "google/protobuf/text_format.h"
#include "wfa/virtual_people/common/event.pb.h"
#include "wfa/virtual_people/events_generator/events_generator.h"

ABSL_FLAG(uint32_t, total_publishers, 10, "The count of unique publishers.");
ABSL_FLAG(uint32_t, total_events, 1000, "The count of unique event ids.");
ABSL_FLAG(double, unknown_device_ratio, 0.5,
          "The chance of device to be unknown.");
ABSL_FLAG(uint32_t, unknown_device_count, 1000,
          "The count of possible unknown device values.");
ABSL_FLAG(uint32_t, total_countries, 10, "The count of possible countries.");
ABSL_FLAG(uint32_t, regions_per_country, 10,
          "The count of possible regions per country.");
ABSL_FLAG(uint32_t, cities_per_region, 10,
          "The count of possible cities per region.");
ABSL_FLAG(double, email_events_ratio, 0.5,
          "The chance of each event to have email user info.");
ABSL_FLAG(double, phone_events_ratio, 0.5,
          "The chance of each event to have phone user info.");
ABSL_FLAG(double, proprietary_id_space_1_events_ratio, 0.5,
          "The chance of each event to have proprietary id space 1 user info.");
ABSL_FLAG(uint32_t, email_users_count, 100,
          "The count of possible email users.");
ABSL_FLAG(uint32_t, phone_users_count, 100,
          "The count of possible phone users.");
ABSL_FLAG(uint32_t, proprietary_id_space_1_users_count, 100,
          "The count of possible proprietary id space 1 users.");
ABSL_FLAG(uint32_t, profile_version_days, 1,
          "The allowed profile version is in "
          "[today - profile_version_days, today].");
ABSL_FLAG(std::string, output_dir, "",
          "Path to directory to output the events.");

// Write textproto to file
void WriteTextProtoFile(absl::string_view path,
                        const google::protobuf::Message& message) {
  // The output file is only accessible by owner.
  int fd = open(path.data(), O_CREAT | O_WRONLY, S_IRWXU);
  CHECK(fd > 0) << "Unable to create file: " << path;
  google::protobuf::io::FileOutputStream file_output(fd);
  file_output.SetCloseOnDelete(true);
  CHECK(google::protobuf::TextFormat::Print(message, &file_output))
      << "Unable to write textproto file: " << path;
}

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  google::InitGoogleLogging(argv[0]);

  std::string output_dir = absl::GetFlag(FLAGS_output_dir);
  CHECK(!output_dir.empty()) << "output_dir is not set.";

  wfa_virtual_people::EventsGenerator generator(
      absl::ToUnixMicros(absl::Now()), absl::GetFlag(FLAGS_total_publishers),
      absl::GetFlag(FLAGS_total_events),
      absl::GetFlag(FLAGS_unknown_device_count),
      absl::GetFlag(FLAGS_email_users_count),
      absl::GetFlag(FLAGS_phone_users_count),
      absl::GetFlag(FLAGS_proprietary_id_space_1_users_count));

  for (int i = 0; i < absl::GetFlag(FLAGS_total_events); i++) {
    wfa_virtual_people::DataProviderEvent event = generator.GetEvents(
        absl::GetFlag(FLAGS_unknown_device_ratio),
        absl::GetFlag(FLAGS_total_countries),
        absl::GetFlag(FLAGS_regions_per_country),
        absl::GetFlag(FLAGS_cities_per_region),
        absl::GetFlag(FLAGS_email_events_ratio),
        absl::GetFlag(FLAGS_phone_events_ratio),
        absl::GetFlag(FLAGS_proprietary_id_space_1_events_ratio),
        absl::GetFlag(FLAGS_profile_version_days));
    WriteTextProtoFile(absl::StrCat(output_dir, "/event_", i + 1, ".textproto"),
                       event);
  }

  return 0;
}
