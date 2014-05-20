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

TEST_CASE("VersionToInt", "[Utils][Unit]") {
  CHECK(kInvalidVersion == VersionToInt(""));
  CHECK(kInvalidVersion == VersionToInt("Rubbish"));
  CHECK(kInvalidVersion == VersionToInt("0.0.0.000"));
  CHECK(kInvalidVersion == VersionToInt("0.000"));
  CHECK(kInvalidVersion == VersionToInt("a.0.000"));
  CHECK(kInvalidVersion == VersionToInt("0.a.000"));
  CHECK(kInvalidVersion == VersionToInt("0.0.aaa"));
  CHECK(kInvalidVersion == VersionToInt("0.00.000"));
  CHECK(kInvalidVersion == VersionToInt("0.0.00"));
  CHECK(kInvalidVersion == VersionToInt("-1.0.000"));
  CHECK(kInvalidVersion == VersionToInt("0.-1.000"));
  CHECK(kInvalidVersion == VersionToInt("0.0.-1"));
  CHECK(0 == VersionToInt("0.0.000"));
  CHECK(1 == VersionToInt("0.0.001"));
  CHECK(10 == VersionToInt("0.0.010"));
  CHECK(100 == VersionToInt("0.0.100"));
  CHECK(1000 == VersionToInt("0.1.000"));
  CHECK(1001 == VersionToInt("0.1.001"));
  CHECK(1010 == VersionToInt("0.1.010"));
  CHECK(1100 == VersionToInt("0.1.100"));
  CHECK(9000 == VersionToInt("0.9.000"));
  CHECK(9001 == VersionToInt("0.9.001"));
  CHECK(9010 == VersionToInt("0.9.010"));
  CHECK(9100 == VersionToInt("0.9.100"));
  CHECK(10000 == VersionToInt("1.0.000"));
  CHECK(10001 == VersionToInt("1.0.001"));
  CHECK(10010 == VersionToInt("1.0.010"));
  CHECK(10100 == VersionToInt("1.0.100"));
  CHECK(11000 == VersionToInt("1.1.000"));
  CHECK(11001 == VersionToInt("1.1.001"));
  CHECK(11010 == VersionToInt("1.1.010"));
  CHECK(11100 == VersionToInt("1.1.100"));
  CHECK(19000 == VersionToInt("1.9.000"));
  CHECK(19001 == VersionToInt("1.9.001"));
  CHECK(19010 == VersionToInt("1.9.010"));
  CHECK(19100 == VersionToInt("1.9.100"));
  CHECK(100000 == VersionToInt("10.0.000"));
  CHECK(100001 == VersionToInt("10.0.001"));
  CHECK(100010 == VersionToInt("10.0.010"));
  CHECK(100100 == VersionToInt("10.0.100"));
  CHECK(101000 == VersionToInt("10.1.000"));
  CHECK(101001 == VersionToInt("10.1.001"));
  CHECK(101010 == VersionToInt("10.1.010"));
  CHECK(101100 == VersionToInt("10.1.100"));
  CHECK(109000 == VersionToInt("10.9.000"));
  CHECK(109001 == VersionToInt("10.9.001"));
  CHECK(109010 == VersionToInt("10.9.010"));
  CHECK(109100 == VersionToInt("10.9.100"));
}

TEST_CASE("Names", "[Utils][Unit]") {
  CHECK(kCompanyName() == "MaidSafe");
  CHECK(kApplicationName() == "TestCommon");
}

TEST_CASE("ByteRatios", "[Utils][Unit]") {
  CHECK(Bytes(1000) == KiloBytes(1));
  CHECK(KiloBytes(1000) == MegaBytes(1));
  CHECK(MegaBytes(1000) == GigaBytes(1));
  CHECK((Bytes(1) * 1000) == KiloBytes(1));
  CHECK(Bytes(2000) == KiloBytes(4) / 2);
  CHECK(Bytes(2) != KiloBytes(4) / 2000);  // this is a narrowing call rhs == 0
  CHECK((Bytes(1) + Bytes(1)) == Bytes(2));
  CHECK((Bytes(2) - Bytes(1)) == Bytes(1));
  CHECK(Bytes(1).count() == 1ULL);
  CHECK(KiloBytes(1).count() == 1ULL);
}

