/*  Copyright 2014 MaidSafe.net limited

    This MaidSafe Software is licensed to you under (1) the MaidSafe.net Commercial License,
    version 1.0 or later, or (2) The General Public License (GPL), version 3, depending on which
    licence you accepted on initial access to the Software (the "Licences").

    By contributing code to the MaidSafe Software, or to this project generally, you agree to be
    bound by the terms of the MaidSafe Contributor Agreement, version 1.0, found in the root
    directory of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also
    available at: http://www.maidsafe.net/licenses

    Unless required by applicable law or agreed to in writing, the MaidSafe Software distributed
    under the GPL Licence is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
    OF ANY KIND, either express or implied.

    See the Licences for the specific language governing permissions and limitations relating to
    use of the MaidSafe Software.                                                                 */
#include <forward_list>
#include <initializer_list>
#include <list>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "boost/range/adaptor/map.hpp"
#include "boost/range/iterator_range.hpp"
#include "boost/utility/string_ref.hpp"

#include "cereal/cereal.hpp"
#include "cereal/archives/binary.hpp"

#include "maidsafe/common/hash.h"
#include "maidsafe/common/test.h"

namespace {
struct IgnoreInternalSerialize {
  int one, two, three;

  template<typename Archive>
  void serialize(Archive& archive) {
    archive(three, two, one);
  }

  template<typename HashAlgorithm>
  void HashAppend(HashAlgorithm& hash) const {
    hash(one, two, three);
  }
};

struct UseInternalSerializeWithVersion {
  const static bool use_serialize_for_hashing = true;  //NOLINT
  int one, two, three;

  template<typename Archive>
  void serialize(Archive& archive, const unsigned version) {
    EXPECT_EQ(0u, version);
    archive & one & two & three;
  }
};

struct UseInternalSerializeWithoutVersion {
  const static bool use_serialize_for_hashing = true;  //NOLINT
  int one, two, three;

  template<typename Archive>
  void serialize(Archive& archive) {
    archive & one & two & three;
  }
};

struct IgnoreExternalSerialize {
  int one, two, three;
};

struct UseExternalSerializeWithVersion {
  const static bool use_serialize_for_hashing = true;  //NOLINT
  int one, two, three;
};

struct UseExternalSerializeWithoutVersion {
  int one, two, three;
};

template<typename Archive>
void serialize(Archive& archive, IgnoreExternalSerialize& value) {
  archive(value.three, value.two, value.one);
}

template<typename HashAlgorithm>
void HashAppend(HashAlgorithm& hash, const IgnoreExternalSerialize& value) {
  hash(value.one, value.two, value.three);
}

template<typename Archive>
void serialize(
    Archive& archive, UseExternalSerializeWithVersion& value, const unsigned version) {
  EXPECT_EQ(10u, version);
  archive & value.one & value.two & value.three;
}

template<typename Archive>
void serialize(Archive& archive, UseExternalSerializeWithoutVersion& value) {
  archive & value.one & value.two & value.three;
}

template<typename CustomType>
void TestCustomType() {
  const maidsafe::SeededHash<maidsafe::SipHash> hash{};
  CustomType original = {-500, 5000, 50000};
  const std::uint64_t original_hash = hash(original);

  std::string serialized;
  {
    std::ostringstream out;
    {
      cereal::BinaryOutputArchive archive(out);
      archive(original);
    }
    serialized = out.str();
  }

  CustomType copy{0, 0, 0};
  EXPECT_NE(original_hash, hash(copy));
  {
    std::istringstream in(serialized);
    cereal::BinaryInputArchive archive(in);
    archive(copy);
  }
  EXPECT_EQ(original_hash, hash(copy));
}

}  // namespace

MAIDSAFE_HASH_AND_CEREAL_CLASS_VERSION(UseExternalSerializeWithVersion, 10)

