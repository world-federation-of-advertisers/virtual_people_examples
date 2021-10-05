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

#include "wfa/virtual_people/events_generator/random_generator.h"

#include <math.h>

#include <random>
#include <string>

#include "absl/random/distributions.h"
#include "absl/strings/string_view.h"
#include "absl/time/civil_time.h"
#include "glog/logging.h"
#include "src/farmhash.h"

namespace wfa_virtual_people {

namespace {

// 24 * 3600 * 1000
constexpr uint64_t kMicrosecPerDay = 86400000;

// Returns power(base, exp).
// We have to do the naive power to avoid overflow.
uint64_t int_pow(uint32_t base, uint32_t exp) {
  uint64_t result = 1;
  for (int i = 0; i < exp; ++i) {
    result *= base;
  }
  return result;
}

}  // namespace

bool RandomGenerator::GetBool(const double true_chance) {
  CHECK(true_chance >= 0.0 && true_chance <= 1.0)
      << "True chance must be between 0 and 1.";
  double random = absl::Uniform(absl::IntervalClosed, generator_, 0.0, 1.0);
  return random < true_chance;
}

std::string RandomGenerator::GetDigits(const uint32_t length) {
  CHECK(length <= 18) << "The max length is 18.";
  uint64_t min = int_pow(10, length - 1);
  uint64_t max = int_pow(10, length) - 1;
  uint64_t random = absl::Uniform(absl::IntervalClosed, generator_, min, max);
  return std::to_string(random);
}

std::string RandomGenerator::GetLowerLetters(const uint32_t length) {
  std::string output(length, ' ');
  for (int i = 0; i < length; ++i) {
    output[i] = absl::Uniform<char>(absl::IntervalClosed, generator_, 'a', 'z');
  }
  return std::string(output);
}

std::string RandomGenerator::GetLowerLetters(const uint32_t length_min,
                                             const uint32_t length_max) {
  CHECK(length_min <= length_max)
      << "length_max cannot be less than length_min.";
  double mean = (length_min + length_max + 1.0) / 2.0;
  double stddev = (mean - length_min) / 3.0;
  double random = absl::Gaussian(generator_, mean, stddev);
  int32_t length = static_cast<int32_t>(random);
  if (length < length_min) length = length_min;
  if (length > length_max) length = length_max;
  return GetLowerLetters(length);
}

int32_t RandomGenerator::GetInteger(const int32_t min, const int32_t max) {
  CHECK(min <= max) << "max must be no less than min.";
  return absl::Uniform(absl::IntervalClosed, generator_, min, max);
}

uint64_t RandomGenerator::GetTimestampUsecInNDays(
    const uint64_t current_timestamp, const uint32_t n) {
  CHECK(n <= 10000) << "N should be at most 10000.";
  return absl::Uniform(absl::IntervalClosed, generator_,
                       current_timestamp - n * kMicrosecPerDay,
                       current_timestamp);
}

absl::CivilDay RandomGenerator::GetDateInNDays(absl::CivilDay current_date,
                                               const uint32_t n) {
  CHECK(n <= 10000) << "N should be at most 10000.";
  uint32_t days =
      absl::Uniform(absl::IntervalClosed, generator_, uint32_t{0}, n);
  return current_date - days;
}

uint64_t RandomGenerator::GetTimestampUsecInNDaysWithSeed(
    const uint64_t current_timestamp, const uint32_t n,
    absl::string_view seed) const {
  CHECK(n <= 10000) << "N should be at most 10000.";
  uint64_t fingerprint = util::Fingerprint64(seed);
  return current_timestamp - fingerprint % (n * kMicrosecPerDay + 1);
}

int32_t RandomGenerator::GetIntegerWithSeed(const int32_t min,
                                            const int32_t max,
                                            absl::string_view seed) const {
  CHECK(min <= max) << "max must be no less than min.";
  uint64_t fingerprint = util::Fingerprint64(seed);
  return min + fingerprint % (max - min + 1);
}

double RandomGenerator::GetDoubleWithSeed(const double min, const double max,
                                          absl::string_view seed) const {
  CHECK(min <= max) << "max must be no less than min.";
  uint64_t fingerprint = util::Fingerprint64(seed);
  double rate = static_cast<double>(fingerprint) /
                static_cast<double>(std::numeric_limits<uint64_t>::max());
  return min + (max - min) * rate;
}

}  // namespace wfa_virtual_people