TEST_CASE("BytesToDecimalSiUnits", "[Utils][Unit]") {
  CHECK("0 B" == BytesToDecimalSiUnits(0U));
  CHECK("1 B" == BytesToDecimalSiUnits(1U));
  CHECK("12 B" == BytesToDecimalSiUnits(12U));
  CHECK("123 B" == BytesToDecimalSiUnits(123U));
  CHECK("999 B" == BytesToDecimalSiUnits(999U));

  CHECK("1 kB" == BytesToDecimalSiUnits(1000U));
  CHECK("1 kB" == BytesToDecimalSiUnits(1499U));
  CHECK("2 kB" == BytesToDecimalSiUnits(1500U));
  CHECK("2 kB" == BytesToDecimalSiUnits(1999U));
  CHECK("12 kB" == BytesToDecimalSiUnits(12499U));
  CHECK("13 kB" == BytesToDecimalSiUnits(12500U));
  CHECK("123 kB" == BytesToDecimalSiUnits(123499U));
  CHECK("124 kB" == BytesToDecimalSiUnits(123500U));
  CHECK("999 kB" == BytesToDecimalSiUnits(999499U));

  CHECK("1 MB" == BytesToDecimalSiUnits(999500U));
  CHECK("1 MB" == BytesToDecimalSiUnits(1000000U));
  CHECK("1 MB" == BytesToDecimalSiUnits(1499999U));
  CHECK("2 MB" == BytesToDecimalSiUnits(1500000U));
  CHECK("12 MB" == BytesToDecimalSiUnits(12499999U));
  CHECK("13 MB" == BytesToDecimalSiUnits(12500000U));
  CHECK("123 MB" == BytesToDecimalSiUnits(123499999U));
  CHECK("124 MB" == BytesToDecimalSiUnits(123500000U));
  CHECK("999 MB" == BytesToDecimalSiUnits(999499999U));

  CHECK("1 GB" == BytesToDecimalSiUnits(999500000U));
  CHECK("1 GB" == BytesToDecimalSiUnits(1000000000U));
  CHECK("1 GB" == BytesToDecimalSiUnits(1499999999U));
  CHECK("2 GB" == BytesToDecimalSiUnits(1500000000U));
  CHECK("12 GB" == BytesToDecimalSiUnits(12499999999U));
  CHECK("13 GB" == BytesToDecimalSiUnits(12500000000U));
  CHECK("123 GB" == BytesToDecimalSiUnits(123499999999U));
  CHECK("124 GB" == BytesToDecimalSiUnits(123500000000U));
  CHECK("999 GB" == BytesToDecimalSiUnits(999499999999U));

  CHECK("1 TB" == BytesToDecimalSiUnits(999500000000U));
  CHECK("1 TB" == BytesToDecimalSiUnits(1000000000000U));
  CHECK("1 TB" == BytesToDecimalSiUnits(1499999999999U));
  CHECK("2 TB" == BytesToDecimalSiUnits(1500000000000U));
  CHECK("12 TB" == BytesToDecimalSiUnits(12499999999999U));
  CHECK("13 TB" == BytesToDecimalSiUnits(12500000000000U));
  CHECK("123 TB" == BytesToDecimalSiUnits(123499999999999U));
  CHECK("124 TB" == BytesToDecimalSiUnits(123500000000000U));
  CHECK("999 TB" == BytesToDecimalSiUnits(999499999999999U));

  CHECK("1 PB" == BytesToDecimalSiUnits(999500000000000U));
  CHECK("1 PB" == BytesToDecimalSiUnits(1000000000000000U));
  CHECK("1 PB" == BytesToDecimalSiUnits(1499999999999999U));
  CHECK("2 PB" == BytesToDecimalSiUnits(1500000000000000U));
  CHECK("12 PB" == BytesToDecimalSiUnits(12499999999999999U));
  CHECK("13 PB" == BytesToDecimalSiUnits(12500000000000000U));
  CHECK("123 PB" == BytesToDecimalSiUnits(123499999999999999U));
  CHECK("124 PB" == BytesToDecimalSiUnits(123500000000000000U));
  CHECK("999 PB" == BytesToDecimalSiUnits(999499999999999999U));

  CHECK("1 EB" == BytesToDecimalSiUnits(999500000000000000U));
  CHECK("1 EB" == BytesToDecimalSiUnits(1000000000000000000U));
  CHECK("1 EB" == BytesToDecimalSiUnits(1499999999999999999U));
  CHECK("2 EB" == BytesToDecimalSiUnits(1500000000000000000U));
  CHECK("9 EB" == BytesToDecimalSiUnits(9499999999999999999U));
  CHECK("10 EB" == BytesToDecimalSiUnits(9500000000000000000U));
  CHECK("12 EB" == BytesToDecimalSiUnits(12499999999999999999U));
  CHECK("13 EB" == BytesToDecimalSiUnits(12500000000000000000U));
  CHECK("18 EB" == BytesToDecimalSiUnits(18446744073709551615U));
}

