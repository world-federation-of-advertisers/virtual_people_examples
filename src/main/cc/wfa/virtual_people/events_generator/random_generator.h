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

#ifndef SRC_MAIN_CC_WFA_VIRTUAL_PEOPLE_EVENTS_GENERATOR_RANDOM_GENERATOR_H_
#define SRC_MAIN_CC_WFA_VIRTUAL_PEOPLE_EVENTS_GENERATOR_RANDOM_GENERATOR_H_

#include <random>
#include <string>

#include "absl/strings/string_view.h"
#include "absl/time/civil_time.h"

namespace wfa_virtual_people {

// RandomGenerator provides the util methods to generate random values.
class RandomGenerator {
 public:
  // Initialzies the pseudo-random number generator with default seed.
  RandomGenerator() {
    std::random_device rd;
    generator_ = std::mt19937_64(rd());
  }

  // Initializes the pseudo-random number generator to set the seed.
  explicit RandomGenerator(uint32_t seed) : generator_(seed) {}

  // The methods below use the pseudo-random number generator to generate the
  // values.
  //
  // Generates a true/false value.
  // @true_chance is the chance that the output is true.
  bool GetBool(double true_chance);
  // Generates a string composed of digits with the given @length.
  std::string GetDigits(uint32_t length);
  // Generates a string composed of lower case letters with the given @length.
  std::string GetLowerLetters(uint32_t length);
  // Generates a string composed of lower case letters with length between
  // @length_min and @length_max inclusively.
  // The length is selected randomly with Gaussian distribution.
  std::string GetLowerLetters(uint32_t length_min, uint32_t length_max);
  // Generates an integer with value between @min and @max inclusively.
  int32_t GetInteger(int32_t min, int32_t max);
  // Generates a timestamp in microseconds, with value between @n days ago to
  // @current_timestamp inclusively.
  // @current_timestamp is in microseconds.
  uint64_t GetTimestampUsecInNDays(uint64_t current_timestamp, uint32_t n);
  // Generates a date, with value between @n days ago to @current_date
  // inclusively.
  absl::CivilDay GetDateInNDays(absl::CivilDay current_date, uint32_t n);

  // The methods below use the farmhash to generate the values. The outputs are
  // the same when using the same @seed.
  //
  // Generates a timestamp in microseconds, with value between @n days ago to
  // @current_timestamp inclusively.
  // @current_timestamp is in microseconds.
  uint64_t GetTimestampUsecInNDaysWithSeed(uint64_t current_timestamp,
                                           uint32_t n,
                                           absl::string_view seed) const;
  // Generates an integer with value between @min and @max inclusively.
  int32_t GetIntegerWithSeed(int32_t min, int32_t max,
                             absl::string_view seed) const;
  double GetDoubleWithSeed(double min, double max,
                           absl::string_view seed) const;

 private:
  std::mt19937_64 generator_;
};

}  // namespace wfa_virtual_people

#endif  // SRC_MAIN_CC_WFA_VIRTUAL_PEOPLE_EVENTS_GENERATOR_RANDOM_GENERATOR_H_
