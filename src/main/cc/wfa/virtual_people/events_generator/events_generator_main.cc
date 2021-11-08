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
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/time/time.h"
#include "common_cpp/protobuf_util/textproto_io.h"
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
ABSL_FLAG(bool, textproto, true, "If true, writes textproto files");
ABSL_FLAG(bool, binary, false, "If true, writes binary proto files");

absl::Status WriteBinaryProtoFile(absl::string_view filename,
                                  const google::protobuf::Message& message) {
  int fd = open(filename.data(), O_CREAT | O_WRONLY, S_IRWXU);
  google::protobuf::io::FileOutputStream output(fd);
  output.SetCloseOnDelete(true);

  if (!message.SerializeToZeroCopyStream(&output)) {
    return absl::InternalError("Unable to serialize");
  }

  return absl::OkStatus();
}

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  google::InitGoogleLogging(argv[0]);

  std::string output_dir = absl::GetFlag(FLAGS_output_dir);
  CHECK(!output_dir.empty()) << "output_dir is not set.";

  CHECK(absl::GetFlag(FLAGS_textproto) || absl::GetFlag(FLAGS_binary))
      << "At least one of --textproto and --binary is required";

  wfa_virtual_people::EventsGeneratorOptions event_generator_options = {
      .current_timestamp =
          static_cast<uint64_t>(absl::ToUnixMicros(absl::Now())),
      .total_publishers = absl::GetFlag(FLAGS_total_publishers),
      .total_events = absl::GetFlag(FLAGS_total_events),
      .unknown_device_count = absl::GetFlag(FLAGS_unknown_device_count),
      .email_users_count = absl::GetFlag(FLAGS_email_users_count),
      .phone_users_count = absl::GetFlag(FLAGS_phone_users_count),
      .proprietary_id_space_1_users_count =
          absl::GetFlag(FLAGS_proprietary_id_space_1_users_count)};

  wfa_virtual_people::EventOptions event_options = {
      .unknown_device_ratio = absl::GetFlag(FLAGS_unknown_device_ratio),
      .total_countries = absl::GetFlag(FLAGS_total_countries),
      .regions_per_country = absl::GetFlag(FLAGS_regions_per_country),
      .cities_per_region = absl::GetFlag(FLAGS_cities_per_region),
      .email_events_ratio = absl::GetFlag(FLAGS_email_events_ratio),
      .phone_events_ratio = absl::GetFlag(FLAGS_phone_events_ratio),
      .proprietary_id_space_1_events_ratio =
          absl::GetFlag(FLAGS_proprietary_id_space_1_events_ratio),
      .profile_version_days = absl::GetFlag(FLAGS_profile_version_days)};

  wfa_virtual_people::EventsGenerator generator(event_generator_options);

  for (int i = 0; i < absl::GetFlag(FLAGS_total_events); ++i) {
    wfa_virtual_people::DataProviderEvent event =
        generator.GetEvent(event_options);

    std::string prefix = absl::StrCat(output_dir, "/event-", i + 1);

    if (absl::GetFlag(FLAGS_textproto)) {
      std::string filename = absl::StrCat(prefix, ".textproto");
      absl::Status status = wfa::WriteTextProtoFile(filename, event);
      CHECK(status.ok()) << status;
    }

    if (absl::GetFlag(FLAGS_binary)) {
      std::string filename = absl::StrCat(prefix, ".pb");
      absl::Status status = WriteBinaryProtoFile(filename, event);
      CHECK(status.ok()) << status;
    }
  }

  return 0;
}