TEST_CASE("BytesToBinarySiUnits", "[Utils][Unit]") {
  CHECK("0 B" == BytesToBinarySiUnits(0U));
  CHECK("1 B" == BytesToBinarySiUnits(1U));
  CHECK("12 B" == BytesToBinarySiUnits(12U));
  CHECK("123 B" == BytesToBinarySiUnits(123U));
  CHECK("1023 B" == BytesToBinarySiUnits(1023U));

  CHECK("1 KiB" == BytesToBinarySiUnits(1024U));
  CHECK("1 KiB" == BytesToBinarySiUnits(1535U));
  CHECK("2 KiB" == BytesToBinarySiUnits(1536U));
  CHECK("12 KiB" == BytesToBinarySiUnits(12799U));
  CHECK("13 KiB" == BytesToBinarySiUnits(12800U));
  CHECK("123 KiB" == BytesToBinarySiUnits(126463U));
  CHECK("124 KiB" == BytesToBinarySiUnits(126464U));
  CHECK("1023 KiB" == BytesToBinarySiUnits(1048063U));

  CHECK("1 MiB" == BytesToBinarySiUnits(1048064U));
  CHECK("1 MiB" == BytesToBinarySiUnits(1572863U));
  CHECK("2 MiB" == BytesToBinarySiUnits(1572864U));
  CHECK("12 MiB" == BytesToBinarySiUnits(13107199U));
  CHECK("13 MiB" == BytesToBinarySiUnits(13107200U));
  CHECK("123 MiB" == BytesToBinarySiUnits(129499135U));
  CHECK("124 MiB" == BytesToBinarySiUnits(129499136U));
  CHECK("1023 MiB" == BytesToBinarySiUnits(1073217535U));

  CHECK("1 GiB" == BytesToBinarySiUnits(1073217536U));
  CHECK("1 GiB" == BytesToBinarySiUnits(1610612735U));
  CHECK("2 GiB" == BytesToBinarySiUnits(1610612736U));
  CHECK("12 GiB" == BytesToBinarySiUnits(13421772799U));
  CHECK("13 GiB" == BytesToBinarySiUnits(13421772800U));
  CHECK("123 GiB" == BytesToBinarySiUnits(132607115263U));
  CHECK("124 GiB" == BytesToBinarySiUnits(132607115264U));
  CHECK("1023 GiB" == BytesToBinarySiUnits(1098974756863U));

  CHECK("1 TiB" == BytesToBinarySiUnits(1098974756864U));
  CHECK("1 TiB" == BytesToBinarySiUnits(1649267441663U));
  CHECK("2 TiB" == BytesToBinarySiUnits(1649267441664U));
  CHECK("12 TiB" == BytesToBinarySiUnits(13743895347199U));
  CHECK("13 TiB" == BytesToBinarySiUnits(13743895347200U));
  CHECK("123 TiB" == BytesToBinarySiUnits(135789686030335U));
  CHECK("124 TiB" == BytesToBinarySiUnits(135789686030336U));
  CHECK("1023 TiB" == BytesToBinarySiUnits(1125350151028735U));

  CHECK("1 PiB" == BytesToBinarySiUnits(1125350151028736U));
  CHECK("1 PiB" == BytesToBinarySiUnits(1688849860263935U));
  CHECK("2 PiB" == BytesToBinarySiUnits(1688849860263936U));
  CHECK("12 PiB" == BytesToBinarySiUnits(14073748835532799U));
  CHECK("13 PiB" == BytesToBinarySiUnits(14073748835532800U));
  CHECK("123 PiB" == BytesToBinarySiUnits(139048638495064063U));
  CHECK("124 PiB" == BytesToBinarySiUnits(139048638495064064U));
  CHECK("1023 PiB" == BytesToBinarySiUnits(1152358554653425663U));

  CHECK("1 EiB" == BytesToBinarySiUnits(1152358554653425664U));
  CHECK("1 EiB" == BytesToBinarySiUnits(1729382256910270463U));
  CHECK("2 EiB" == BytesToBinarySiUnits(1729382256910270464U));
  CHECK("9 EiB" == BytesToBinarySiUnits(10952754293765046271U));
  CHECK("10 EiB" == BytesToBinarySiUnits(10952754293765046272U));
  CHECK("15 EiB" == BytesToBinarySiUnits(17870283321406128127U));
  CHECK("16 EiB" == BytesToBinarySiUnits(17870283321406128128U));
  CHECK("16 EiB" == BytesToBinarySiUnits(18446744073709551615U));
}

