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

#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace wfa_virtual_people {
namespace {

using ::testing::AllOf;
using ::testing::Ge;
using ::testing::Le;
using ::testing::MatchesRegex;

constexpr int kRepeatNumber = 1000;

TEST(RandomGeneratorTest, GetDigitsSanityCheck) {
  RandomGenerator generator;
  for (int i = 0; i < kRepeatNumber; i++) {
    std::string output = generator.GetDigits(16);
    EXPECT_THAT(output, MatchesRegex("[0-9]{16}"));
  }
}

TEST(RandomGeneratorTest, GetLowerLettersFixedLengthSanityCheck) {
  RandomGenerator generator;
  for (int i = 0; i < kRepeatNumber; i++) {
    std::string output = generator.GetLowerLetters(10);
    EXPECT_THAT(output, MatchesRegex("[a-z]{10}"));
  }
}

TEST(RandomGeneratorTest, GetLowerLettersLengthRangeSanityCheck) {
  RandomGenerator generator;
  for (int i = 0; i < kRepeatNumber; i++) {
    std::string output = generator.GetLowerLetters(5, 10);
    EXPECT_THAT(output, MatchesRegex("[a-z]{5,10}"));
  }
}

TEST(RandomGeneratorTest, GetIntegerSanityCheck) {
  RandomGenerator generator;
  for (int i = 0; i < kRepeatNumber; i++) {
    int32_t output = generator.GetInteger(10, 20);
    EXPECT_THAT(output, AllOf(Ge(10), Le(20)));
  }
}

TEST(RandomGeneratorTest, GetTimestampUsecInNDaysSanityCheck) {
  RandomGenerator generator;
  for (int i = 0; i < kRepeatNumber; i++) {
    uint64_t output = generator.GetTimestampUsecInNDays(1626847100000000, 30);
    EXPECT_THAT(output, AllOf(Ge(1626847100000000 - (uint64_t)30 * 86400000),
                              Le(1626847100000000)));
  }
}

TEST(RandomGeneratorTest, GetDateInNDaysSanityCheck) {
  RandomGenerator generator;
  for (int i = 0; i < kRepeatNumber; i++) {
    absl::CivilDay output =
        generator.GetDateInNDays(absl::CivilDay(2021, 9, 20), 10);
    EXPECT_THAT(output, AllOf(Ge(absl::CivilDay(2021, 9, 10)),
                              Le(absl::CivilDay(2021, 9, 20))));
  }
}

TEST(RandomGeneratorTest, GetTimestampUsecInNDaysWithSeedSanityCheck) {
  RandomGenerator generator;
  for (int i = 0; i < kRepeatNumber; i++) {
    uint64_t output = generator.GetTimestampUsecInNDaysWithSeed(
        1626847100000000, 30, std::to_string(i));
    EXPECT_THAT(output, AllOf(Ge(1626847100000000 - (uint64_t)30 * 86400000),
                              Le(1626847100000000)));
  }
}

TEST(RandomGeneratorTest, GetTimestampUsecInNDaysWithSeedDeterministicCheck) {
  RandomGenerator generator;
  for (int i = 0; i < kRepeatNumber; i++) {
    EXPECT_EQ(generator.GetTimestampUsecInNDaysWithSeed(1626847100000000, 30,
                                                        std::to_string(i)),
              generator.GetTimestampUsecInNDaysWithSeed(1626847100000000, 30,
                                                        std::to_string(i)));
  }
}

TEST(RandomGeneratorTest, GetIntegerWithSeedSanityCheck) {
  RandomGenerator generator;
  for (int i = 0; i < kRepeatNumber; i++) {
    int32_t output = generator.GetIntegerWithSeed(10, 20, std::to_string(i));
    EXPECT_THAT(output, AllOf(Ge(10), Le(20)));
  }
}

TEST(RandomGeneratorTest, GetIntegerWithSeedDeterministicCheck) {
  RandomGenerator generator;
  for (int i = 0; i < kRepeatNumber; i++) {
    EXPECT_EQ(generator.GetIntegerWithSeed(10, 20, std::to_string(i)),
              generator.GetIntegerWithSeed(10, 20, std::to_string(i)));
  }
}

TEST(RandomGeneratorTest, GetDoubleWithSeedSanityCheck) {
  RandomGenerator generator;
  for (int i = 0; i < kRepeatNumber; i++) {
    int32_t output = generator.GetDoubleWithSeed(1.0, 2.0, std::to_string(i));
    EXPECT_THAT(output, AllOf(Ge(1.0), Le(2.0)));
  }
}

TEST(RandomGeneratorTest, GetDoubleWithSeedDeterministicCheck) {
  RandomGenerator generator;
  for (int i = 0; i < kRepeatNumber; i++) {
    EXPECT_EQ(generator.GetDoubleWithSeed(1.0, 2.0, std::to_string(i)),
              generator.GetDoubleWithSeed(1.0, 2.0, std::to_string(i)));
  }
}

}  // namespace
}  // namespace wfa_virtual_people
