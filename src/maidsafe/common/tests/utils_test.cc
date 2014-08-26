/*  Copyright 2009 MaidSafe.net limited

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

#include "maidsafe/common/utils.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cwchar>
#include <set>
#include <thread>
#include <vector>

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"

#include "maidsafe/common/application_support_directories.h"
#include "maidsafe/common/config.h"
#include "maidsafe/common/crypto.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/process.h"
#include "maidsafe/common/test.h"

namespace fs = boost::filesystem;
namespace bptime = boost::posix_time;

namespace maidsafe {

namespace test {

TEST(UtilsTest, BEH_VersionInfo) {
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

TEST(UtilsTest, BEH_Names) {
  EXPECT_EQ(kCompanyName(), "MaidSafe");
  EXPECT_EQ(kApplicationName(), "TestCommon");
}

TEST(UtilsTest, BEH_Ratios) {
  EXPECT_EQ(Bytes(1000), KiloBytes(1));
  EXPECT_EQ(KiloBytes(1000), MegaBytes(1));
  EXPECT_EQ(MegaBytes(1000), GigaBytes(1));
  EXPECT_EQ((Bytes(1) * 1000), KiloBytes(1));
  EXPECT_EQ(Bytes(2000), KiloBytes(4) / 2);
  EXPECT_NE(Bytes(2), KiloBytes(4) / 2000);  // this is a narrowing call rhs == 0
  EXPECT_EQ((Bytes(1) + Bytes(1)), Bytes(2));
  EXPECT_EQ((Bytes(2) - Bytes(1)), Bytes(1));
  EXPECT_EQ(Bytes(1).count(), 1ULL);
  EXPECT_EQ(KiloBytes(1).count(), 1ULL);
}

TEST(UtilsTest, BEH_BytesToDecimalSiUnits ) {
  EXPECT_EQ("0 B", BytesToDecimalSiUnits(0U));
  EXPECT_EQ("1 B", BytesToDecimalSiUnits(1U));
  EXPECT_EQ("12 B", BytesToDecimalSiUnits(12U));
  EXPECT_EQ("123 B", BytesToDecimalSiUnits(123U));
  EXPECT_EQ("999 B", BytesToDecimalSiUnits(999U));

  EXPECT_EQ("1.00 kB", BytesToDecimalSiUnits(1000U));
  EXPECT_EQ("1.50 kB", BytesToDecimalSiUnits(1499U));
  EXPECT_EQ("1.50 kB", BytesToDecimalSiUnits(1500U));
  EXPECT_EQ("2.00 kB", BytesToDecimalSiUnits(1999U));
  EXPECT_EQ("12.50 kB", BytesToDecimalSiUnits(12499U));
  EXPECT_EQ("12.50 kB", BytesToDecimalSiUnits(12500U));
  EXPECT_EQ("123.50 kB", BytesToDecimalSiUnits(123499U));
  EXPECT_EQ("123.50 kB", BytesToDecimalSiUnits(123500U));
  EXPECT_EQ("999.50 kB", BytesToDecimalSiUnits(999499U));

  EXPECT_EQ("1.00 MB", BytesToDecimalSiUnits(999500U));
  EXPECT_EQ("1.00 MB", BytesToDecimalSiUnits(1000000U));
  EXPECT_EQ("1.50 MB", BytesToDecimalSiUnits(1499999U));
  EXPECT_EQ("1.50 MB", BytesToDecimalSiUnits(1500000U));
  EXPECT_EQ("12.50 MB", BytesToDecimalSiUnits(12499999U));
  EXPECT_EQ("12.50 MB", BytesToDecimalSiUnits(12500000U));
  EXPECT_EQ("123.50 MB", BytesToDecimalSiUnits(123499999U));
  EXPECT_EQ("123.50 MB", BytesToDecimalSiUnits(123500000U));
  EXPECT_EQ("999.50 MB", BytesToDecimalSiUnits(999499999U));

  EXPECT_EQ("1.00 GB", BytesToDecimalSiUnits(999500000U));
  EXPECT_EQ("1.00 GB", BytesToDecimalSiUnits(1000000000U));
  EXPECT_EQ("1.50 GB", BytesToDecimalSiUnits(1499999999U));
  EXPECT_EQ("1.50 GB", BytesToDecimalSiUnits(1500000000U));
  EXPECT_EQ("12.50 GB", BytesToDecimalSiUnits(12499999999U));
  EXPECT_EQ("12.50 GB", BytesToDecimalSiUnits(12500000000U));
  EXPECT_EQ("123.50 GB", BytesToDecimalSiUnits(123499999999U));
  EXPECT_EQ("123.50 GB", BytesToDecimalSiUnits(123500000000U));
  EXPECT_EQ("999.50 GB", BytesToDecimalSiUnits(999499999999U));

  EXPECT_EQ("1.00 TB", BytesToDecimalSiUnits(999500000000U));
  EXPECT_EQ("1.00 TB", BytesToDecimalSiUnits(1000000000000U));
  EXPECT_EQ("1.50 TB", BytesToDecimalSiUnits(1499999999999U));
  EXPECT_EQ("1.50 TB", BytesToDecimalSiUnits(1500000000000U));
  EXPECT_EQ("12.50 TB", BytesToDecimalSiUnits(12499999999999U));
  EXPECT_EQ("12.50 TB", BytesToDecimalSiUnits(12500000000000U));
  EXPECT_EQ("123.50 TB", BytesToDecimalSiUnits(123499999999999U));
  EXPECT_EQ("123.50 TB", BytesToDecimalSiUnits(123500000000000U));
  EXPECT_EQ("999.50 TB", BytesToDecimalSiUnits(999499999999999U));

  EXPECT_EQ("1.00 PB", BytesToDecimalSiUnits(999500000000000U));
  EXPECT_EQ("1.00 PB", BytesToDecimalSiUnits(1000000000000000U));
  EXPECT_EQ("1.50 PB", BytesToDecimalSiUnits(1499999999999999U));
  EXPECT_EQ("1.50 PB", BytesToDecimalSiUnits(1500000000000000U));
  EXPECT_EQ("12.50 PB", BytesToDecimalSiUnits(12499999999999999U));
  EXPECT_EQ("12.50 PB", BytesToDecimalSiUnits(12500000000000000U));
  EXPECT_EQ("123.50 PB", BytesToDecimalSiUnits(123499999999999999U));
  EXPECT_EQ("123.50 PB", BytesToDecimalSiUnits(123500000000000000U));
  EXPECT_EQ("999.50 PB", BytesToDecimalSiUnits(999499999999999999U));

  EXPECT_EQ("1.00 EB", BytesToDecimalSiUnits(999500000000000000U));
  EXPECT_EQ("1.00 EB", BytesToDecimalSiUnits(1000000000000000000U));
  EXPECT_EQ("1.50 EB", BytesToDecimalSiUnits(1499999999999999999U));
  EXPECT_EQ("1.50 EB", BytesToDecimalSiUnits(1500000000000000000U));
  EXPECT_EQ("9.50 EB", BytesToDecimalSiUnits(9499999999999999999U));
  EXPECT_EQ("9.50 EB", BytesToDecimalSiUnits(9500000000000000000U));
  EXPECT_EQ("12.50 EB", BytesToDecimalSiUnits(12499999999999999999U));
  EXPECT_EQ("12.50 EB", BytesToDecimalSiUnits(12500000000000000000U));
  EXPECT_EQ("18.45 EB", BytesToDecimalSiUnits(18446744073709551615U));
}

TEST(UtilsTest, BEH_BytesToBinarySiUnits) {
  EXPECT_EQ("0 B", BytesToBinarySiUnits(0U));
  EXPECT_EQ("1 B", BytesToBinarySiUnits(1U));
  EXPECT_EQ("12 B", BytesToBinarySiUnits(12U));
  EXPECT_EQ("123 B", BytesToBinarySiUnits(123U));
  EXPECT_EQ("1023 B", BytesToBinarySiUnits(1023U));

  EXPECT_EQ("1.00 KiB", BytesToBinarySiUnits(1024U));
  EXPECT_EQ("1.50 KiB", BytesToBinarySiUnits(1535U));
  EXPECT_EQ("1.50 KiB", BytesToBinarySiUnits(1536U));
  EXPECT_EQ("12.50 KiB", BytesToBinarySiUnits(12799U));
  EXPECT_EQ("12.50 KiB", BytesToBinarySiUnits(12800U));
  EXPECT_EQ("123.50 KiB", BytesToBinarySiUnits(126463U));
  EXPECT_EQ("123.50 KiB", BytesToBinarySiUnits(126464U));
  EXPECT_EQ("1023.50 KiB", BytesToBinarySiUnits(1048063U));

  EXPECT_EQ("1.00 MiB", BytesToBinarySiUnits(1048064U));
  EXPECT_EQ("1.50 MiB", BytesToBinarySiUnits(1572863U));
  EXPECT_EQ("1.50 MiB", BytesToBinarySiUnits(1572864U));
  EXPECT_EQ("12.50 MiB", BytesToBinarySiUnits(13107199U));
  EXPECT_EQ("12.50 MiB", BytesToBinarySiUnits(13107200U));
  EXPECT_EQ("123.50 MiB", BytesToBinarySiUnits(129499135U));
  EXPECT_EQ("123.50 MiB", BytesToBinarySiUnits(129499136U));
  EXPECT_EQ("1023.50 MiB", BytesToBinarySiUnits(1073217535U));

  EXPECT_EQ("1.00 GiB", BytesToBinarySiUnits(1073217536U));
  EXPECT_EQ("1.50 GiB", BytesToBinarySiUnits(1610612735U));
  EXPECT_EQ("1.50 GiB", BytesToBinarySiUnits(1610612736U));
  EXPECT_EQ("12.50 GiB", BytesToBinarySiUnits(13421772799U));
  EXPECT_EQ("12.50 GiB", BytesToBinarySiUnits(13421772800U));
  EXPECT_EQ("123.50 GiB", BytesToBinarySiUnits(132607115263U));
  EXPECT_EQ("123.50 GiB", BytesToBinarySiUnits(132607115264U));
  EXPECT_EQ("1023.50 GiB", BytesToBinarySiUnits(1098974756863U));

  EXPECT_EQ("1.00 TiB", BytesToBinarySiUnits(1098974756864U));
  EXPECT_EQ("1.50 TiB", BytesToBinarySiUnits(1649267441663U));
  EXPECT_EQ("1.50 TiB", BytesToBinarySiUnits(1649267441664U));
  EXPECT_EQ("12.50 TiB", BytesToBinarySiUnits(13743895347199U));
  EXPECT_EQ("12.50 TiB", BytesToBinarySiUnits(13743895347200U));
  EXPECT_EQ("123.50 TiB", BytesToBinarySiUnits(135789686030335U));
  EXPECT_EQ("123.50 TiB", BytesToBinarySiUnits(135789686030336U));
  EXPECT_EQ("1023.50 TiB", BytesToBinarySiUnits(1125350151028735U));

  EXPECT_EQ("1.00 PiB", BytesToBinarySiUnits(1125350151028736U));
  EXPECT_EQ("1.50 PiB", BytesToBinarySiUnits(1688849860263935U));
  EXPECT_EQ("1.50 PiB", BytesToBinarySiUnits(1688849860263936U));
  EXPECT_EQ("12.50 PiB", BytesToBinarySiUnits(14073748835532799U));
  EXPECT_EQ("12.50 PiB", BytesToBinarySiUnits(14073748835532800U));
  EXPECT_EQ("123.50 PiB", BytesToBinarySiUnits(139048638495064063U));
  EXPECT_EQ("123.50 PiB", BytesToBinarySiUnits(139048638495064064U));
  EXPECT_EQ("1023.50 PiB", BytesToBinarySiUnits(1152358554653425663U));

  EXPECT_EQ("1.00 EiB", BytesToBinarySiUnits(1152358554653425664U));
  EXPECT_EQ("1.50 EiB", BytesToBinarySiUnits(1729382256910270463U));
  EXPECT_EQ("1.50 EiB", BytesToBinarySiUnits(1729382256910270464U));
  EXPECT_EQ("9.50 EiB", BytesToBinarySiUnits(10952754293765046271U));
  EXPECT_EQ("9.50 EiB", BytesToBinarySiUnits(10952754293765046272U));
  EXPECT_EQ("15.50 EiB", BytesToBinarySiUnits(17870283321406128127U));
  EXPECT_EQ("15.50 EiB", BytesToBinarySiUnits(17870283321406128128U));
  EXPECT_EQ("16.00 EiB", BytesToBinarySiUnits(18446744073709551615U));
}

TEST(UtilsTest, FUNC_RandomStringMultiThread) {
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
  std::set<std::string> random_strings;
  const size_t kCount(100);
  const size_t kMaxDuplicates(1);
  for (size_t j = 10; j < 100; ++j) {
    for (size_t i = 0; i < kCount; ++i) {
      random_strings.insert(RandomString(j));
    }
    EXPECT_GE(kMaxDuplicates, (kCount - random_strings.size()));
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
  const std::string kAlphaNumeric(
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefgh"
      "ijklmnopqrstuvwxyz");
  for (size_t i = 0; i < kStringSize; ++i) {
    EXPECT_NE(std::string::npos, kAlphaNumeric.find(test1.at(i)));
    EXPECT_NE(std::string::npos, kAlphaNumeric.find(test2.at(i)));
  }
}

TEST(UtilsTest, BEH_HexEncodeDecode) {
  bool expected_sizes_ok{ true }, decoded_ok{ true };

  maidsafe::test::RunInParallel(100, [&] {
    for (int i = 0; i < 10; ++i) {
      std::string original = RandomString(100);
      std::string encoded = HexEncode(original);
      if (encoded.size() != 200U)
        expected_sizes_ok = false;
      std::string decoded = HexDecode(encoded);
      if (decoded != original)
        decoded_ok = false;
    }
  });

  EXPECT_TRUE(expected_sizes_ok);
  EXPECT_TRUE(decoded_ok);

  const std::string kKnownEncoded("0123456789abcdef");
  const std::string kKnownDecoded("\x1\x23\x45\x67\x89\xab\xcd\xef");
  EXPECT_EQ(kKnownEncoded, HexEncode(kKnownDecoded));
  EXPECT_EQ(kKnownDecoded, HexDecode(kKnownEncoded));
  EXPECT_TRUE(HexEncode("").empty());
  EXPECT_TRUE(HexDecode("").empty());
  EXPECT_THROW(HexDecode("{"), common_error);
}

TEST(UtilsTest, BEH_Base64EncodeDecode) {
  bool expected_sizes_ok{ true }, decoded_ok{ true };

  maidsafe::test::RunInParallel(100, [&] {
    for (int i = 0; i < 10; ++i) {
      std::string original = RandomString(100);
      std::string encoded = Base64Encode(original);
      if (encoded.size() != 136U)
        expected_sizes_ok = false;
      std::string decoded = Base64Decode(encoded);
      if (decoded != original)
        decoded_ok = false;
    }
  });

  EXPECT_TRUE(expected_sizes_ok);
  EXPECT_TRUE(decoded_ok);

  // from wikipedia
  std::string man;
  man += "Man is distinguished, not only by his reason, but by this singular ";
  man += "passion from other animals, which is a lust of the mind, that by a ";
  man += "perseverance of delight in the continued and indefatigable generation";
  man += " of knowledge, exceeds the short vehemence of any carnal pleasure.";
  std::string encoded_man;
  encoded_man += "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlz";
  encoded_man += "IHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2Yg";
  encoded_man += "dGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGlu";
  encoded_man += "dWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRo";
  encoded_man += "ZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=";
  EXPECT_EQ(Base64Encode(man), encoded_man);
  EXPECT_EQ(man, Base64Decode(Base64Encode(man)));
  EXPECT_EQ(Base64Encode("pleasure."), "cGxlYXN1cmUu");
  EXPECT_EQ("pleasure", Base64Decode(Base64Encode("pleasure")));
  EXPECT_EQ(Base64Encode("leasure."), "bGVhc3VyZS4=");
  EXPECT_EQ("leasure.", Base64Decode(Base64Encode("leasure.")));
  EXPECT_EQ(Base64Encode("easure."), "ZWFzdXJlLg==");
  EXPECT_EQ("easure.", Base64Decode(Base64Encode("easure.")));
  EXPECT_EQ(Base64Encode("asure."), "YXN1cmUu");
  EXPECT_EQ("asure.", Base64Decode(Base64Encode("asure.")));
  EXPECT_EQ(Base64Encode("sure."), "c3VyZS4=");
  EXPECT_EQ("sure.", Base64Decode(Base64Encode("sure.")));
  // test vectors from RFC4648
  EXPECT_EQ(Base64Encode("f"), "Zg==");
  EXPECT_EQ(Base64Encode("fo"), "Zm8=");
  EXPECT_EQ(Base64Encode("foo"), "Zm9v");
  EXPECT_EQ(Base64Encode("foob"), "Zm9vYg==");
  EXPECT_EQ(Base64Encode("fooba"), "Zm9vYmE=");
  EXPECT_EQ(Base64Encode("foobar"), "Zm9vYmFy");
  EXPECT_EQ("f", Base64Decode("Zg=="));
  EXPECT_EQ("fo", Base64Decode("Zm8="));
  EXPECT_EQ("foo", Base64Decode("Zm9v"));
  EXPECT_EQ("foob", Base64Decode("Zm9vYg=="));
  EXPECT_EQ("fooba", Base64Decode("Zm9vYmE="));
  EXPECT_EQ("foobar", Base64Decode("Zm9vYmFy"));
  EXPECT_THROW(Base64Decode("Zg="), common_error);
  EXPECT_THROW(Base64Decode("Zg"), common_error);
  EXPECT_THROW(Base64Decode("Z"), common_error);
}

TEST(UtilsTest, BEH_HexSubstr) {
  EXPECT_TRUE(HexSubstr("").empty());
  EXPECT_EQ("41", HexSubstr("A"));
  EXPECT_EQ("58595a", HexSubstr("XYZ"));
  EXPECT_EQ("616263646566", HexSubstr("abcdef"));
  EXPECT_EQ("616263..656667", HexSubstr("abcdefg"));
  EXPECT_EQ(14U, HexSubstr(RandomString(8 + RandomUint32() % 20)).size());
}

TEST(UtilsTest, BEH_Base64Substr) {
  EXPECT_TRUE(Base64Substr("").empty());
  EXPECT_EQ("QQ==", Base64Substr("A"));
  EXPECT_EQ("WFla", Base64Substr("XYZ"));
  EXPECT_EQ("YWJjZGVmZ2g=", Base64Substr("abcdefgh"));
  EXPECT_EQ("YWJjZGV..mtsbW5v", Base64Substr("abcdefghijklmno"));
  EXPECT_EQ(16U, Base64Substr(RandomString(32 + RandomUint32() % 20)).size());
}

std::string WstringToStringOldMethod(const std::wstring& input) {
  const std::locale kLocale("");
  std::string string_buffer(input.size(), 0);
  std::use_facet<std::ctype<wchar_t>>(kLocale).narrow(
      &input[0], &input[0] + input.size(), '?', &string_buffer[0]);

  return std::string(&string_buffer[0], input.size());
}

TEST(UtilsTest, BEH_WstringToString) {
#ifdef MAIDSAFE_WIN32
  std::wstring input(L"Test wstring");
  std::string converted(WstringToString(input));
  EXPECT_EQ(converted, "Test wstring");

  for (int i(0); i != 100; ++i) {
    input.assign(5, static_cast<wchar_t>(RandomUint32() % std::numeric_limits<wchar_t>::max()));
    auto old_method(WstringToStringOldMethod(input));
    try {
      auto new_method(WstringToString(input));
      ASSERT_EQ(new_method, old_method);
    }
    catch(const common_error&) {}
  }
#endif
}

std::wstring StringToWstringOldMethod(const std::string& input) {
  std::wstring buffer(input.size(), L'\0');
  size_t num_chars = mbstowcs(&buffer[0], input.c_str(), input.size());
  buffer.resize(num_chars);
  return buffer;
}

TEST(UtilsTest, BEH_StringToWstring) {
#ifdef MAIDSAFE_WIN32
  std::string input("Test string");
  std::wstring converted(StringToWstring(input));
  EXPECT_EQ(converted, L"Test string");

  for (int i(0); i != 100; ++i) {
    input.assign(5, static_cast<char>(RandomUint32() % std::numeric_limits<char>::max()));
    auto old_method(StringToWstringOldMethod(input));
    try {
      auto new_method(StringToWstring(input));
      if (input[0] != '\0')
        ASSERT_EQ(new_method, old_method) << static_cast<int>(input[0]);
    }
    catch(const common_error&) {}
  }
#endif
}

TEST(UtilsTest, BEH_TimeFunctions) {
  uint64_t ms_since_epoch(GetTimeStamp());
  auto now(bptime::microsec_clock::universal_time());
  auto from_timestamp(TimeStampToPtime(ms_since_epoch));
  EXPECT_LE((now - from_timestamp), bptime::milliseconds(2));
}

TEST(UtilsTest, FUNC_RandomNumberGen) {
  bool within_threshold{ true };
  maidsafe::test::RunInParallel(10, [&] {
    std::set<int32_t> random_ints;
    std::set<uint32_t> random_uints;
    const size_t kCount(10000);
    // look for less than 0.05% duplicates
    const size_t kMaxDuplicates(kCount / 2000);
    for (size_t i = 0; i < kCount; ++i) {
      random_ints.insert(RandomInt32());
      random_uints.insert(RandomUint32());
    }
    if (kMaxDuplicates < (kCount - random_ints.size()))
      within_threshold = false;
    if (kMaxDuplicates < (kCount - random_uints.size()))
      within_threshold = false;
  });
  EXPECT_TRUE(within_threshold);
}

TEST(UtilsTest, BEH_ReadFileandWriteFile) {
  TestPath test_path(CreateTestPath("MaidSafe_TestUtils"));
  fs::path file_path(*test_path / "file.dat");
  std::string file_content;
  ASSERT_FALSE(fs::exists(file_path));
  EXPECT_FALSE(ReadFile(file_path, nullptr));
  EXPECT_FALSE(ReadFile(file_path, &file_content));
  EXPECT_TRUE(file_content.empty());
  EXPECT_THROW(ReadFile(file_path), std::exception);
  EXPECT_FALSE(WriteFile("", file_content));
  EXPECT_TRUE(WriteFile(file_path, file_content));
  EXPECT_TRUE(fs::exists(file_path));
  EXPECT_EQ(0, fs::file_size(file_path));
  EXPECT_FALSE(ReadFile(file_path, nullptr));
  EXPECT_TRUE(ReadFile(file_path, &file_content));
  EXPECT_TRUE(file_content.empty());

  file_content = RandomString(3000 + RandomUint32() % 1000);
  EXPECT_TRUE(WriteFile(file_path, file_content));
  EXPECT_NO_THROW(ReadFile(file_path));
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
  Sleep(std::chrono::milliseconds(100));
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
  EXPECT_NE(user_app.find(home), std::string::npos);
  EXPECT_EQ(system.find(home), std::string::npos);
}

TEST(UtilsTest, BEH_Concurrency) {
  EXPECT_TRUE(Concurrency()>= 2U);
}

namespace {

void CleanupTest(fs::path*& test_path) {
  if (!test_path->empty()) {
    boost::system::error_code error_code;
    if (fs::remove_all(*test_path, error_code) == 0)
      LOG(kWarning) << "Test directory " << *test_path << " already deleted.";
    if (error_code)
      LOG(kWarning) << "Failed to clean up test directory " << *test_path
                    << "  (error message: " << error_code.message() << ")";
  }
  delete test_path;
  test_path = nullptr;
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
    EXPECT_EQ(0, error_code.value());
  }
  EXPECT_FALSE(fs::exists(test_path, error_code));
  EXPECT_EQ(boost::system::errc::no_such_file_or_directory, error_code.value());
  {
    TestPath test_path_ptr(CreateTestPath("Another"));
    test_path = *test_path_ptr;
    EXPECT_FALSE(test_path.empty());
    EXPECT_TRUE(fs::exists(test_path, error_code));
    EXPECT_EQ(0, error_code.value());
  }
  EXPECT_FALSE(fs::exists(test_path, error_code));
  EXPECT_EQ(boost::system::errc::no_such_file_or_directory, error_code.value());
  // Ensure we're able to cope with error cases
  auto empty_path(new fs::path);
  CleanupTest(empty_path);
  EXPECT_TRUE(!empty_path);
  fs::path* non_existent(new fs::path(std::string(100, 'a')));
  CleanupTest(non_existent);
  EXPECT_TRUE(!non_existent);
}

TEST(UtilsTest, BEH_GetProcessId) {
  EXPECT_GT(process::GetProcessId(), 0);
}

}  // namespace test

}  // namespace maidsafe
