/* Copyright 2009 MaidSafe.net limited

This MaidSafe Software is licensed under the MaidSafe.net Commercial License, version 1.0 or later,
and The General Public License (GPL), version 3. By contributing code to this project You agree to
the terms laid out in the MaidSafe Contributor Agreement, version 1.0, found in the root directory
of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also available at:

http://www.novinet.com/license

Unless required by applicable law or agreed to in writing, software distributed under the License is
distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. See the License for the specific language governing permissions and limitations under the
License.
*/

#include <thread>
#include <algorithm>
#include <cstdlib>
#include <set>
#include <vector>
#include <chrono>

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"

#include "maidsafe/common/config.h"
#include "maidsafe/common/crypto.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"

namespace fs = boost::filesystem;
namespace bptime = boost::posix_time;

namespace maidsafe {

namespace test {

TEST(UtilsTest, BEH_VersionToString) {
  EXPECT_TRUE(VersionToString(-2).empty());
  EXPECT_TRUE(VersionToString(kInvalidVersion).empty());
  EXPECT_EQ("0.0.000", VersionToString(0));
  EXPECT_EQ("0.0.001", VersionToString(1));
  EXPECT_EQ("0.0.010", VersionToString(10));
  EXPECT_EQ("0.0.100", VersionToString(100));
  EXPECT_EQ("0.1.000", VersionToString(1000));
  EXPECT_EQ("0.1.001", VersionToString(1001));
  EXPECT_EQ("0.1.010", VersionToString(1010));
  EXPECT_EQ("0.1.100", VersionToString(1100));
  EXPECT_EQ("0.9.000", VersionToString(9000));
  EXPECT_EQ("0.9.001", VersionToString(9001));
  EXPECT_EQ("0.9.010", VersionToString(9010));
  EXPECT_EQ("0.9.100", VersionToString(9100));
  EXPECT_EQ("1.0.000", VersionToString(10000));
  EXPECT_EQ("1.0.001", VersionToString(10001));
  EXPECT_EQ("1.0.010", VersionToString(10010));
  EXPECT_EQ("1.0.100", VersionToString(10100));
  EXPECT_EQ("1.1.000", VersionToString(11000));
  EXPECT_EQ("1.1.001", VersionToString(11001));
  EXPECT_EQ("1.1.010", VersionToString(11010));
  EXPECT_EQ("1.1.100", VersionToString(11100));
  EXPECT_EQ("1.9.000", VersionToString(19000));
  EXPECT_EQ("1.9.001", VersionToString(19001));
  EXPECT_EQ("1.9.010", VersionToString(19010));
  EXPECT_EQ("1.9.100", VersionToString(19100));
  EXPECT_EQ("10.0.000", VersionToString(100000));
  EXPECT_EQ("10.0.001", VersionToString(100001));
  EXPECT_EQ("10.0.010", VersionToString(100010));
  EXPECT_EQ("10.0.100", VersionToString(100100));
  EXPECT_EQ("10.1.000", VersionToString(101000));
  EXPECT_EQ("10.1.001", VersionToString(101001));
  EXPECT_EQ("10.1.010", VersionToString(101010));
  EXPECT_EQ("10.1.100", VersionToString(101100));
  EXPECT_EQ("10.9.000", VersionToString(109000));
  EXPECT_EQ("10.9.001", VersionToString(109001));
  EXPECT_EQ("10.9.010", VersionToString(109010));
  EXPECT_EQ("10.9.100", VersionToString(109100));
  std::string major_version, minor_version, patch_version;
  EXPECT_EQ("1.1.001", VersionToString(11001, &major_version, &minor_version, &patch_version));
  EXPECT_EQ("1", major_version);
  EXPECT_EQ("1", minor_version);
  EXPECT_EQ("001", patch_version);
  EXPECT_EQ("12.3.456", VersionToString(123456, &major_version, &minor_version, &patch_version));
  EXPECT_EQ("12", major_version);
  EXPECT_EQ("3", minor_version);
  EXPECT_EQ("456", patch_version);
}

TEST(UtilsTest, BEH_VersionToInt) {
  EXPECT_EQ(kInvalidVersion, VersionToInt(""));
  EXPECT_EQ(kInvalidVersion, VersionToInt("Rubbish"));
  EXPECT_EQ(kInvalidVersion, VersionToInt("0.0.0.000"));
  EXPECT_EQ(kInvalidVersion, VersionToInt("0.000"));
  EXPECT_EQ(kInvalidVersion, VersionToInt("a.0.000"));
  EXPECT_EQ(kInvalidVersion, VersionToInt("0.a.000"));
  EXPECT_EQ(kInvalidVersion, VersionToInt("0.0.aaa"));
  EXPECT_EQ(kInvalidVersion, VersionToInt("0.00.000"));
  EXPECT_EQ(kInvalidVersion, VersionToInt("0.0.00"));
  EXPECT_EQ(kInvalidVersion, VersionToInt("-1.0.000"));
  EXPECT_EQ(kInvalidVersion, VersionToInt("0.-1.000"));
  EXPECT_EQ(kInvalidVersion, VersionToInt("0.0.-1"));
  EXPECT_EQ(0, VersionToInt("0.0.000"));
  EXPECT_EQ(1, VersionToInt("0.0.001"));
  EXPECT_EQ(10, VersionToInt("0.0.010"));
  EXPECT_EQ(100, VersionToInt("0.0.100"));
  EXPECT_EQ(1000, VersionToInt("0.1.000"));
  EXPECT_EQ(1001, VersionToInt("0.1.001"));
  EXPECT_EQ(1010, VersionToInt("0.1.010"));
  EXPECT_EQ(1100, VersionToInt("0.1.100"));
  EXPECT_EQ(9000, VersionToInt("0.9.000"));
  EXPECT_EQ(9001, VersionToInt("0.9.001"));
  EXPECT_EQ(9010, VersionToInt("0.9.010"));
  EXPECT_EQ(9100, VersionToInt("0.9.100"));
  EXPECT_EQ(10000, VersionToInt("1.0.000"));
  EXPECT_EQ(10001, VersionToInt("1.0.001"));
  EXPECT_EQ(10010, VersionToInt("1.0.010"));
  EXPECT_EQ(10100, VersionToInt("1.0.100"));
  EXPECT_EQ(11000, VersionToInt("1.1.000"));
  EXPECT_EQ(11001, VersionToInt("1.1.001"));
  EXPECT_EQ(11010, VersionToInt("1.1.010"));
  EXPECT_EQ(11100, VersionToInt("1.1.100"));
  EXPECT_EQ(19000, VersionToInt("1.9.000"));
  EXPECT_EQ(19001, VersionToInt("1.9.001"));
  EXPECT_EQ(19010, VersionToInt("1.9.010"));
  EXPECT_EQ(19100, VersionToInt("1.9.100"));
  EXPECT_EQ(100000, VersionToInt("10.0.000"));
  EXPECT_EQ(100001, VersionToInt("10.0.001"));
  EXPECT_EQ(100010, VersionToInt("10.0.010"));
  EXPECT_EQ(100100, VersionToInt("10.0.100"));
  EXPECT_EQ(101000, VersionToInt("10.1.000"));
  EXPECT_EQ(101001, VersionToInt("10.1.001"));
  EXPECT_EQ(101010, VersionToInt("10.1.010"));
  EXPECT_EQ(101100, VersionToInt("10.1.100"));
  EXPECT_EQ(109000, VersionToInt("10.9.000"));
  EXPECT_EQ(109001, VersionToInt("10.9.001"));
  EXPECT_EQ(109010, VersionToInt("10.9.010"));
  EXPECT_EQ(109100, VersionToInt("10.9.100"));
}

TEST(UtilsTest, BEH_Cpu_Size) {
  ASSERT_TRUE(CpuSize() == 32 || CpuSize() == 64);
}

TEST(UtilsTest, BEH_Names) {
  ASSERT_EQ(kCompanyName, "maidsafe");
  ASSERT_EQ(kApplicationName, "lifestuff");
}

TEST(UtilsTest, BEH_ByteRatios) {
  EXPECT_EQ(Bytes(1000), KiloBytes(1));
  EXPECT_EQ(KiloBytes(1000), MegaBytes(1));
  EXPECT_EQ(MegaBytes(1000), GigaBytes(1));
  EXPECT_EQ(Bytes(1) * 1000, KiloBytes(1));
  EXPECT_EQ(Bytes(2000), KiloBytes(4) / 2);
  EXPECT_NE(Bytes(2), KiloBytes(4) / 2000);  // this is a narrowing call rhs == 0
  EXPECT_EQ(Bytes(1) + Bytes(1), Bytes(2));
  EXPECT_EQ(Bytes(2) - Bytes(1), Bytes(1));
  EXPECT_EQ(Bytes(1).count(), 1);
  EXPECT_EQ(KiloBytes(1).count(), 1);
}


TEST(UtilsTest, BEH_BytesToDecimalSiUnits) {
  EXPECT_EQ("0 B", BytesToDecimalSiUnits(0U));
  EXPECT_EQ("1 B", BytesToDecimalSiUnits(1U));
  EXPECT_EQ("12 B", BytesToDecimalSiUnits(12U));
  EXPECT_EQ("123 B", BytesToDecimalSiUnits(123U));
  EXPECT_EQ("999 B", BytesToDecimalSiUnits(999U));

  EXPECT_EQ("1 kB", BytesToDecimalSiUnits(1000U));
  EXPECT_EQ("1 kB", BytesToDecimalSiUnits(1499U));
  EXPECT_EQ("2 kB", BytesToDecimalSiUnits(1500U));
  EXPECT_EQ("2 kB", BytesToDecimalSiUnits(1999U));
  EXPECT_EQ("12 kB", BytesToDecimalSiUnits(12499U));
  EXPECT_EQ("13 kB", BytesToDecimalSiUnits(12500U));
  EXPECT_EQ("123 kB", BytesToDecimalSiUnits(123499U));
  EXPECT_EQ("124 kB", BytesToDecimalSiUnits(123500U));
  EXPECT_EQ("999 kB", BytesToDecimalSiUnits(999499U));

  EXPECT_EQ("1 MB", BytesToDecimalSiUnits(999500U));
  EXPECT_EQ("1 MB", BytesToDecimalSiUnits(1000000U));
  EXPECT_EQ("1 MB", BytesToDecimalSiUnits(1499999U));
  EXPECT_EQ("2 MB", BytesToDecimalSiUnits(1500000U));
  EXPECT_EQ("12 MB", BytesToDecimalSiUnits(12499999U));
  EXPECT_EQ("13 MB", BytesToDecimalSiUnits(12500000U));
  EXPECT_EQ("123 MB", BytesToDecimalSiUnits(123499999U));
  EXPECT_EQ("124 MB", BytesToDecimalSiUnits(123500000U));
  EXPECT_EQ("999 MB", BytesToDecimalSiUnits(999499999U));

  EXPECT_EQ("1 GB", BytesToDecimalSiUnits(999500000U));
  EXPECT_EQ("1 GB", BytesToDecimalSiUnits(1000000000U));
  EXPECT_EQ("1 GB", BytesToDecimalSiUnits(1499999999U));
  EXPECT_EQ("2 GB", BytesToDecimalSiUnits(1500000000U));
  EXPECT_EQ("12 GB", BytesToDecimalSiUnits(12499999999U));
  EXPECT_EQ("13 GB", BytesToDecimalSiUnits(12500000000U));
  EXPECT_EQ("123 GB", BytesToDecimalSiUnits(123499999999U));
  EXPECT_EQ("124 GB", BytesToDecimalSiUnits(123500000000U));
  EXPECT_EQ("999 GB", BytesToDecimalSiUnits(999499999999U));

  EXPECT_EQ("1 TB", BytesToDecimalSiUnits(999500000000U));
  EXPECT_EQ("1 TB", BytesToDecimalSiUnits(1000000000000U));
  EXPECT_EQ("1 TB", BytesToDecimalSiUnits(1499999999999U));
  EXPECT_EQ("2 TB", BytesToDecimalSiUnits(1500000000000U));
  EXPECT_EQ("12 TB", BytesToDecimalSiUnits(12499999999999U));
  EXPECT_EQ("13 TB", BytesToDecimalSiUnits(12500000000000U));
  EXPECT_EQ("123 TB", BytesToDecimalSiUnits(123499999999999U));
  EXPECT_EQ("124 TB", BytesToDecimalSiUnits(123500000000000U));
  EXPECT_EQ("999 TB", BytesToDecimalSiUnits(999499999999999U));

  EXPECT_EQ("1 PB", BytesToDecimalSiUnits(999500000000000U));
  EXPECT_EQ("1 PB", BytesToDecimalSiUnits(1000000000000000U));
  EXPECT_EQ("1 PB", BytesToDecimalSiUnits(1499999999999999U));
  EXPECT_EQ("2 PB", BytesToDecimalSiUnits(1500000000000000U));
  EXPECT_EQ("12 PB", BytesToDecimalSiUnits(12499999999999999U));
  EXPECT_EQ("13 PB", BytesToDecimalSiUnits(12500000000000000U));
  EXPECT_EQ("123 PB", BytesToDecimalSiUnits(123499999999999999U));
  EXPECT_EQ("124 PB", BytesToDecimalSiUnits(123500000000000000U));
  EXPECT_EQ("999 PB", BytesToDecimalSiUnits(999499999999999999U));

  EXPECT_EQ("1 EB", BytesToDecimalSiUnits(999500000000000000U));
  EXPECT_EQ("1 EB", BytesToDecimalSiUnits(1000000000000000000U));
  EXPECT_EQ("1 EB", BytesToDecimalSiUnits(1499999999999999999U));
  EXPECT_EQ("2 EB", BytesToDecimalSiUnits(1500000000000000000U));
  EXPECT_EQ("9 EB", BytesToDecimalSiUnits(9499999999999999999U));
  EXPECT_EQ("10 EB", BytesToDecimalSiUnits(9500000000000000000U));
  EXPECT_EQ("12 EB", BytesToDecimalSiUnits(12499999999999999999U));
  EXPECT_EQ("13 EB", BytesToDecimalSiUnits(12500000000000000000U));
  EXPECT_EQ("18 EB", BytesToDecimalSiUnits(18446744073709551615U));
}

TEST(UtilsTest, BEH_BytesToBinarySiUnits) {
  EXPECT_EQ("0 B", BytesToBinarySiUnits(0U));
  EXPECT_EQ("1 B", BytesToBinarySiUnits(1U));
  EXPECT_EQ("12 B", BytesToBinarySiUnits(12U));
  EXPECT_EQ("123 B", BytesToBinarySiUnits(123U));
  EXPECT_EQ("1023 B", BytesToBinarySiUnits(1023U));

  EXPECT_EQ("1 KiB", BytesToBinarySiUnits(1024U));
  EXPECT_EQ("1 KiB", BytesToBinarySiUnits(1535U));
  EXPECT_EQ("2 KiB", BytesToBinarySiUnits(1536U));
  EXPECT_EQ("12 KiB", BytesToBinarySiUnits(12799U));
  EXPECT_EQ("13 KiB", BytesToBinarySiUnits(12800U));
  EXPECT_EQ("123 KiB", BytesToBinarySiUnits(126463U));
  EXPECT_EQ("124 KiB", BytesToBinarySiUnits(126464U));
  EXPECT_EQ("1023 KiB", BytesToBinarySiUnits(1048063U));

  EXPECT_EQ("1 MiB", BytesToBinarySiUnits(1048064U));
  EXPECT_EQ("1 MiB", BytesToBinarySiUnits(1572863U));
  EXPECT_EQ("2 MiB", BytesToBinarySiUnits(1572864U));
  EXPECT_EQ("12 MiB", BytesToBinarySiUnits(13107199U));
  EXPECT_EQ("13 MiB", BytesToBinarySiUnits(13107200U));
  EXPECT_EQ("123 MiB", BytesToBinarySiUnits(129499135U));
  EXPECT_EQ("124 MiB", BytesToBinarySiUnits(129499136U));
  EXPECT_EQ("1023 MiB", BytesToBinarySiUnits(1073217535U));

  EXPECT_EQ("1 GiB", BytesToBinarySiUnits(1073217536U));
  EXPECT_EQ("1 GiB", BytesToBinarySiUnits(1610612735U));
  EXPECT_EQ("2 GiB", BytesToBinarySiUnits(1610612736U));
  EXPECT_EQ("12 GiB", BytesToBinarySiUnits(13421772799U));
  EXPECT_EQ("13 GiB", BytesToBinarySiUnits(13421772800U));
  EXPECT_EQ("123 GiB", BytesToBinarySiUnits(132607115263U));
  EXPECT_EQ("124 GiB", BytesToBinarySiUnits(132607115264U));
  EXPECT_EQ("1023 GiB", BytesToBinarySiUnits(1098974756863U));

  EXPECT_EQ("1 TiB", BytesToBinarySiUnits(1098974756864U));
  EXPECT_EQ("1 TiB", BytesToBinarySiUnits(1649267441663U));
  EXPECT_EQ("2 TiB", BytesToBinarySiUnits(1649267441664U));
  EXPECT_EQ("12 TiB", BytesToBinarySiUnits(13743895347199U));
  EXPECT_EQ("13 TiB", BytesToBinarySiUnits(13743895347200U));
  EXPECT_EQ("123 TiB", BytesToBinarySiUnits(135789686030335U));
  EXPECT_EQ("124 TiB", BytesToBinarySiUnits(135789686030336U));
  EXPECT_EQ("1023 TiB", BytesToBinarySiUnits(1125350151028735U));

  EXPECT_EQ("1 PiB", BytesToBinarySiUnits(1125350151028736U));
  EXPECT_EQ("1 PiB", BytesToBinarySiUnits(1688849860263935U));
  EXPECT_EQ("2 PiB", BytesToBinarySiUnits(1688849860263936U));
  EXPECT_EQ("12 PiB", BytesToBinarySiUnits(14073748835532799U));
  EXPECT_EQ("13 PiB", BytesToBinarySiUnits(14073748835532800U));
  EXPECT_EQ("123 PiB", BytesToBinarySiUnits(139048638495064063U));
  EXPECT_EQ("124 PiB", BytesToBinarySiUnits(139048638495064064U));
  EXPECT_EQ("1023 PiB", BytesToBinarySiUnits(1152358554653425663U));

  EXPECT_EQ("1 EiB", BytesToBinarySiUnits(1152358554653425664U));
  EXPECT_EQ("1 EiB", BytesToBinarySiUnits(1729382256910270463U));
  EXPECT_EQ("2 EiB", BytesToBinarySiUnits(1729382256910270464U));
  EXPECT_EQ("9 EiB", BytesToBinarySiUnits(10952754293765046271U));
  EXPECT_EQ("10 EiB", BytesToBinarySiUnits(10952754293765046272U));
  EXPECT_EQ("15 EiB", BytesToBinarySiUnits(17870283321406128127U));
  EXPECT_EQ("16 EiB", BytesToBinarySiUnits(17870283321406128128U));
  EXPECT_EQ("16 EiB", BytesToBinarySiUnits(18446744073709551615U));
}

TEST(UtilsTest, BEH_RandomStringMultiThread) {
  std::vector<std::thread> threads;
  for (int i(0); i != 20; ++i)
    threads.push_back(std::move(std::thread([] {
      for (int j(0); j != 1000; ++j)
        RandomString(4096);
    })));
  for (std::thread& thread : threads)
    thread.join();
}

TEST(UtilsTest, BEH_RandomStringGenerator) {
  std::set<std::string>random_strings;
  const size_t kCount(100);
  const size_t kMaxDuplicates(1);
  for (size_t j = 10; j< 100; ++j) {
    for (size_t i = 0; i < kCount; ++i) {
      random_strings.insert(RandomString(j));
    }
    EXPECT_GE(kMaxDuplicates, kCount - random_strings.size());
    random_strings.clear();
  }
}

TEST(UtilsTest, BEH_RandomStringSingleThread) {
  const size_t kStringSize = 4096;
  std::string test1 = RandomAlphaNumericString(kStringSize);
  std::string test2 = RandomAlphaNumericString(kStringSize);
  EXPECT_EQ(kStringSize, test1.size());
  EXPECT_EQ(kStringSize, test2.size());
  EXPECT_NE(test1, test2);
  const std::string kAlphaNumeric("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefgh"
                                  "ijklmnopqrstuvwxyz");
  for (size_t i = 0; i < kStringSize; ++i) {
    EXPECT_NE(std::string::npos, kAlphaNumeric.find(test1.at(i)));
    EXPECT_NE(std::string::npos, kAlphaNumeric.find(test2.at(i)));
  }
}

TEST(UtilsTest, BEH_HexEncodeDecode) {
  maidsafe::test::RunInParallel(100, [&] {
    for (int i = 0; i < 10; ++i) {
      std::string original = RandomString(100);
      std::string encoded = EncodeToHex(original);
      EXPECT_EQ(200U, encoded.size());
      std::string decoded = DecodeFromHex(encoded);
      EXPECT_EQ(original, decoded);
    }
  });
  const std::string kKnownEncoded("0123456789abcdef");
  const std::string kKnownDecoded("\x1\x23\x45\x67\x89\xab\xcd\xef");
  EXPECT_EQ(kKnownEncoded, EncodeToHex(kKnownDecoded));
  EXPECT_EQ(kKnownDecoded, DecodeFromHex(kKnownEncoded));
  EXPECT_TRUE(EncodeToHex("").empty());
  EXPECT_TRUE(DecodeFromHex("").empty());
  EXPECT_TRUE(DecodeFromHex("{").empty());
}

TEST(UtilsTest, BEH_Base64EncodeDecode) {
  maidsafe::test::RunInParallel(100, [&] {
    for (int i = 0; i < 10; ++i) {
      std::string original = RandomString(100);
      std::string encoded = EncodeToBase64(original);
      EXPECT_EQ(136U, encoded.size()) << "Encoding failed.";
      std::string decoded = DecodeFromBase64(encoded);
      EXPECT_EQ(original, decoded) << "encoded -> decoded failed.";
    }
  });
  const std::string kKnownEncoded("BCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqr"
                                  "stuvwxyz0123456789+/A");
  const std::string kKnownDecoded("\x04\x20\xc4\x14\x61\xc8\x24\xa2\xcc\x34\xe3"
      "\xd0\x45\x24\xd4\x55\x65\xd8\x65\xa6\xdc\x75\xe7\xe0\x86\x28\xe4\x96\x69"
      "\xe8\xa6\xaa\xec\xb6\xeb\xf0\xc7\x2c\xf4\xd7\x6d\xf8\xe7\xae\xfc\xf7\xef"
      "\xc0");
  EXPECT_EQ(kKnownEncoded, EncodeToBase64(kKnownDecoded));
  EXPECT_EQ(kKnownDecoded, DecodeFromBase64(kKnownEncoded));
  EXPECT_TRUE(EncodeToBase64("").empty());
  EXPECT_TRUE(DecodeFromBase64("").empty());
  EXPECT_TRUE(DecodeFromBase64("{").empty());
}

TEST(UtilsTest, BEH_Base32EncodeDecode) {
  maidsafe::test::RunInParallel(100, [&] {
    for (int i = 0; i < 10; ++i) {
      std::string original = RandomString(100);
      std::string encoded = EncodeToBase32(original);
      EXPECT_EQ(160U, encoded.size()) << "Encoding failed.";
      std::string decoded = DecodeFromBase32(encoded);
      EXPECT_EQ(original, decoded) << "encoded -> decoded failed.";
    }
  });
  const std::string kKnownEncoded("bcdefghijkmnpqrstuvwxyz23456789a");
  const std::string kKnownDecoded("\x08\x86\x42\x98\xe8\x4a\x96\xc6\xb9\xf0\x8c"
                                  "\xa7\x4a\xda\xf8\xce\xb7\xce\xfb\xe0");
  EXPECT_EQ(kKnownEncoded, EncodeToBase32(kKnownDecoded));
  EXPECT_EQ(kKnownDecoded, DecodeFromBase32(kKnownEncoded));
  EXPECT_TRUE(EncodeToBase32("").empty());
  EXPECT_TRUE(DecodeFromBase32("").empty());
  EXPECT_TRUE(DecodeFromBase32("{").empty());
}

TEST(UtilsTest, BEH_HexSubstr) {
  EXPECT_TRUE(HexSubstr("").empty());
  EXPECT_EQ("41", HexSubstr("A"));
  EXPECT_EQ("58595a", HexSubstr("XYZ"));
  EXPECT_EQ("616263646566", HexSubstr("abcdef"));
  EXPECT_EQ("616263..656667", HexSubstr("abcdefg"));
  EXPECT_EQ(14U, HexSubstr(RandomString(8 + RandomUint32() % 20)).size());
}

TEST(UtilsTest, BEH_Base32Substr) {
  EXPECT_TRUE(Base32Substr("").empty());
  EXPECT_EQ("ie", Base32Substr("A"));
  EXPECT_EQ("mbnxw", Base32Substr("XYZ"));
  EXPECT_EQ("nftgg3dfn3vys", Base32Substr("abcdefgh"));
  EXPECT_EQ("nftgg3d..ys4mkpn", Base32Substr("abcdefghijk"));
  EXPECT_EQ(16U, Base32Substr(RandomString(16 + RandomUint32() % 20)).size());
}

TEST(UtilsTest, BEH_Base64Substr) {
  EXPECT_TRUE(Base64Substr("").empty());
  EXPECT_EQ("QQ==", Base64Substr("A"));
  EXPECT_EQ("WFla", Base64Substr("XYZ"));
  EXPECT_EQ("YWJjZGVmZ2g=", Base64Substr("abcdefgh"));
  EXPECT_EQ("YWJjZGV..mtsbW5v", Base64Substr("abcdefghijklmno"));
  EXPECT_EQ(16U, Base64Substr(RandomString(32 + RandomUint32() % 20)).size());
}

TEST(UtilsTest, BEH_TimeFunctions) {
  uint64_t s, ms, ns;
  bptime::time_duration since_epoch(GetDurationSinceEpoch());
  ms = since_epoch.total_milliseconds();
  ns = since_epoch.total_nanoseconds();
  s = since_epoch.total_seconds();
  EXPECT_EQ(s, ms / 1000) << "s vs. ms failed.";
  EXPECT_EQ(s, ns / 1000000000) << "s vs. ns failed.";
  EXPECT_EQ(ms, ns / 1000000) << "ms vs. ns failed.";
  EXPECT_GE(GetTimeStamp() + 1 , MillisecondTimeStamp() / 1000);
}

TEST(UtilsTest, BEH_RandomNumberGen) {
  maidsafe::test::RunInParallel(10, [&] {
    std::set<int32_t>random_ints;
    std::set<uint32_t>random_uints;
    const size_t kCount(10000);
    // look for less than 0.05% duplicates
    const size_t kMaxDuplicates(kCount / 2000);
    for (size_t i = 0; i < kCount; ++i) {
      random_ints.insert(RandomInt32());
      random_uints.insert(RandomUint32());
    }
    EXPECT_GE(kMaxDuplicates, kCount - random_ints.size());
    EXPECT_GE(kMaxDuplicates, kCount - random_uints.size());
  });
}

TEST(UtilsTest, BEH_ReadWriteFile) {
  TestPath test_path(CreateTestPath("MaidSafe_TestUtils"));
  fs::path file_path(*test_path / "file.dat");
  std::string file_content;
  ASSERT_FALSE(fs::exists(file_path));
  EXPECT_FALSE(ReadFile(file_path, NULL));
  EXPECT_FALSE(ReadFile(file_path, &file_content));
  EXPECT_TRUE(file_content.empty());
  EXPECT_THROW(ReadFile(file_path), std::exception);
  EXPECT_FALSE(WriteFile("", file_content));
  EXPECT_TRUE(WriteFile(file_path, file_content));
  EXPECT_TRUE(fs::exists(file_path));
  EXPECT_EQ(0, fs::file_size(file_path));
  EXPECT_FALSE(ReadFile(file_path, NULL));
  EXPECT_TRUE(ReadFile(file_path, &file_content));
  EXPECT_TRUE(file_content.empty());

  file_content = RandomString(3000 + RandomUint32() % 1000);
  EXPECT_TRUE(WriteFile(file_path, file_content));
  EXPECT_NO_THROW(ReadFile(file_path));
  EXPECT_EQ(crypto::Hash<crypto::SHA512>(file_content),
            crypto::HashFile<crypto::SHA512>(file_path));

  std::string file_content_in;
  EXPECT_TRUE(ReadFile(file_path, &file_content_in));
  EXPECT_EQ(file_content, file_content_in);

  EXPECT_TRUE(WriteFile(file_path, "moo"));
  EXPECT_TRUE(ReadFile(file_path, &file_content_in));
  EXPECT_EQ("moo", file_content_in);
}

TEST(UtilsTest, BEH_Sleep) {
  bptime::ptime first_time(bptime::microsec_clock::universal_time());
  bptime::ptime second_time(bptime::microsec_clock::universal_time());
  EXPECT_LT((second_time - first_time).total_milliseconds(), 100);
  Sleep(bptime::milliseconds(100));
  bptime::ptime third_time(bptime::microsec_clock::universal_time());
  EXPECT_GE((third_time - first_time).total_milliseconds(), 100);
}

TEST(UtilsTest, BEH_GetHomeDir) {
  EXPECT_FALSE(GetHomeDir().empty());
  LOG(kInfo) << "Your home directory is " << GetHomeDir();
}

TEST(UtilsTest, BEH_GetUserAppDir) {
  EXPECT_FALSE(GetUserAppDir().empty());
  LOG(kInfo) << "Your user app directory is " << GetUserAppDir();
}

TEST(UtilsTest, BEH_GetSystemAppSupportDir) {
  EXPECT_FALSE(GetSystemAppSupportDir().empty());
  LOG(kInfo) << "Your system app support directory is " << GetSystemAppSupportDir();
}

TEST(UtilsTest, BEH_GetAppInstallDir) {
  EXPECT_FALSE(GetAppInstallDir().empty());
  LOG(kInfo) << "Your app install directory is " << GetAppInstallDir();
}

TEST(UtilsTest, BEH_AppDir) {
  EXPECT_NE(GetSystemAppSupportDir(), GetUserAppDir());
  EXPECT_NE(GetSystemAppSupportDir(), GetHomeDir());
  EXPECT_NE(GetUserAppDir(), GetHomeDir());
  std::string home(GetHomeDir().string());
  std::string system(GetSystemAppSupportDir().string());
  std::string user_app(GetUserAppDir().string());
  EXPECT_TRUE(user_app.find(home) != std::string::npos);
  EXPECT_TRUE(system.find(home) == std::string::npos);
}

TEST(UtilsTest, BEH_Concurrency) {
  EXPECT_GE(Concurrency(), 2U);
}

namespace {

void CleanupTest(fs::path *&test_path) {
  if (!test_path->empty()) {
    boost::system::error_code error_code;
    if (fs::remove_all(*test_path, error_code) == 0)
      LOG(kWarning) << "Test directory " << *test_path << " already deleted.";
    if (error_code)
      LOG(kWarning) << "Failed to clean up test directory " << *test_path
                   << "  (error message: " << error_code.message() << ")";
  }
  delete test_path;
  test_path = NULL;
}

}  // unnamed namespace

TEST(UtilsTest, BEH_CreateTestPath) {
  fs::path test_path;
  boost::system::error_code error_code;
  {
    TestPath test_path_ptr(CreateTestPath());
    test_path = *test_path_ptr;
    EXPECT_FALSE(test_path.empty());
    EXPECT_TRUE(fs::exists(test_path, error_code));
    EXPECT_EQ(0, error_code.value()) << error_code.message();
  }
  EXPECT_FALSE(fs::exists(test_path, error_code));
  EXPECT_EQ(boost::system::errc::no_such_file_or_directory, error_code.value())
        << error_code.message();
  {
    TestPath test_path_ptr(CreateTestPath("Another"));
    test_path = *test_path_ptr;
    EXPECT_FALSE(test_path.empty());
    EXPECT_TRUE(fs::exists(test_path, error_code));
    EXPECT_EQ(0, error_code.value()) << error_code.message();
  }
  EXPECT_FALSE(fs::exists(test_path, error_code));
  EXPECT_EQ(boost::system::errc::no_such_file_or_directory, error_code.value())
        << error_code.message();
  // Ensure we're able to cope with error cases
  fs::path *empty_path(new fs::path);
  CleanupTest(empty_path);
  EXPECT_TRUE(NULL == empty_path);
  fs::path *non_existent(new fs::path(std::string(100, 'a')));
  CleanupTest(non_existent);
  EXPECT_TRUE(NULL == non_existent);
}

}  // namespace test

}  // namespace maidsafe
