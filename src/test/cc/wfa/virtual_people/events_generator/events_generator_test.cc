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

#include <string>
#include <regex>

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

// 24 * 3600 * 1000
constexpr uint64_t kMicrosecPerDay = 86400000;

Matcher<absl::string_view> IsValidPublisher() {
  return MatchesRegex("[0-9]{8}");
}

Matcher<absl::string_view> IsValidId() { return MatchesRegex("[0-9]{16}"); }

Matcher<int64_t> IsValidTimestampUsec(const int64_t current_timestamp) {
  return AllOf(Ge(current_timestamp - 30 * kMicrosecPerDay),
               Le(current_timestamp));
}

Matcher<absl::string_view> IsValidUserAgent() {
  return AnyOf(MatchesRegex("[a-z]{10}"), MatchesRegex("[0-9]{1,2}"));
}

bool GeoSanityCheck(const GeoLocation& geo, const uint32_t total_countries,
                    const uint32_t regions_per_country,
                    const uint32_t cities_per_region) {
  uint32_t country_id = geo.country_id();
  uint32_t region_id = geo.region_id();
  uint32_t city_id = geo.city_id();
  if (country_id < 100 || country_id > 99 + total_countries) {
    return false;
  }
  if (region_id < country_id * 1000 ||
      region_id > country_id * 1000 + regions_per_country - 1) {
    return false;
  }
  if (city_id < region_id * 1000 ||
      city_id > region_id * 1000 + cities_per_region - 1) {
    return false;
  }
  return true;
}

MATCHER_P3(
    IsValidGeo, total_countries, regions_per_country, cities_per_region, "") {
  return GeoSanityCheck(
      arg, total_countries, regions_per_country, cities_per_region);
}

bool UserInfoSanityCheck(const UserInfo& user_info,
                         const uint32_t total_countries,
                         const uint32_t regions_per_country,
                         const uint32_t cities_per_region) {
  if (!std::regex_match(user_info.profile_version(),
                        std::regex("[0-9]{4}-[0-9]{2}-[0-9]{2}"))) {
    return false;
  }
  uint32_t min_age = user_info.demo().demo_bucket().age().min_age();
  if (min_age < 0 || min_age > 120) {
    return false;
  }
  uint32_t max_age = user_info.demo().demo_bucket().age().max_age();
  if ((max_age < 0 || max_age > 120) && max_age != 1000) {
    return false;
  }
  double confidence = user_info.demo().confidence();
  if (confidence < 0.0 || confidence > 1.0) {
    return false;
  }
  if (!GeoSanityCheck(
      user_info.home_geo(),
      total_countries, regions_per_country, cities_per_region)) {
    return false;
  }
  return true;
}

bool ProfileInfoSanityCheck(const ProfileInfo& profile_info,
                            const uint32_t total_countries,
                            const uint32_t regions_per_country,
                            const uint32_t cities_per_region) {
  if (profile_info.has_email_user_info()) {
    if (!std::regex_match(profile_info.email_user_info().user_id(),
                          std::regex("[a-z]{1,10}@[a-z]{4,8}\\.example.com"))) {
      return false;
    }
    if (!UserInfoSanityCheck(profile_info.email_user_info(), total_countries,
                             regions_per_country, cities_per_region)) {
      return false;
    }
  }
  if (profile_info.has_phone_user_info()) {
    if (!std::regex_match(profile_info.phone_user_info().user_id(),
                          std::regex("\\+\\(555\\)[0-9]{3}-[0-9]{4}"))) {
      return false;
    }
    if (!UserInfoSanityCheck(profile_info.phone_user_info(), total_countries,
                             regions_per_country, cities_per_region)) {
      return false;
    }
  }
  if (profile_info.has_proprietary_id_space_1_user_info()) {
    if (!std::regex_match(
        profile_info.proprietary_id_space_1_user_info().user_id(),
        std::regex("[0-9]{16}"))) {
      return false;
    }
    if (!UserInfoSanityCheck(
        profile_info.proprietary_id_space_1_user_info(),
        total_countries, regions_per_country, cities_per_region)) {
      return false;
    }
  }
  return true;
}

MATCHER_P3(
    IsValidProfileInfo, total_countries, regions_per_country, cities_per_region,
    "") {
  return ProfileInfoSanityCheck(
      arg, total_countries, regions_per_country, cities_per_region);
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

  EventOptions event_options({.unknown_device_ratio = 0.5,
                              .total_countries = total_countries,
                              .regions_per_country = regions_per_country,
                              .cities_per_region = cities_per_region,
                              .email_events_ratio = 0.5,
                              .phone_events_ratio = 0.5,
                              .proprietary_id_space_1_events_ratio = 0.5,
                              .profile_version_days = 1});

  EventsGenerator generator(events_generator_options);
  for (int i = 0; i < total_events; i++) {
    DataProviderEvent event = generator.GetEvents(event_options);
    SCOPED_TRACE(absl::StrCat(
        "Index: ", i, "\n", "Event: ", event.DebugString()));
    const LabelerInput& labeler_input = event.log_event().labeler_input();
    EXPECT_THAT(labeler_input.event_id().publisher(), IsValidPublisher());
    EXPECT_THAT(labeler_input.event_id().id(), IsValidId());
    EXPECT_THAT(labeler_input.timestamp_usec(),
                IsValidTimestampUsec(current_timestamp));
    EXPECT_THAT(labeler_input.user_agent(), IsValidUserAgent());
    EXPECT_THAT(
        labeler_input.geo(),
        IsValidGeo(total_countries, regions_per_country, cities_per_region));
    EXPECT_THAT(
        labeler_input.profile_info(),
        IsValidProfileInfo(
            total_countries, regions_per_country, cities_per_region));
  }
}

}  // namespace
}  // namespace wfa_virtual_people
