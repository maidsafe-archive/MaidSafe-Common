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

#include "maidsafe/common/bounded_string.h"
#include "maidsafe/common/convert.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/types.h"
#include "maidsafe/common/utils.h"

namespace maidsafe {

namespace test {

template <typename T>
struct UnderlyingString;

template <>
struct UnderlyingString<std::string> {
  using Type = std::string;
};

template <>
struct UnderlyingString<std::vector<byte>> {
  using Type = std::vector<byte>;
};

template <>
struct UnderlyingString<
    maidsafe::detail::BoundedString<1, static_cast<std::size_t>(-1), std::string>> {
  using Type = std::string;
};

template <>
struct UnderlyingString<NonEmptyString> {
  using Type = std::vector<byte>;
};

template <typename T>
class EncodeTest : public testing::Test {
 protected:
  using UnderlyingStringType = typename UnderlyingString<T>::Type;
  T ToTestType(const std::string& input) {
    return T(UnderlyingStringType(input.begin(), input.end()));
  }
};

template <typename T>
std::string ToString(const T& input);

template <>
std::string ToString<std::string>(const std::string& input) {
  return input;
}

template <>
std::string ToString<std::vector<byte>>(const std::vector<byte>& input) {
  return convert::ToString(input);
}

using EncodeTestTypes =
    testing::Types<std::string, std::vector<byte>,
                   maidsafe::detail::BoundedString<1, static_cast<std::size_t>(-1), std::string>,
                   NonEmptyString>;

TYPED_TEST_CASE(EncodeTest, EncodeTestTypes);

TYPED_TEST(EncodeTest, BEH_Hex) {
  bool expected_sizes_ok{true}, decoded_ok{true};

  for (int i = 0; i < 10; ++i) {
    std::string original = RandomString(100);
    std::string encoded = hex::Encode(this->ToTestType(original));
    if (encoded.size() != 200U)
      expected_sizes_ok = false;
    std::string decoded = hex::DecodeToString(encoded);
    if (decoded != ToString(original))
      decoded_ok = false;
    std::vector<byte> decoded_bytes = hex::DecodeToBytes(encoded);
    if (convert::ToString(decoded_bytes) != ToString(original))
      decoded_ok = false;
  }

  EXPECT_TRUE(expected_sizes_ok);
  EXPECT_TRUE(decoded_ok);

  const std::string kKnownEncoded("0123456789abcdef");
  const std::string kKnownDecoded("\x1\x23\x45\x67\x89\xab\xcd\xef");
  EXPECT_EQ(kKnownEncoded, hex::Encode(this->ToTestType(kKnownDecoded)));
  EXPECT_EQ(kKnownDecoded, hex::DecodeToString(kKnownEncoded));
  EXPECT_EQ(kKnownDecoded, convert::ToString(hex::DecodeToBytes(kKnownEncoded)));

  typename TestFixture::UnderlyingStringType empty_str;
  EXPECT_TRUE(hex::Encode(empty_str).empty());
  EXPECT_TRUE(hex::DecodeToString("").empty());
  EXPECT_TRUE(hex::DecodeToBytes("").empty());
  EXPECT_THROW(hex::DecodeToString("{"), common_error);
  EXPECT_THROW(hex::DecodeToBytes("{"), common_error);
}

TYPED_TEST(EncodeTest, BEH_Base64) {
  bool expected_sizes_ok{true}, decoded_ok{true};

  for (int i = 0; i < 10; ++i) {
    std::string original = RandomString(100);
    std::string encoded = base64::Encode(this->ToTestType(original));
    if (encoded.size() != 136U)
      expected_sizes_ok = false;
    std::string decoded = base64::DecodeToString(encoded);
    if (decoded != ToString(original))
      decoded_ok = false;
    std::vector<byte> decoded_bytes = base64::DecodeToBytes(encoded);
    if (convert::ToString(decoded_bytes) != ToString(original))
      decoded_ok = false;
  }

  EXPECT_TRUE(expected_sizes_ok);
  EXPECT_TRUE(decoded_ok);

  // from wikipedia
  std::string input_str(
      "Man is distinguished, not only by his reason, but by this singular passion from other "
      "animals, which is a lust of the mind, that by a perseverance of delight in the continued "
      "and indefatigable generation of knowledge, exceeds the short vehemence of any carnal "
      "pleasure.");
  std::vector<byte> input_vec(input_str.begin(), input_str.end());

  std::string encoded(
      "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlzIHNpbmd1bGFyIHBh"
      "c3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJz"
      "ZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yg"
      "a25vd2xlZGdlLCBleGNlZWRzIHRoZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=");
  EXPECT_EQ(base64::Encode(this->ToTestType(input_str)), encoded);
  EXPECT_EQ(input_str, base64::DecodeToString(base64::Encode(this->ToTestType(input_str))));
  EXPECT_EQ(input_vec, base64::DecodeToBytes(base64::Encode(this->ToTestType(input_str))));

  EXPECT_EQ(base64::Encode(this->ToTestType("pleasure.")), "cGxlYXN1cmUu");
  EXPECT_EQ("pleasure", base64::DecodeToString(base64::Encode(this->ToTestType("pleasure"))));
  EXPECT_EQ("pleasure",
            convert::ToString(base64::DecodeToBytes(base64::Encode(this->ToTestType("pleasure")))));

  EXPECT_EQ(base64::Encode(this->ToTestType("leasure.")), "bGVhc3VyZS4=");
  EXPECT_EQ("leasure.", base64::DecodeToString(base64::Encode(this->ToTestType("leasure."))));
  EXPECT_EQ("leasure.",
            convert::ToString(base64::DecodeToBytes(base64::Encode(this->ToTestType("leasure.")))));

  EXPECT_EQ(base64::Encode(this->ToTestType("easure.")), "ZWFzdXJlLg==");
  EXPECT_EQ("easure.", base64::DecodeToString(base64::Encode(this->ToTestType("easure."))));
  EXPECT_EQ("easure.",
            convert::ToString(base64::DecodeToBytes(base64::Encode(this->ToTestType("easure.")))));

  EXPECT_EQ(base64::Encode(this->ToTestType("asure.")), "YXN1cmUu");
  EXPECT_EQ("asure.", base64::DecodeToString(base64::Encode(this->ToTestType("asure."))));
  EXPECT_EQ("asure.",
            convert::ToString(base64::DecodeToBytes(base64::Encode(this->ToTestType("asure.")))));

  EXPECT_EQ(base64::Encode(this->ToTestType("sure.")), "c3VyZS4=");
  EXPECT_EQ("sure.", base64::DecodeToString(base64::Encode(this->ToTestType("sure."))));
  EXPECT_EQ("sure.",
            convert::ToString(base64::DecodeToBytes(base64::Encode(this->ToTestType("sure.")))));

  // test vectors from RFC4648
  EXPECT_EQ(base64::Encode(this->ToTestType("f")), "Zg==");
  EXPECT_EQ(base64::Encode(this->ToTestType("fo")), "Zm8=");
  EXPECT_EQ(base64::Encode(this->ToTestType("foo")), "Zm9v");
  EXPECT_EQ(base64::Encode(this->ToTestType("foob")), "Zm9vYg==");
  EXPECT_EQ(base64::Encode(this->ToTestType("fooba")), "Zm9vYmE=");
  EXPECT_EQ(base64::Encode(this->ToTestType("foobar")), "Zm9vYmFy");
  EXPECT_EQ("f", base64::DecodeToString("Zg=="));
  EXPECT_EQ("f", convert::ToString(base64::DecodeToBytes("Zg==")));
  EXPECT_EQ("fo", base64::DecodeToString("Zm8="));
  EXPECT_EQ("fo", convert::ToString(base64::DecodeToBytes("Zm8=")));
  EXPECT_EQ("foo", base64::DecodeToString("Zm9v"));
  EXPECT_EQ("foo", convert::ToString(base64::DecodeToBytes("Zm9v")));
  EXPECT_EQ("foob", base64::DecodeToString("Zm9vYg=="));
  EXPECT_EQ("foob", convert::ToString(base64::DecodeToBytes("Zm9vYg==")));
  EXPECT_EQ("fooba", base64::DecodeToString("Zm9vYmE="));
  EXPECT_EQ("fooba", convert::ToString(base64::DecodeToBytes("Zm9vYmE=")));
  EXPECT_EQ("foobar", base64::DecodeToString("Zm9vYmFy"));
  EXPECT_EQ("foobar", convert::ToString(base64::DecodeToBytes("Zm9vYmFy")));
  EXPECT_THROW(base64::DecodeToString("Zg="), common_error);
  EXPECT_THROW(base64::DecodeToBytes("Zg="), common_error);
  EXPECT_THROW(base64::DecodeToString("Zg"), common_error);
  EXPECT_THROW(base64::DecodeToBytes("Zg"), common_error);
  EXPECT_THROW(base64::DecodeToString("Z"), common_error);
  EXPECT_THROW(base64::DecodeToBytes("Z"), common_error);
}

TYPED_TEST(EncodeTest, BEH_HexSubstr) {
  EXPECT_TRUE(hex::Substr(typename TestFixture::UnderlyingStringType()).empty());
  EXPECT_EQ("41", hex::Substr(this->ToTestType("A")));
  EXPECT_EQ("58595a", hex::Substr(this->ToTestType("XYZ")));
  EXPECT_EQ("61626364656667", hex::Substr(this->ToTestType("abcdefg")));
  EXPECT_EQ("616263..666768", hex::Substr(this->ToTestType("abcdefgh")));
  EXPECT_EQ(14U, hex::Substr(this->ToTestType(RandomString(8, 100))).size());
}

TYPED_TEST(EncodeTest, BEH_Base64Substr) {
  EXPECT_TRUE(base64::Substr(typename TestFixture::UnderlyingStringType()).empty());
  EXPECT_EQ("QQ==", base64::Substr(this->ToTestType("A")));
  EXPECT_EQ("WFla", base64::Substr(this->ToTestType("XYZ")));
  EXPECT_EQ("YWJjZGVmZ2g=", base64::Substr(this->ToTestType("abcdefgh")));
  EXPECT_EQ("YWJjZG..tsbW5v", base64::Substr(this->ToTestType("abcdefghijklmno")));
  EXPECT_EQ(14U, base64::Substr(this->ToTestType(RandomString(32, 100))).size());
}

}  // namespace test

}  // namespace maidsafe