TEST_CASE("RandomStringMultiThread", "[Utils][Unit]") {  // Timeout 60
  std::vector<std::thread> threads;
  for (int i(0); i != 20; ++i)
    threads.push_back(std::move(std::thread([] {
      for (int j(0); j != 1000; ++j)
        RandomString(4096);
    })));
  for (std::thread& thread : threads)
    thread.join();
  CHECK(true);  // To avoid Catch '--warn NoAssertions' triggering a CTest failure.
}

TEST_CASE("RandomStringGenerator", "[Utils][Unit]") {
  std::set<std::string> random_strings;
  const size_t kCount(100);
  const size_t kMaxDuplicates(1);
  for (size_t j = 10; j < 100; ++j) {
    for (size_t i = 0; i < kCount; ++i) {
      random_strings.insert(RandomString(j));
    }
    CHECK(kMaxDuplicates >= (kCount - random_strings.size()));
    random_strings.clear();
  }
}

TEST_CASE("RandomStringSingleThread", "[Utils][Unit]") {  // Timeout 10
  const size_t kStringSize = 4096;
  std::string test1 = RandomAlphaNumericString(kStringSize);
  std::string test2 = RandomAlphaNumericString(kStringSize);
  CHECK(kStringSize == test1.size());
  CHECK(kStringSize == test2.size());
  CHECK(test1 != test2);
  const std::string kAlphaNumeric(
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefgh"
      "ijklmnopqrstuvwxyz");
  for (size_t i = 0; i < kStringSize; ++i) {
    CHECK(std::string::npos != kAlphaNumeric.find(test1.at(i)));
    CHECK(std::string::npos != kAlphaNumeric.find(test2.at(i)));
  }
}

