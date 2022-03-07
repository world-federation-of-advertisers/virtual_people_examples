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
#include <unordered_set>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/time/civil_time.h"
#include "absl/time/time.h"
#include "glog/logging.h"
#include "wfa/virtual_people/common/demographic.pb.h"
#include "wfa/virtual_people/common/event.pb.h"
#include "wfa/virtual_people/common/geo_location.pb.h"
#include "wfa/virtual_people/events_generator/random_generator.h"

namespace wfa_virtual_people {

namespace {

// Converts a timestamp in microseconds to a CivilDay, using UTC time zone.
absl::CivilDay ConvertToDay(const uint64_t timestamp_usec) {
  absl::Time time = absl::FromUnixMicros(timestamp_usec);
  return absl::ToCivilDay(time, absl::UTCTimeZone());
}

// Converts a CivilDay to a timestamp in microseconds, using UTC time zone.
uint64_t ConvertToTimestampUsec(absl::CivilDay day) {
  absl::Time t = absl::FromCivil(day, absl::UTCTimeZone());
  return absl::ToUnixMicros(t);
}

DemoInfo GetUserInfoDemo(RandomGenerator& random_generator,
                         absl::string_view seed_prefix) {
  DemoInfo demo;

  // Sets gender.
  int32_t gender = random_generator.GetIntegerWithSeed(
      0, 2, absl::StrCat(seed_prefix, "_demo_gender"));
  demo.mutable_demo_bucket()->set_gender(static_cast<Gender>(gender));

  // Sets age.
  int32_t min_age = random_generator.GetIntegerWithSeed(
      0, 120, absl::StrCat(seed_prefix, "_demo_age"));
  int32_t max_age = random_generator.GetIntegerWithSeed(
      0, 121, absl::StrCat(seed_prefix, "_demo_age"));
  if (max_age == 121) {
    max_age = 1000;
  }
  if (min_age > max_age) {
    std::swap(min_age, max_age);
  }
  demo.mutable_demo_bucket()->mutable_age()->set_min_age(min_age);
  demo.mutable_demo_bucket()->mutable_age()->set_max_age(max_age);

  // Sets confidence.
  double confidence = random_generator.GetDoubleWithSeed(
      0.0, 1.0, absl::StrCat(seed_prefix, "_demo_confidence"));
  demo.set_confidence(confidence);

  return demo;
}

GeoLocation GetHomeGeo(RandomGenerator& random_generator,
                       const uint32_t total_countries,
                       const uint32_t regions_per_country,
                       const uint32_t cities_per_region,
                       absl::string_view seed_prefix) {
  CHECK(total_countries >= 1 && total_countries <= 900)
      << "total_countries must be between 1 and 900.";
  CHECK(regions_per_country >= 1 && regions_per_country <= 1000)
      << "regions_per_country must be between 1 and 1000.";
  CHECK(cities_per_region >= 1 && cities_per_region <= 1000)
      << "cities_per_region must be between 1 and 1000.";
  int32_t country_id = random_generator.GetIntegerWithSeed(
      100, 99 + total_countries,
      absl::StrCat(seed_prefix, "_home_geo_country"));
  int32_t region_id =
      country_id * 1000 + random_generator.GetIntegerWithSeed(
                              0, regions_per_country - 1,
                              absl::StrCat(seed_prefix, "_home_geo_region"));
  int32_t city_id =
      region_id * 1000 + random_generator.GetIntegerWithSeed(
                             0, cities_per_region - 1,
                             absl::StrCat(seed_prefix, "_home_geo_city"));
  GeoLocation geo;
  geo.set_country_id(country_id);
  geo.set_region_id(region_id);
  geo.set_city_id(city_id);
  return geo;
}

UserInfo GetUserInfo(RandomGenerator& random_generator,
                     const std::vector<std::string>& user_id_pool,
                     absl::CivilDay current_day,
                     const uint32_t profile_version_days,
                     const uint32_t total_countries,
                     const uint32_t regions_per_country,
                     const uint32_t cities_per_region) {
  UserInfo user_info;

  // Sets user_id.
  int32_t index = random_generator.GetInteger(0, user_id_pool.size() - 1);
  user_info.set_user_id(user_id_pool.at(index));

  // Sets profile_version.
  absl::CivilDay profile_version =
      random_generator.GetDateInNDays(current_day, profile_version_days);
  user_info.set_profile_version(absl::FormatCivilTime(profile_version));

  std::string seed_prefix =
      absl::StrCat(user_info.user_id(), "_", user_info.profile_version());

  // Sets demo.
  *user_info.mutable_demo() = GetUserInfoDemo(random_generator, seed_prefix);

  // Sets home_geo.
  *user_info.mutable_home_geo() =
      GetHomeGeo(random_generator, total_countries, regions_per_country,
                 cities_per_region, seed_prefix);

  // Sets creation_time_usec.
  uint64_t creation_time_usec =
      random_generator.GetTimestampUsecInNDaysWithSeed(
          ConvertToTimestampUsec(profile_version), 1000,
          absl::StrCat(seed_prefix, "_creation_time"));
  user_info.set_creation_time_usec(creation_time_usec);

  return user_info;
}

}  // namespace

void EventsGenerator::BuildEventIdPool(const uint32_t total_publishers,
                                       const uint32_t total_events) {
  CHECK(total_publishers > 0 && total_publishers <= 100)
      << "total_publishers must be a positive integer no larger than 100.";
  CHECK(total_events > 0 && total_events <= 1000000)
      << "total_events must be a positive integer no larger than 1000000.";
  // A set of existing publishers.
  absl::flat_hash_set<std::string> publishers;
  // A set of existing ids.
  absl::flat_hash_set<std::string> ids;
  while (publishers.size() < total_publishers) {
    std::string publisher = random_generator_.GetDigits(8);
    auto [publisher_itr, publisher_inserted] = publishers.insert(publisher);
    if (!publisher_inserted) continue;
    // Gets the total count of events for this publisher.
    uint32_t events_for_publisher = total_events / total_publishers;
    if (publishers.size() <= total_events % total_publishers) {
      ++events_for_publisher;
    }
    uint32_t event_count = 0;
    while (event_count < events_for_publisher) {
      std::string id = random_generator_.GetDigits(16);
      auto [id_itr, id_inserted] = ids.insert(id);
      if (!id_inserted) continue;
      ++event_count;
      event_id_pool_.push_back(PublisherEventId({publisher, id}));
    }
  }
}

void EventsGenerator::BuildUnknownDevicePool(
    const uint32_t unknown_device_count) {
  CHECK(unknown_device_count > 0 && unknown_device_count <= 10000)
      << "unknown_device_count must be a positive integer no larger than "
         "10000.";
  absl::flat_hash_set<std::string> unknown_devices;
  while (unknown_devices.size() < unknown_device_count) {
    std::string unknown_device = random_generator_.GetLowerLetters(10);
    auto [itr, inserted] = unknown_devices.insert(unknown_device);
    if (!inserted) continue;
    unknown_device_pool_.push_back(unknown_device);
  }
}

void EventsGenerator::BuildEmailPool(const uint32_t email_users_count) {
  CHECK(email_users_count > 0 && email_users_count <= 10000)
      << "email_users_count must be a positive integer no larger than 10000.";
  absl::flat_hash_set<std::string> emails;
  while (emails.size() < email_users_count) {
    std::string email =
        absl::StrCat(random_generator_.GetLowerLetters(1, 10), "@",
                     random_generator_.GetLowerLetters(4, 8), ".example.com");
    auto [itr, inserted] = emails.insert(email);
    if (!inserted) continue;
    email_pool_.push_back(email);
  }
}

void EventsGenerator::BuildPhonePool(const uint32_t phone_users_count) {
  CHECK(phone_users_count > 0 && phone_users_count <= 10000)
      << "phone_users_count must be a positive integer no larger than 10000.";
  absl::flat_hash_set<std::string> phones;
  while (phones.size() < phone_users_count) {
    std::string phone = absl::StrCat("+(555)", random_generator_.GetDigits(3),
                                     "-", random_generator_.GetDigits(4));
    auto [itr, inserted] = phones.insert(phone);
    if (!inserted) continue;
    phone_pool_.push_back(phone);
  }
}

void EventsGenerator::BuildProprietaryIdSpace1Pool(
    const uint32_t proprietary_id_space_1_users_count) {
  CHECK(proprietary_id_space_1_users_count > 0 &&
        proprietary_id_space_1_users_count <= 10000)
      << "proprietary_id_space_1_users_count must be a positive integer no "
         "larger than 10000.";
  absl::flat_hash_set<std::string> proprietary_id_space_1s;
  while (proprietary_id_space_1s.size() < proprietary_id_space_1_users_count) {
    std::string proprietary_id_space_1 = random_generator_.GetDigits(16);
    auto [itr, inserted] =
        proprietary_id_space_1s.insert(proprietary_id_space_1);
    if (!inserted) continue;
    proprietary_id_space_1_pool_.push_back(proprietary_id_space_1);
  }
}

void EventsGenerator::Initialize(
    const uint32_t total_publishers, const uint32_t total_events,
    const uint32_t unknown_device_count, const uint32_t email_users_count,
    const uint32_t phone_users_count,
    const uint32_t proprietary_id_space_1_users_count) {
  BuildEventIdPool(total_publishers, total_events);
  BuildUnknownDevicePool(unknown_device_count);
  BuildEmailPool(email_users_count);
  BuildPhonePool(phone_users_count);
  BuildProprietaryIdSpace1Pool(proprietary_id_space_1_users_count);
}

EventsGenerator::EventsGenerator(const EventsGeneratorOptions& options)
    : current_timestamp_(options.current_timestamp),
      current_day_(ConvertToDay(options.current_timestamp)) {
  Initialize(options.total_publishers, options.total_events,
             options.unknown_device_count, options.email_users_count,
             options.phone_users_count,
             options.proprietary_id_space_1_users_count);
}

EventsGenerator::EventsGenerator(const EventsGeneratorOptions& options,
                                 const uint32_t seed)
    : random_generator_(seed),
      current_timestamp_(options.current_timestamp),
      current_day_(ConvertToDay(options.current_timestamp)) {
  Initialize(options.total_publishers, options.total_events,
             options.unknown_device_count, options.email_users_count,
             options.phone_users_count,
             options.proprietary_id_space_1_users_count);
}

EventId EventsGenerator::GetEventId() {
  CHECK(!event_id_pool_.empty()) << "All event ids are used.";
  PublisherEventId event_id = event_id_pool_.back();
  event_id_pool_.pop_back();
  EventId output;
  output.set_publisher(event_id.publisher);
  output.set_id(event_id.id);
  return output;
}

std::string EventsGenerator::GetDevice(const double unknown_device_ratio) {
  CHECK(unknown_device_ratio >= 0.0 && unknown_device_ratio <= 1.0)
      << "unknown_device_ratio must be between 0 and 1.";
  bool is_unknown = random_generator_.GetBool(unknown_device_ratio);
  if (is_unknown) {
    int index =
        random_generator_.GetInteger(0, unknown_device_pool_.size() - 1);
    return unknown_device_pool_.at(index);
  }
  return std::to_string(random_generator_.GetInteger(0, 99));
}

GeoLocation EventsGenerator::GetGeo(const uint32_t total_countries,
                                    const uint32_t regions_per_country,
                                    const uint32_t cities_per_region) {
  CHECK(total_countries >= 1 && total_countries <= 900)
      << "total_countries must be between 1 and 900.";
  CHECK(regions_per_country >= 1 && regions_per_country <= 1000)
      << "regions_per_country must be between 1 and 1000.";
  CHECK(cities_per_region >= 1 && cities_per_region <= 1000)
      << "cities_per_region must be between 1 and 1000.";
  int32_t country_id = random_generator_.GetInteger(100, 99 + total_countries);
  int32_t region_id = country_id * 1000 +
                      random_generator_.GetInteger(0, regions_per_country - 1);
  int32_t city_id =
      region_id * 1000 + random_generator_.GetInteger(0, cities_per_region - 1);
  GeoLocation geo;
  geo.set_country_id(country_id);
  geo.set_region_id(region_id);
  geo.set_city_id(city_id);
  return geo;
}

ProfileInfo EventsGenerator::GetProfileInfo(const ProfileInfoOptions& options) {
  CHECK(options.email_events_ratio >= 0.0 && options.email_events_ratio <= 1.0)
      << "email_events_ratio must be between 0 and 1.";
  CHECK(options.phone_events_ratio >= 0.0 && options.phone_events_ratio <= 1.0)
      << "phone_events_ratio must be between 0 and 1.";
  CHECK(options.proprietary_id_space_1_events_ratio >= 0.0 &&
        options.proprietary_id_space_1_events_ratio <= 1.0)
      << "proprietary_id_space_1_events_ratio must be between 0 and 1.";
  CHECK(options.profile_version_days <= 3)
      << "profile_version_days must be no larger than 3.";

  ProfileInfo profile_info;
  if (random_generator_.GetBool(options.email_events_ratio)) {
    *profile_info.mutable_email_user_info() =
        GetUserInfo(random_generator_, email_pool_, current_day_,
                    options.profile_version_days, options.total_countries,
                    options.regions_per_country, options.cities_per_region);
  }
  if (random_generator_.GetBool(options.phone_events_ratio)) {
    *profile_info.mutable_phone_user_info() =
        GetUserInfo(random_generator_, phone_pool_, current_day_,
                    options.profile_version_days, options.total_countries,
                    options.regions_per_country, options.cities_per_region);
  }
  if (random_generator_.GetBool(options.proprietary_id_space_1_events_ratio)) {
    *profile_info.mutable_proprietary_id_space_1_user_info() = GetUserInfo(
        random_generator_, proprietary_id_space_1_pool_, current_day_,
        options.profile_version_days, options.total_countries,
        options.regions_per_country, options.cities_per_region);
  }
  return profile_info;
}

DataProviderEvent EventsGenerator::GetEvent(const EventOptions& options) {
  DataProviderEvent event;
  LabelerInput* labeler_input =
      event.mutable_log_event()->mutable_labeler_input();

  *labeler_input->mutable_event_id() = GetEventId();

  labeler_input->set_timestamp_usec(
      random_generator_.GetTimestampUsecInNDays(current_timestamp_, 30));

  labeler_input->set_user_agent(GetDevice(options.unknown_device_ratio));

  *labeler_input->mutable_geo() =
      GetGeo(options.total_countries, options.regions_per_country,
             options.cities_per_region);

  *labeler_input->mutable_profile_info() =
      GetProfileInfo({options.email_events_ratio, options.phone_events_ratio,
                      options.proprietary_id_space_1_events_ratio,
                      options.profile_version_days, options.total_countries,
                      options.regions_per_country, options.cities_per_region});

  return event;
}

}  // namespace wfa_virtual_people
