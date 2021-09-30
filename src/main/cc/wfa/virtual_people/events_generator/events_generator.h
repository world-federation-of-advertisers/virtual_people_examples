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

#ifndef SRC_MAIN_CC_WFA_VIRTUAL_PEOPLE_EVENTS_GENERATOR_EVENTS_GENERATOR_H_
#define SRC_MAIN_CC_WFA_VIRTUAL_PEOPLE_EVENTS_GENERATOR_EVENTS_GENERATOR_H_

#include <string>
#include <vector>

#include "absl/time/civil_time.h"
#include "wfa/virtual_people/common/event.pb.h"
#include "wfa/virtual_people/events_generator/random_generator.h"

namespace wfa_virtual_people {

struct PublisherEventId {
  std::string publisher;
  std::string id;
};

// EventsGenerator is used to generate random DataProviderEvents. For fields in
// log_event.labeler_input
// * event_id.publisher is composed of 8 digits.
// * event_id.id is composed of 16 digits.
// * timestamp_usec is a timestamp between 30 days ago to current timestamp in
//   microsecond.
// * user_agent is an integer between 0 and 99 when representing known device,
//   or composed of 10 lower case letters when representing unknown device.
// * geo.country_id is a 3-digit integer.
// * geo.region_id is a 6-digit integer, and the first 3-digit is same as
//   geo.country_id.
// * geo.city_id is a 9-digit integer, and the first 6-digit is same as
//   geo.region_id.
// * profile_info.email_user_info.user_id is in format <PART1>@<PART2>.com,
//   while <PART1> is composed of 1 to 10 lower case letters, and <PART2> is
//   composed of 4 to 8 lower case letters.
// * profile_info.phone_user_info.user_id is composed of 10 digits.
// * profile_info.proprietary_id_space_1_user_info.user_id is composed of 16
//   digits.
// * For profile_info.email_user_info.profile_version,
//   profile_info.phone_user_info.profile_version, and
//   profile_info.proprietary_id_space_1_user_info.profile_version, the format
//   is YYYY-MM-DD.
// * For other fields in profile_info.email_user_info,
//   profile_info.phone_user_info, and
//   profile_info.proprietary_id_space_1_user_info, the value are decided by the
//   value of user_id and profile_version.
// *** demo.demo_bucket.age.min_age is between 0 and 120.
// *** demo.demo_bucket.age.max_age is between 0 and 120, or 1000, and is larger
//     than demo.demo_bucket.age.min_age.
// *** demo.confidence is between 0.0 and 1.0.
// *** home_geo has the same pattern as the geo field above.
// *** creation_time_usec is a timestamp between 100 days ago to profile_version
//     in microsecond.
class EventsGenerator {
 public:
  // Initialzies the pseudo-random number generator with default seed.
  // * @current_timestamp is the current timestamp in microseconds.
  // * @total_publishers is the count of unique event_id.publisher.
  // * @total_events is the count of unique event_id.id.
  // * @unknown_device_count is the count of unique user_agent when it
  //   represents unknown device.
  // * @email_users_count is the count of unique
  //   profile_info.email_user_info.user_id.
  // * @phone_users_count is the count of unique
  //   profile_info.phone_user_info.user_id.
  // * @proprietary_id_space_1_users_count is the count of unique
  //   profile_info.proprietary_id_space_1_user_info.user_id.
  EventsGenerator(uint64_t current_timestamp, uint32_t total_publishers,
                  uint32_t total_events, uint32_t unknown_device_count,
                  uint32_t email_users_count, uint32_t phone_users_count,
                  uint32_t proprietary_id_space_1_users_count);

  // Same as above, but initializes the pseudo-random number generator to set
  // the @seed for the random generator.
  EventsGenerator(uint64_t current_timestamp, uint32_t total_publishers,
                  uint32_t total_events, uint32_t unknown_device_count,
                  uint32_t email_users_count, uint32_t phone_users_count,
                  uint32_t proprietary_id_space_1_users_count, uint32_t seed);

  // Generates a random DataProviderEvent. Only fields in
  // log_event.labeler_input are set.
  // * @unknown_device_ratio is chance that user_agent represents unknown
  //   device.
  // * @total_countries is the count of possible country_ids. The value of
  //   geo.country_id is between 100 and (99 + @total_countries).
  // * @regions_per_country is the count of possible region_ids for each
  //   country_id. The value of last 3 digits of geo.region_id is between 000 to
  //   (@regions_per_country - 1).
  // * @cities_per_region is the count of possible city_ids for each region_id.
  //   The value of last 3 digits of geo.city_id is between 000 to
  //   (@cities_per_region - 1).
  // * @email_events_ratio is the chance that profile_info.email_user_info is
  //   set.
  // * @phone_events_ratio is the chance that profile_info.phone_user_info is
  //   set.
  // * @proprietary_id_space_1_events_ratio is the chance that
  //   profile_info.proprietary_id_space_1_user_info is set.
  // * @profile_version_days decides the range of profile_version. The value of
  //   profile_version is between @profile_version_days ago to current date.
  DataProviderEvent GetEvents(double unknown_device_ratio,
                              uint32_t total_countries,
                              uint32_t regions_per_country,
                              uint32_t cities_per_region,
                              double email_events_ratio,
                              double phone_events_ratio,
                              double proprietary_id_space_1_events_ratio,
                              uint32_t profile_version_days);

 private:
  void BuildEventIdPool(uint32_t total_publishers, uint32_t total_events);
  void BuildUnknownDevicePool(uint32_t unknown_device_count);
  void BuildEmailPool(uint32_t email_users_count);
  void BuildPhonePool(uint32_t phone_users_count);
  void BuildProprietaryIdSpace1Pool(
      uint32_t proprietary_id_space_1_users_count);
  void Initialize(uint32_t total_publishers, uint32_t total_events,
                  uint32_t unknown_device_count, uint32_t email_users_count,
                  uint32_t phone_users_count,
                  uint32_t proprietary_id_space_1_users_count);
  EventId GetEventId();
  std::string GetDevice(double unknown_device_ratio);
  GeoLocation GetGeo(uint32_t total_countries, uint32_t regions_per_country,
                     uint32_t cities_per_region);
  ProfileInfo GetProfileInfo(double email_events_ratio,
                             double phone_events_ratio,
                             double proprietary_id_space_1_events_ratio,
                             uint32_t profile_version_days,
                             uint32_t total_countries,
                             uint32_t regions_per_country,
                             uint32_t cities_per_region);

  RandomGenerator random_generator_;
  uint64_t current_timestamp_;
  absl::CivilDay current_day_;
  std::vector<PublisherEventId> event_id_pool_;
  std::vector<std::string> unknown_device_pool_;
  std::vector<std::string> email_pool_;
  std::vector<std::string> phone_pool_;
  std::vector<std::string> proprietary_id_space_1_pool_;
};

}  // namespace wfa_virtual_people

#endif  // SRC_MAIN_CC_WFA_VIRTUAL_PEOPLE_EVENTS_GENERATOR_EVENTS_GENERATOR_H_