TEST_CASE("HexEncodeDecode", "[Utils][Unit]") {
  maidsafe::test::RunInParallel(100, [&] {
    for (int i = 0; i < 10; ++i) {
      std::string original = RandomString(100);
      std::string encoded = HexEncode(original);
      CHECK(200U == encoded.size());
      std::string decoded = HexDecode(encoded);
      CHECK(original == decoded);
    }
  });
  const std::string kKnownEncoded("0123456789abcdef");
  const std::string kKnownDecoded("\x1\x23\x45\x67\x89\xab\xcd\xef");
  CHECK(kKnownEncoded == HexEncode(kKnownDecoded));
  CHECK(kKnownDecoded == HexDecode(kKnownEncoded));
  CHECK(HexEncode("").empty());
  CHECK(HexDecode("").empty());
  CHECK_THROWS_AS(HexDecode("{"), common_error);
}

TEST_CASE("Base64EncodeDecode", "[Utils][Unit]") {  // Timeout 10
  maidsafe::test::RunInParallel(100, [&] {
    for (int i = 0; i < 10; ++i) {
      std::string original = RandomString(100);
      std::string encoded = Base64Encode(original);
      CHECK(136U == encoded.size());
      std::string decoded = Base64Decode(encoded);
      CHECK(original == decoded);
    }
  });
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
  CHECK(Base64Encode(man) == encoded_man);
  CHECK(man == Base64Decode(Base64Encode(man)));
  CHECK(Base64Encode("pleasure.") == "cGxlYXN1cmUu");
  CHECK("pleasure" == Base64Decode(Base64Encode("pleasure")));
  CHECK(Base64Encode("leasure.") == "bGVhc3VyZS4=");
  CHECK("leasure." == Base64Decode(Base64Encode("leasure.")));
  CHECK(Base64Encode("easure.") == "ZWFzdXJlLg==");
  CHECK("easure." == Base64Decode(Base64Encode("easure.")));
  CHECK(Base64Encode("asure.") == "YXN1cmUu");
  CHECK("asure." == Base64Decode(Base64Encode("asure.")));
  CHECK(Base64Encode("sure.") == "c3VyZS4=");
  CHECK("sure." == Base64Decode(Base64Encode("sure.")));
  // test vectors from RFC4648
  CHECK(Base64Encode("f") == "Zg==");
  CHECK(Base64Encode("fo") == "Zm8=");
  CHECK(Base64Encode("foo") == "Zm9v");
  CHECK(Base64Encode("foob") == "Zm9vYg==");
  CHECK(Base64Encode("fooba") == "Zm9vYmE=");
  CHECK(Base64Encode("foobar") == "Zm9vYmFy");
  CHECK("f" == Base64Decode("Zg=="));
  CHECK("fo" == Base64Decode("Zm8="));
  CHECK("foo" == Base64Decode("Zm9v"));
  CHECK("foob" == Base64Decode("Zm9vYg=="));
  CHECK("fooba" == Base64Decode("Zm9vYmE="));
  CHECK("foobar" == Base64Decode("Zm9vYmFy"));
  CHECK_THROWS_AS(Base64Decode("Zg="), common_error);
  CHECK_THROWS_AS(Base64Decode("Zg"), common_error);
  CHECK_THROWS_AS(Base64Decode("Z"), common_error);
}

TEST_CASE("HexSubstr", "[Utils][Unit]") {
  CHECK(HexSubstr("").empty());
  CHECK("41" == HexSubstr("A"));
  CHECK("58595a" == HexSubstr("XYZ"));
  CHECK("616263646566" == HexSubstr("abcdef"));
  CHECK("616263..656667" == HexSubstr("abcdefg"));
  CHECK(14U == HexSubstr(RandomString(8 + RandomUint32() % 20)).size());
}

TEST_CASE("Base64Substr", "[Utils][Unit]") {
  CHECK(Base64Substr("").empty());
  CHECK("QQ==" == Base64Substr("A"));
  CHECK("WFla" == Base64Substr("XYZ"));
  CHECK("YWJjZGVmZ2g=" == Base64Substr("abcdefgh"));
  CHECK("YWJjZGV..mtsbW5v" == Base64Substr("abcdefghijklmno"));
  CHECK(16U == Base64Substr(RandomString(32 + RandomUint32() % 20)).size());
}