namespace maidsafe {

template<>
struct UseSerializeForHashing<UseExternalSerializeWithoutVersion>
  : std::true_type {};

namespace test {

TEST(HashTest, BEH_NumericRange) {
  const maidsafe::SeededHash<maidsafe::SipHash> hash{};
  const int data[] = {10, 20, 30};
  const std::uint64_t reference = hash(data);

  EXPECT_EQ(reference, hash(10, 20, 30, std::size_t(3)));
  EXPECT_EQ(reference, hash(std::array<int, 3>({{10, 20, 30}})));
  EXPECT_EQ(
      reference,
      hash(std::map<int, int>({{1, 10}, {2, 20}, {3, 30}}) | boost::adaptors::map_values));
  EXPECT_EQ(reference, hash(std::forward_list<int>({10, 20, 30})));
  EXPECT_EQ(reference, hash(std::list<int>({10, 20, 30})));
  EXPECT_EQ(reference, hash(std::initializer_list<int>({10, 20, 30})));
  EXPECT_EQ(reference, hash(std::set<int>({10, 20, 30})));
  EXPECT_EQ(reference, hash(std::vector<int>({10, 20, 30})));
}

TEST(HashTest, BEH_PairVerification) {
  {
    maidsafe::SipHash hash1{{{}}};
    maidsafe::SipHash hash2{{{}}};

    hash1(100, 1000);
    hash2(std::make_pair(100, 1000));

    EXPECT_EQ(hash1.Finalize(), hash2.Finalize());
  }
  {
    maidsafe::SipHash hash1{{{}}};
    maidsafe::SipHash hash2{{{}}};

    hash1(boost::string_ref("<-->"), 100.5);
    hash2(std::make_pair(boost::string_ref("<-->"), 100.5));

    EXPECT_EQ(hash1.Finalize(), hash2.Finalize());
  }
}

TEST(HashTest, BEH_PairRange) {
  const maidsafe::SeededHash<maidsafe::SipHash> hash{};
  const std::initializer_list<std::pair<int, int>> data = {{3, 10}, {50, 20}, {1000, 30}};
  const std::uint64_t reference = hash(data);

  EXPECT_EQ(reference, hash(std::forward_list<std::pair<int, int>>(data)));
  EXPECT_EQ(reference, hash(std::list<std::pair<int, int>>(data)));
  EXPECT_EQ(reference, hash(std::map<int, int>({{3, 10}, {50, 20}, {1000, 30}})));
  EXPECT_EQ(reference, hash(std::set<std::pair<int, int>>(data)));
  EXPECT_EQ(reference, hash(std::vector<std::pair<int, int>>(data)));

  EXPECT_EQ(
      reference,
      hash(std::forward_list<std::tuple<int, int>>({
          std::make_tuple(3, 10), std::make_tuple(50, 20), std::make_tuple(1000, 30) })));
}

TEST(HashTest, BEH_TupleVerification) {
  {
    const maidsafe::SipHash hash1{{{}}};
    maidsafe::SipHash hash2{{{}}};
    hash2(std::tuple<>{});
    EXPECT_EQ(hash1.Finalize(), hash2.Finalize());
  }
  {
    maidsafe::SipHash hash1{{{}}};
    maidsafe::SipHash hash2{{{}}};

    hash1(100, 1000, 10000);
    hash2(std::make_tuple(100, 1000, 10000));

    EXPECT_EQ(hash1.Finalize(), hash2.Finalize());
  }
  {
    maidsafe::SipHash hash1{{{}}};
    maidsafe::SipHash hash2{{{}}};

    hash1(boost::string_ref("<-->"), 100.5, 90000);
    hash2(std::make_tuple(boost::string_ref("<-->"), 100.5, 90000));

    EXPECT_EQ(hash1.Finalize(), hash2.Finalize());
  }
}

TEST(HashTest, BEH_TupleRange) {
  const maidsafe::SeededHash<maidsafe::SipHash> hash{};
  const std::initializer_list<std::tuple<int, int, int>> data {
     std::make_tuple(3, 10, 1000),
     std::make_tuple(50, 20, 122),
     std::make_tuple(1000, 30, 33)
  };
  const std::uint64_t reference = hash(data);

  EXPECT_EQ(reference, hash(std::forward_list<std::tuple<int, int, int>>(data)));
  EXPECT_EQ(reference, hash(std::list<std::tuple<int, int, int>>(data)));
  EXPECT_EQ(reference, hash(std::vector<std::tuple<int, int, int>>(data)));
}

TEST(HashTest, BEH_FloatRange) {
  const maidsafe::SeededHash<maidsafe::SipHash> hash{};
  const std::initializer_list<std::pair<int, float>> data =
      { {3, 10.4f}, {50, -0.f}, {1000, 30.2f} };
  const std::uint64_t reference = hash(data);

  EXPECT_EQ(reference, hash(std::forward_list<std::pair<int, float>>(data)));
  EXPECT_EQ(reference, hash(std::list<std::pair<int, float>>(data)));
  EXPECT_EQ(reference, hash(std::map<int, float>({{3, 10.4f}, {50, -0.f}, {1000, 30.2f}})));
  EXPECT_EQ(reference, hash(std::set<std::pair<int, float>>(data)));
  EXPECT_EQ(reference, hash(std::vector<std::pair<int, float>>(data)));

  // Retry with positive 0
  EXPECT_EQ(
      reference,
      hash(std::vector<std::pair<int, float>>({{3, 10.4f}, {50, 0.f}, {1000, 30.2f}})));
}

TEST(HashTest, BEH_DoubleRange) {
  const maidsafe::SeededHash<maidsafe::SipHash> hash{};
  const std::initializer_list<std::pair<int, double>> data =
      { {3, 10.4}, {50, -0.f}, {1000, 30.2} };
  const std::uint64_t reference = hash(data);

  EXPECT_EQ(reference, hash(std::forward_list<std::pair<int, double>>(data)));
  EXPECT_EQ(reference, hash(std::list<std::pair<int, double>>(data)));
  EXPECT_EQ(reference, hash(std::map<int, double>({{3, 10.4}, {50, -0.f}, {1000, 30.2}})));
  EXPECT_EQ(reference, hash(std::set<std::pair<int, double>>(data)));
  EXPECT_EQ(reference, hash(std::vector<std::pair<int, double>>(data)));

  // Retry with positive 0
  EXPECT_EQ(
      reference,
      hash(std::vector<std::pair<int, double>>({ {3, 10.4}, {50, 0.f}, {1000, 30.2} })));
}

TEST(HashTest, BEH_StringRange) {
  const maidsafe::SeededHash<maidsafe::SipHash> hash{};
  const std::initializer_list<std::string> data = { "string1", "string2", "string3" };
  const std::uint64_t reference = hash(data);

  EXPECT_EQ(
      reference,
      hash(
          std::map<std::string, int>({{"string1", 10}, {"string2", 20}, {"string3", 30}}) |
          boost::adaptors::map_keys));
  EXPECT_EQ(reference, hash(std::forward_list<std::string>(data)));
  EXPECT_EQ(reference, hash(std::list<std::string>(data)));
  EXPECT_EQ(reference, hash(std::set<std::string>(data)));
  EXPECT_EQ(reference, hash(std::vector<std::string>(data)));

  std::vector<boost::string_ref> ref_data = { "string1-", "string2", "string3" };
  EXPECT_NE(reference, hash(ref_data));
  ref_data[0].remove_suffix(1);
  EXPECT_EQ(reference, hash(ref_data));
}

TEST(HashTest, BEH_PreferHashAppend) {
  const maidsafe::SeededHash<maidsafe::SipHash> hash{};
  EXPECT_EQ(
      hash(UseInternalSerializeWithVersion{10, 20, 30}),
      hash(IgnoreInternalSerialize{10, 20, 30}));
  EXPECT_EQ(
      hash(UseInternalSerializeWithVersion{10, 20, 30}),
      hash(IgnoreExternalSerialize{10, 20, 30}));
}

TEST(HashTest, BEH_CerealAndHashing) {
  TestCustomType<IgnoreInternalSerialize>();
  TestCustomType<UseInternalSerializeWithVersion>();
  TestCustomType<UseInternalSerializeWithoutVersion>();
  TestCustomType<IgnoreExternalSerialize>();
  TestCustomType<UseExternalSerializeWithVersion>();
  TestCustomType<UseExternalSerializeWithoutVersion>();
}

TEST(HashTest, BEH_InHashMap) {
  std::unordered_map<
    std::pair<std::string, std::string>,
    int,
    maidsafe::SeededHash<maidsafe::SipHash>> hash_table;

  hash_table[{"entry", "one"}] = -1;
  hash_table[{"entry", "two"}] = 2;
  hash_table[{"entri", "one"}] = 3;

  const auto value1 = hash_table[{"entry", "one"}];
  const auto value2 = hash_table[{"entry", "two"}];
  const auto value3 = hash_table[{"entri", "one"}];

  EXPECT_EQ(-1, value1);
  EXPECT_EQ(2, value2);
  EXPECT_EQ(3, value3);
}

}  // namespace test
}  // namespace maidsafe
