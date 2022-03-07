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

#include "wfa/virtual_people/events_generator/events_generator.h"

#include <regex>
#include <string>

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "wfa/virtual_people/common/event.pb.h"

namespace wfa_virtual_people {
namespace {

using ::testing::AllOf;
using ::testing::AnyOf;
using ::testing::Eq;
using ::testing::Ge;
using ::testing::Le;
using ::testing::Matcher;
using ::testing::MatchesRegex;
using ::testing::Property;

// 24 * 3600 * 1000
constexpr uint64_t kMicrosecPerDay = 86400000;

MATCHER_P2(IsBetween, low, high, "") {
  return ExplainMatchResult(AllOf(Ge(low), Le(high)), arg, result_listener);
}

Matcher<absl::string_view> IsValidPublisher() {
  return MatchesRegex("[0-9]{8}");
}

Matcher<absl::string_view> IsValidId() { return MatchesRegex("[0-9]{16}"); }

Matcher<int64_t> IsValidTimestampUsec(const int64_t current_timestamp) {
  return IsBetween(current_timestamp - 30 * kMicrosecPerDay, current_timestamp);
}

Matcher<absl::string_view> IsValidUserAgent() {
  return AnyOf(MatchesRegex("[a-z]{10}"), MatchesRegex("[0-9]{1,2}"));
}

MATCHER_P3(IsValidGeo, total_countries, regions_per_country, cities_per_region,
           "") {
  uint32_t country_id = arg.country_id();
  uint32_t region_id = arg.region_id();
  return ExplainMatchResult(
      AllOf(Property("country_id", &GeoLocation::country_id,
                     IsBetween(100, 99 + total_countries)),
            Property("region_id", &GeoLocation::region_id,
                     IsBetween(country_id * 1000,
                               country_id * 1000 + regions_per_country - 1)),
            Property("city_id", &GeoLocation::city_id,
                     IsBetween(region_id * 1000,
                               region_id * 1000 + cities_per_region - 1))),
      arg, result_listener);
}

MATCHER(IsValidDemo, "") {
  return ExplainMatchResult(
      AllOf(Property(
                "demo_bucket", &DemoInfo::demo_bucket,
                Property("age", &DemoBucket::age,
                         AllOf(Property("min_age", &AgeRange::min_age,
                                        IsBetween(0, 120)),
                               Property("max_age", &AgeRange::max_age,
                                        AnyOf(IsBetween(0, 120), Eq(1000)))))),
            Property("confidence", &DemoInfo::confidence, IsBetween(0.0, 1.0))),
      arg, result_listener);
}

MATCHER_P4(IsValidUserInfo, user_id_regex, total_countries, regions_per_country,
           cities_per_region, "") {
  return ExplainMatchResult(
      AllOf(
          Property("user_id", &UserInfo::user_id, MatchesRegex(user_id_regex)),
          Property("profile_version", &UserInfo::profile_version,
                   MatchesRegex("[0-9]{4}-[0-9]{2}-[0-9]{2}")),
          Property("demo", &UserInfo::demo, IsValidDemo()),
          Property("home_geo", &UserInfo::home_geo,
                   IsValidGeo(total_countries, regions_per_country,
                              cities_per_region))),
      arg, result_listener);
}

MATCHER_P3(IsValidProfileInfo, total_countries, regions_per_country,
           cities_per_region, "") {
  if (arg.has_email_user_info()) {
    return ExplainMatchResult(
        IsValidUserInfo("[a-z]{1,10}@[a-z]{4,8}\\.example.com", total_countries,
                        regions_per_country, cities_per_region),
        arg.email_user_info(), result_listener);
  }
  if (arg.has_phone_user_info()) {
    return ExplainMatchResult(
        IsValidUserInfo("\\+\\(555\\)[0-9]{3}-[0-9]{4}", total_countries,
                        regions_per_country, cities_per_region),
        arg.phone_user_info(), result_listener);
  }
  if (arg.has_proprietary_id_space_1_user_info()) {
    return ExplainMatchResult(
        IsValidUserInfo("[0-9]{16}", total_countries, regions_per_country,
                        cities_per_region),
        arg.proprietary_id_space_1_user_info(), result_listener);
  }
  return true;
}

TEST(EventsGeneratorTest, SanityCheck) {
  uint64_t current_timestamp = 1626847100000000;
  uint32_t total_events = 1000;
  uint32_t total_countries = 10;
  uint32_t regions_per_country = 10;
  uint32_t cities_per_region = 10;

  EventsGeneratorOptions events_generator_options = {
      .current_timestamp = current_timestamp,
      .total_publishers = 10,
      .total_events = total_events,
      .unknown_device_count = 100,
      .email_users_count = 100,
      .phone_users_count = 100,
      .proprietary_id_space_1_users_count = 100};

  EventOptions event_options = {.unknown_device_ratio = 0.5,
                                .total_countries = total_countries,
                                .regions_per_country = regions_per_country,
                                .cities_per_region = cities_per_region,
                                .email_events_ratio = 0.5,
                                .phone_events_ratio = 0.5,
                                .proprietary_id_space_1_events_ratio = 0.5,
                                .profile_version_days = 1};

  EventsGenerator generator(events_generator_options);
  for (int i = 0; i < total_events; i++) {
    DataProviderEvent event = generator.GetEvent(event_options);
    SCOPED_TRACE(
        absl::StrCat("Index: ", i, "\n", "Event: ", event.DebugString()));
    const LabelerInput& labeler_input = event.log_event().labeler_input();
    EXPECT_THAT(labeler_input.event_id().publisher(), IsValidPublisher());
    EXPECT_THAT(labeler_input.event_id().id(), IsValidId());
    EXPECT_THAT(labeler_input.timestamp_usec(),
                IsValidTimestampUsec(current_timestamp));
    EXPECT_THAT(labeler_input.user_agent(), IsValidUserAgent());
    EXPECT_THAT(
        labeler_input.geo(),
        IsValidGeo(total_countries, regions_per_country, cities_per_region));
    EXPECT_THAT(labeler_input.profile_info(),
                IsValidProfileInfo(total_countries, regions_per_country,
                                   cities_per_region));
  }
}

}  // namespace
}  // namespace wfa_virtual_people
