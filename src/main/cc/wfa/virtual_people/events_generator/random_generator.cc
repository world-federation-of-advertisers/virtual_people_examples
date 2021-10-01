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

// The correspondence between the code and the string composed of lower case
// letters is
// code                                string
// 1 - 26                              a - z
// 26+1 - 26+26^2                      aa - zz
// 26+26^2+1 - 26+26^2+26^3            aaa - zzz
// ......
// If we have
// cut(n) = (26^n - 1) * 26 / 25
// Then the correspondence will be
// code                                string
// cut(0)+1 - cut(1)                   a - z
// cut(1)+1 - cut(2)                   aa - zz
// cut(2)+1 - cut(3)                   aaa - zzz

// Gets the cut code.
uint64_t GetLowerLettersStringCodeCut(uint32_t n) {
  return (int_pow(26, n) - 1) * 26 / 25;
}

// Converts the @code to a string composed of lower case letters.
std::string ConvertToLowerLetters(uint64_t code) {
  CHECK(code > 0) << "Code must be positive.";
  uint32_t length = 1;
  while (code > GetLowerLettersStringCodeCut(length)) {
    ++length;
    CHECK(length <= 13) << "Code is too large.";
  }
  code -= GetLowerLettersStringCodeCut(length - 1);
  --code;
  // Now the code is in range 0 - 26^length-1.
  std::string output;
  for (int i = 0; i < length; ++i) {
    char c = 'a' + code % 26;
    output = c + output;
    code /= 26;
  }
  return output;
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
  CHECK(length <= 13) << "The max length is 13.";
  uint64_t min = GetLowerLettersStringCodeCut(length - 1) + 1;
  uint64_t max = GetLowerLettersStringCodeCut(length);
  uint64_t random = absl::Uniform(absl::IntervalClosed, generator_, min, max);
  return ConvertToLowerLetters(random);
}

std::string RandomGenerator::GetLowerLetters(const uint32_t length_min,
                                             const uint32_t length_max) {
  CHECK(length_max <= 13) << "The max length is 13.";
  uint64_t min = GetLowerLettersStringCodeCut(length_min - 1) + 1;
  uint64_t max = GetLowerLettersStringCodeCut(length_max);
  uint64_t random = absl::Uniform(absl::IntervalClosed, generator_, min, max);
  return ConvertToLowerLetters(random);
}

int32_t RandomGenerator::GetInteger(const int32_t min, const int32_t max) {
  CHECK(min <= max) << "max must be no less than min.";
  return absl::Uniform(absl::IntervalClosed, generator_, min, max);
}

uint64_t RandomGenerator::GetTimestampUsecInNDays(
    const uint64_t current_timestamp, const uint32_t n) {
  CHECK(n <= 10000) << "N should be at most 10000.";
  return absl::Uniform(
      absl::IntervalClosed, generator_,
      current_timestamp - n * kMicrosecPerDay, current_timestamp);
}

absl::CivilDay RandomGenerator::GetDateInNDays(absl::CivilDay current_date,
                                               const uint32_t n) {
  CHECK(n <= 10000) << "N should be at most 10000.";
  uint32_t days = absl::Uniform(
      absl::IntervalClosed, generator_, (uint32_t)0, n);
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
  double rate =
      (double)fingerprint / (double)std::numeric_limits<uint64_t>::max();
  return min + (max - min) * rate;
}

}  // namespace wfa_virtual_people
