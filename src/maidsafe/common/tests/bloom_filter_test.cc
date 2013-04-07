/* Copyright (c) 2013 maidsafe.net limited
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    * Neither the name of the maidsafe.net limited nor the names of its
    contributors may be used to endorse or promote products derived from this
    software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <algorithm>
#include <chrono>
#include <string>
#include <set>
#include <vector>

#include "maidsafe/common/bloom_filter.h"
#include "maidsafe/common/crypto.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/node_id.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"


namespace maidsafe {

namespace test {

namespace {

std::string MsDuration(const std::chrono::steady_clock::time_point& start,
                       const std::chrono::steady_clock::time_point& stop) {
  return std::to_string(
      std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count()) + " ms";
}

}  // unnamed namespace


class BloomFilterTest : public testing::TestWithParam<size_t> {
 protected:
  BloomFilterTest()
      : kInsertionCount_(GetParam()),
        kCheckCount_(100000),
        bloom_filter_((kInsertionCount_ * 96) / 10),
        inserted_values_(),
        check_values_() {}

  void SetUp() {
    std::set<Identity> all_values;
    while (all_values.size() < kInsertionCount_ + kCheckCount_)
      all_values.insert(Identity(RandomString(crypto::SHA512::DIGESTSIZE)));

    inserted_values_.assign(std::begin(all_values), std::end(all_values));
    std::random_shuffle(std::begin(inserted_values_), std::end(inserted_values_));
    check_values_.assign(std::begin(inserted_values_) + kInsertionCount_,
                         std::end(inserted_values_));
    inserted_values_.erase(std::begin(inserted_values_) + kInsertionCount_,
                           std::end(inserted_values_));
  }

  const size_t kInsertionCount_, kCheckCount_;
  BloomFilter bloom_filter_;
  std::vector<Identity> inserted_values_, check_values_;
};

TEST_P(BloomFilterTest, BEH_All) {
  auto start(std::chrono::steady_clock::now());
  for (const auto& inserted_value : inserted_values_)
    bloom_filter_.Insert(inserted_value);
  auto stop(std::chrono::steady_clock::now());
  std::string duration1(MsDuration(start, stop));

  for (const auto& inserted_value : inserted_values_)
    ASSERT_TRUE(bloom_filter_.ProbablyContains(inserted_value));

  size_t false_positive(0);
  start = std::chrono::steady_clock::now();
  for (const auto& check_value : check_values_) {
    if (bloom_filter_.ProbablyContains(check_value))
      ++false_positive;
  }
  stop = std::chrono::steady_clock::now();
  std::string duration2(MsDuration(start, stop));
  double false_positive_rate(static_cast<double>(false_positive) / kCheckCount_);
  EXPECT_GT(1.5, false_positive_rate);

  LOG(kInfo) << "Filter size:               " << bloom_filter_.BitCapacity() / 8 << " bytes.";
  LOG(kInfo) << "Number of hash functions:  " << BloomFilter::kHashFunctionsCount;
  LOG(kInfo) << "Time to insert:            " << duration1 << " for " << kInsertionCount_
             << " values: ";
  LOG(kInfo) << "Time to check:             " << duration2 << " for " << kCheckCount_
             << " values: ";
  LOG(kInfo) << "Set bit count:             " << bloom_filter_.BitsetCount();
  LOG(kInfo) << "Estimated insertion count: " << bloom_filter_.InsertionCountEstimate();
  LOG(kInfo) << "Actual false positives:    " << false_positive << " ("
             << false_positive_rate * 100 << "%)";
  LOG(kInfo) << "Calculated false +ve rate: " << bloom_filter_.FalsePositiveRateEstimate() * 100
             << "%";
}

INSTANTIATE_TEST_CASE_P(BloomFilterAll, BloomFilterTest, testing::Values(100, 1000, 10000, 100000));

}  // namespace test

}  // namespace maidsafe