std::string WstringToStringOldMethod(const std::wstring& input) {
  const std::locale kLocale("");
  std::string string_buffer(input.size(), 0);
  std::use_facet<std::ctype<wchar_t>>(kLocale).narrow(
      &input[0], &input[0] + input.size(), '?', &string_buffer[0]);

  return std::string(&string_buffer[0], input.size());
}

TEST_CASE("WstringToString", "[Utils][Unit]") {
#ifdef MAIDSAFE_WIN32
  std::wstring input(L"Test wstring");
  std::string converted(WstringToString(input));
  CHECK(converted == "Test wstring");

  for (int i(0); i != 100; ++i) {
    input.assign(5, static_cast<wchar_t>(RandomUint32() % std::numeric_limits<wchar_t>::max()));
    auto old_method(WstringToStringOldMethod(input));
    try {
      auto new_method(WstringToString(input));
      REQUIRE(new_method == old_method);
    }
    catch(const common_error&) {}
  }
#else
  CHECK(true);
#endif
}

std::wstring StringToWstringOldMethod(const std::string& input) {
  std::wstring buffer(input.size(), L'\0');
  size_t num_chars = mbstowcs(&buffer[0], input.c_str(), input.size());
  buffer.resize(num_chars);
  return buffer;
}

TEST_CASE("StringToWstring", "[Utils][Unit]") {
#ifdef MAIDSAFE_WIN32
  std::string input("Test string");
  std::wstring converted(StringToWstring(input));
  CHECK(converted == L"Test string");

  for (int i(0); i != 100; ++i) {
    input.assign(5, static_cast<char>(RandomUint32() % std::numeric_limits<char>::max()));
    auto old_method(StringToWstringOldMethod(input));
    try {
      auto new_method(StringToWstring(input));
      if (input[0] != '\0') {
        CAPTURE(static_cast<int>(input[0]));
        REQUIRE(new_method == old_method);
      }
    }
    catch(const common_error&) {}
  }
#else
  CHECK(true);
#endif
}

TEST_CASE("TimeFunctions", "[Utils][Unit]") {
  uint64_t ms_since_epoch(GetTimeStamp());
  auto now(bptime::microsec_clock::universal_time());
  auto from_timestamp(TimeStampToPtime(ms_since_epoch));
  CHECK((now - from_timestamp) <= bptime::milliseconds(1));
}

TEST_CASE("RandomNumberGen", "[Utils][Unit]") {  // Timeout 20
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
    CHECK(kMaxDuplicates >= (kCount - random_ints.size()));
    CHECK(kMaxDuplicates >= (kCount - random_uints.size()));
  });
}

TEST_CASE("ReadFile and WriteFile", "[Utils][Unit]") {
  TestPath test_path(CreateTestPath("MaidSafe_TestUtils"));
  fs::path file_path(*test_path / "file.dat");
  std::string file_content;
  REQUIRE_FALSE(fs::exists(file_path));
  CHECK_FALSE(ReadFile(file_path, nullptr));
  CHECK_FALSE(ReadFile(file_path, &file_content));
  CHECK(file_content.empty());
  CHECK_THROWS_AS(ReadFile(file_path), std::exception);
  CHECK_FALSE(WriteFile("", file_content));
  CHECK(WriteFile(file_path, file_content));
  CHECK(fs::exists(file_path));
  CHECK(0 == fs::file_size(file_path));
  CHECK_FALSE(ReadFile(file_path, nullptr));
  CHECK(ReadFile(file_path, &file_content));
  CHECK(file_content.empty());

  file_content = RandomString(3000 + RandomUint32() % 1000);
  CHECK(WriteFile(file_path, file_content));
  CHECK_NOTHROW(ReadFile(file_path));
  std::string file_content_in;
  CHECK(ReadFile(file_path, &file_content_in));
  CHECK(file_content == file_content_in);

  CHECK(WriteFile(file_path, "moo"));
  CHECK(ReadFile(file_path, &file_content_in));
  CHECK("moo" == file_content_in);
}

