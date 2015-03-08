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

#include "maidsafe/common/encode.h"

//#include <algorithm>
//#include <chrono>
//#include <cstdlib>
//#include <cwchar>
//#include <set>
//#include <thread>
//#include <vector>
//
//#include "boost/filesystem/operations.hpp"
//#include "boost/filesystem/path.hpp"
//
//#include "maidsafe/common/application_support_directories.h"
//#include "maidsafe/common/config.h"
//#include "maidsafe/common/crypto.h"
//#include "maidsafe/common/log.h"
//#include "maidsafe/common/process.h"
#include "maidsafe/common/test.h"

namespace maidsafe {

namespace test {

TEST(UtilsTest, BEH_HexEncodeDecode) {
  bool expected_sizes_ok{true}, decoded_ok{true};

  maidsafe::test::RunInParallel(100, [&] {
    for (int i = 0; i < 10; ++i) {
      std::string original = RandomString(100);
      std::string encoded = hex::Encode(original);
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
  EXPECT_EQ(kKnownEncoded, hex::Encode(kKnownDecoded));
  EXPECT_EQ(kKnownDecoded, HexDecode(kKnownEncoded));
  EXPECT_TRUE(hex::Encode("").empty());
  EXPECT_TRUE(HexDecode("").empty());
  EXPECT_THROW(HexDecode("{"), common_error);
}

TEST(UtilsTest, BEH_Base64EncodeDecode) {
  bool expected_sizes_ok{true}, decoded_ok{true};

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

}  // namespace test

}  // namespace maidsafe
