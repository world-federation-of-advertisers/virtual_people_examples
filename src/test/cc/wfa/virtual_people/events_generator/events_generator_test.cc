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
using ::testing::MatchesRegex;

// 24 * 3600 * 1000
constexpr uint64_t kMicrosecPerDay = 86400000;

void PublisherSanityCheck(absl::string_view publisher) {
  EXPECT_THAT(publisher, MatchesRegex("[0-9]{8}"));
}

void IdSanityCheck(absl::string_view id) {
  EXPECT_THAT(id, MatchesRegex("[0-9]{16}"));
}

void TimestampUsecSanityCheck(const int64_t timestamp_usec,
                              const int64_t current_timestamp) {
  EXPECT_THAT(timestamp_usec,
              AllOf(Ge(current_timestamp - 30 * kMicrosecPerDay),
                    Le(current_timestamp)));
}

void UserAgentSanityCheck(absl::string_view user_agent) {
  EXPECT_THAT(user_agent,
              AnyOf(MatchesRegex("[a-z]{10}"), MatchesRegex("[0-9]{1,2}")));
}

void GeoSanityCheck(const GeoLocation& geo, const uint32_t total_countries,
                    const uint32_t regions_per_country,
                    const uint32_t cities_per_region) {
  uint32_t country_id = geo.country_id();
  uint32_t region_id = geo.region_id();
  uint32_t city_id = geo.city_id();
  EXPECT_THAT(country_id, AllOf(Ge(100), Le(99 + total_countries)));
  EXPECT_THAT(region_id,
              AllOf(Ge(country_id * 1000),
                    Le(country_id * 1000 + regions_per_country - 1)));
  EXPECT_THAT(city_id, AllOf(Ge(region_id * 1000),
                             Le(region_id * 1000 + cities_per_region - 1)));
}

void UserInfoSanityCheck(const UserInfo& user_info,
                         const uint32_t total_countries,
                         const uint32_t regions_per_country,
                         const uint32_t cities_per_region) {
  EXPECT_THAT(user_info.profile_version(),
              MatchesRegex("[0-9]{4}-[0-9]{2}-[0-9]{2}"));
  EXPECT_THAT(user_info.demo().demo_bucket().age().min_age(),
              AllOf(Ge(0), Le(120)));
  EXPECT_THAT(user_info.demo().demo_bucket().age().max_age(),
              AnyOf(AllOf(Ge(0), Le(120)), Eq(1000)));
  EXPECT_THAT(user_info.demo().confidence(), AllOf(Ge(0.0), Le(1.0)));
  GeoSanityCheck(user_info.home_geo(), total_countries, regions_per_country,
                 cities_per_region);
}

void ProfileInfoSanityCheck(const ProfileInfo& profile_info,
                            const uint32_t total_countries,
                            const uint32_t regions_per_country,
                            const uint32_t cities_per_region) {
  if (profile_info.has_email_user_info()) {
    EXPECT_THAT(profile_info.email_user_info().user_id(),
                MatchesRegex("[a-z]{1,10}@[a-z]{4,8}\\.example.com"));
    UserInfoSanityCheck(profile_info.email_user_info(), total_countries,
                        regions_per_country, cities_per_region);
  }
  if (profile_info.has_phone_user_info()) {
    EXPECT_THAT(profile_info.phone_user_info().user_id(),
                MatchesRegex("\\+\\(555\\)[0-9]{3}-[0-9]{4}"));
    UserInfoSanityCheck(profile_info.phone_user_info(), total_countries,
                        regions_per_country, cities_per_region);
  }
  if (profile_info.has_proprietary_id_space_1_user_info()) {
    EXPECT_THAT(profile_info.proprietary_id_space_1_user_info().user_id(),
                MatchesRegex("[0-9]{16}"));
    UserInfoSanityCheck(profile_info.proprietary_id_space_1_user_info(),
                        total_countries, regions_per_country,
                        cities_per_region);
  }
}

TEST(EventsGeneratorTest, SanityCheck) {
  uint64_t current_timestamp = 1626847100000000;
  uint32_t total_events = 1000;
  uint32_t total_countries = 10;
  uint32_t regions_per_country = 10;
  uint32_t cities_per_region = 10;

  EventsGeneratorOptions events_generator_options({
      current_timestamp,
      /* total_publishers = */ 10,
      total_events,
      /* unknown_device_count = */ 100,
      /* email_users_count = */ 100,
      /* phone_users_count = */ 100,
      /* proprietary_id_space_1_users_count = */ 100});
  
  EventOptions event_options({
      /* unknown_device_ratio = */ 0.5,
      total_countries,
      regions_per_country,
      cities_per_region,
      /* email_events_ratio = */ 0.5,
      /* phone_events_ratio = */ 0.5,
      /* proprietary_id_space_1_events_ratio = */ 0.5,
      /* profile_version_days = */ 1});

  EventsGenerator generator(events_generator_options);
  for (int i = 0; i < total_events; i++) {
    DataProviderEvent event = generator.GetEvents(event_options);
    const LabelerInput& labeler_input = event.log_event().labeler_input();
    PublisherSanityCheck(labeler_input.event_id().publisher());
    IdSanityCheck(labeler_input.event_id().id());
    TimestampUsecSanityCheck(labeler_input.timestamp_usec(), current_timestamp);
    UserAgentSanityCheck(labeler_input.user_agent());
    GeoSanityCheck(labeler_input.geo(), total_countries, regions_per_country,
                   cities_per_region);
    ProfileInfoSanityCheck(labeler_input.profile_info(), total_countries,
                           regions_per_country, cities_per_region);
  }
}

}  // namespace
}  // namespace wfa_virtual_people