TEST_CASE("Sleep", "[Utils][Unit]") {
  bptime::ptime first_time(bptime::microsec_clock::universal_time());
  bptime::ptime second_time(bptime::microsec_clock::universal_time());
  CHECK((second_time - first_time).total_milliseconds() < 100);
  Sleep(std::chrono::milliseconds(100));
  bptime::ptime third_time(bptime::microsec_clock::universal_time());
  CHECK((third_time - first_time).total_milliseconds() >= 100);
}

TEST_CASE("GetHomeDir", "[Utils][Unit]") {
  CHECK_FALSE(GetHomeDir().empty());
  LOG(kInfo) << "Your home directory is " << GetHomeDir();
}

TEST_CASE("GetUserAppDir", "[Utils][Unit]") {
  CHECK_FALSE(GetUserAppDir().empty());
  LOG(kInfo) << "Your user app directory is " << GetUserAppDir();
}

TEST_CASE("GetSystemAppSupportDir", "[Utils][Unit]") {
  CHECK_FALSE(GetSystemAppSupportDir().empty());
  LOG(kInfo) << "Your system app support directory is " << GetSystemAppSupportDir();
}

TEST_CASE("GetAppInstallDir", "[Utils][Unit]") {
  CHECK_FALSE(GetAppInstallDir().empty());
  LOG(kInfo) << "Your app install directory is " << GetAppInstallDir();
}

TEST_CASE("AppDir", "[Utils][Unit]") {
  CHECK(GetSystemAppSupportDir() != GetUserAppDir());
  CHECK(GetSystemAppSupportDir() != GetHomeDir());
  CHECK(GetUserAppDir() != GetHomeDir());
  std::string home(GetHomeDir().string());
  std::string system(GetSystemAppSupportDir().string());
  std::string user_app(GetUserAppDir().string());
  CHECK(user_app.find(home) != std::string::npos);
  CHECK(system.find(home) == std::string::npos);
}

TEST_CASE("Concurrency", "[Utils][Unit]") {
  CHECK(Concurrency()>= 2U);
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

TEST_CASE("CreateTestPath", "[Utils][Unit]") {
  fs::path test_path;
  boost::system::error_code error_code;
  {
    TestPath test_path_ptr(CreateTestPath());
    test_path = *test_path_ptr;
    CHECK_FALSE(test_path.empty());
    CHECK(fs::exists(test_path, error_code));
    INFO(error_code.message());
    CHECK(0 == error_code.value());
  }
  CHECK_FALSE(fs::exists(test_path, error_code));
  INFO(error_code.message());
  CHECK(boost::system::errc::no_such_file_or_directory == error_code.value());
  {
    TestPath test_path_ptr(CreateTestPath("Another"));
    test_path = *test_path_ptr;
    CHECK_FALSE(test_path.empty());
    CHECK(fs::exists(test_path, error_code));
    INFO(error_code.message());
    CHECK(0 == error_code.value());
  }
  CHECK_FALSE(fs::exists(test_path, error_code));
  INFO(error_code.message());
  CHECK(boost::system::errc::no_such_file_or_directory == error_code.value());
  // Ensure we're able to cope with error cases
  auto empty_path(new fs::path);
  CleanupTest(empty_path);
  CHECK(!empty_path);
  fs::path* non_existent(new fs::path(std::string(100, 'a')));
  CleanupTest(non_existent);
  CHECK(!non_existent);
}

TEST_CASE("GetProcessId", "[Process][Unit]") {
  CHECK(process::GetProcessId() > 0);
}

}  // namespace test

}  // namespace maidsafe
