/*  Copyright 2013 MaidSafe.net limited

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

#include "maidsafe/common/error.h"

#include "boost/throw_exception.hpp"

#include "maidsafe/common/log.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"

namespace maidsafe {

namespace detail {

namespace test {

TEST(ErrorsTest, BEH_ErrorCodeErrorCondition) {
  common_error null_pointer_error(MakeError(CommonErrors::null_pointer));
  asymm_error data_empty_error(MakeError(AsymmErrors::data_empty));
  EXPECT_TRUE(null_pointer_error.code() != data_empty_error.code());
  EXPECT_TRUE(null_pointer_error.code() == make_error_code(CommonErrors::null_pointer));

  std::error_condition null_pointer_condition(make_error_condition(CommonErrors::null_pointer));
  std::error_condition data_empty_condition(make_error_condition(AsymmErrors::data_empty));
  EXPECT_TRUE(null_pointer_condition != data_empty_condition);
  EXPECT_TRUE(null_pointer_condition == make_error_condition(CommonErrors::null_pointer));

  std::error_condition null_pointer_default_error_condition(
      GetCommonCategory().default_error_condition(null_pointer_error.code().value()));
  std::error_condition data_empty_default_error_condition(
      GetAsymmCategory().default_error_condition(data_empty_error.code().value()));
  EXPECT_TRUE(null_pointer_default_error_condition == data_empty_default_error_condition);

  EXPECT_TRUE(GetCommonCategory().equivalent(null_pointer_error.code(),
                                             null_pointer_condition.value()));
  EXPECT_FALSE(GetCommonCategory().equivalent(null_pointer_error.code().value(),
                                             null_pointer_condition));
  EXPECT_FALSE(GetCommonCategory().equivalent(data_empty_error.code(),
                                             null_pointer_condition.value()));
  EXPECT_FALSE(GetCommonCategory().equivalent(data_empty_error.code().value(),
                                             null_pointer_condition));
}

TEST(ErrorsTest, BEH_ErrorCodesThrownAsBoostExceptions) {
  // Catch as specific error type
  try {
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::file_too_large));
  }
  catch (const common_error& e) {
    LOG(kWarning) << e.what();
    LOG(kError) << boost::diagnostic_information(e);
  }

  // Catch as maidsafe_error
  try {
    BOOST_THROW_EXCEPTION(MakeError(AsymmErrors::decryption_error));
  }
  catch (const maidsafe_error& e) {
    LOG(kWarning) << e.what();
    LOG(kError) << boost::diagnostic_information(e);
  }

  // Catch as std::exception
  try {
    BOOST_THROW_EXCEPTION(MakeError(PassportErrors::id_already_exists));
  }
  catch (const std::exception& e) {
    LOG(kWarning) << e.what();
    LOG(kError) << boost::diagnostic_information(e);
  }

  // Catch as boost::exception
  try {
    BOOST_THROW_EXCEPTION(MakeError(EncryptErrors::bad_sequence));
  }
  catch (const boost::exception& e) {
    LOG(kError) << boost::diagnostic_information(e);
  }

  // Catch as any type
  try {
    BOOST_THROW_EXCEPTION(MakeError(DriveErrors::failed_to_mount));
  }
  catch (...) {
    LOG(kError) << boost::current_exception_diagnostic_information();
  }

  // Use plain throw
  try {
    throw MakeError(NfsErrors::failed_to_get_data);
  }
  catch (const nfs_error& e) {
    LOG(kWarning) << e.what();
    LOG(kError) << boost::diagnostic_information(e);
  }
}

TEST(ErrorsTest, BEH_SerialisingAndParsingErrors) {
  common_error hashing_error{ MakeError(CommonErrors::hashing_error) };
  maidsafe_error::serialised_type serialised{ Serialise(hashing_error) };
  EXPECT_TRUE(hashing_error.code() == Parse(serialised).code());
  EXPECT_TRUE(std::string{ hashing_error.what() } == std::string{ Parse(serialised).what() });

  vault_manager_error listening_error{ MakeError(VaultManagerErrors::failed_to_listen) };
  serialised = Serialise(listening_error);
  EXPECT_TRUE(listening_error.code() == Parse(serialised).code());
  EXPECT_TRUE(std::string{ listening_error.what() } == std::string{ Parse(serialised).what() });
  EXPECT_TRUE(hashing_error.code() != Parse(serialised).code());
  EXPECT_TRUE(std::string{ hashing_error.what() } != std::string{ Parse(serialised).what() });

  EXPECT_THROW(IntToError(0), maidsafe_error);
  EXPECT_TRUE(IntToError(-100000).code() == MakeError(CommonErrors::success).code());
  EXPECT_TRUE(ErrorToInt(MakeError(CommonErrors::success)) == -100000);
}

template <typename ErrorType>
class MaidSafeErrorTest : public testing::Test {};

template <typename ErrorType>
struct EnumClass;

template <>
struct EnumClass<common_error> { typedef CommonErrors type; };

template <>
struct EnumClass<asymm_error> { typedef AsymmErrors type; };

template <>
struct EnumClass<passport_error> { typedef PassportErrors type; };

template <>
struct EnumClass<encrypt_error> { typedef EncryptErrors type; };

template <>
struct EnumClass<routing_error> { typedef RoutingErrors type; };

template <>
struct EnumClass<nfs_error> { typedef NfsErrors type; };

template <>
struct EnumClass<drive_error> { typedef DriveErrors type; };

template <>
struct EnumClass<vault_error> { typedef VaultErrors type; };

template <>
struct EnumClass<vault_manager_error> { typedef VaultManagerErrors type; };

template <>
struct EnumClass<api_error> { typedef ApiErrors type; };

typedef testing::Types<common_error,
                       asymm_error,
                       passport_error,
                       encrypt_error,
                       routing_error,
                       nfs_error,
                       drive_error,
                       vault_error,
                       vault_manager_error,
                       api_error> AllErrorTypes;

TYPED_TEST_CASE(MaidSafeErrorTest, AllErrorTypes);

TYPED_TEST(MaidSafeErrorTest, BEH_ConstructorsAndHelpers) {
  const std::error_code kCode{ make_error_code(typename EnumClass<TypeParam>::type(1)) };
  make_error_condition(typename EnumClass<TypeParam>::type(1));
  const std::string kWhat{ RandomAlphaNumericString(10) };

  TypeParam error(kCode, kWhat);
  EXPECT_EQ(kCode, error.code());
  EXPECT_NE(std::string::npos, std::string{ error.what() }.find_first_of(kWhat));

  error = TypeParam(kCode, kWhat.c_str());
  EXPECT_EQ(kCode, error.code());
  EXPECT_NE(std::string::npos, std::string{ error.what() }.find_first_of(kWhat));

  error = TypeParam(kCode);
  EXPECT_EQ(kCode, error.code());
  EXPECT_EQ(kCode.message(), error.what());

  error = TypeParam(1, kCode.category(), kWhat);
  EXPECT_EQ(kCode, error.code());
  EXPECT_NE(std::string::npos, std::string{ error.what() }.find_first_of(kWhat));

  error = TypeParam(1, kCode.category(), kWhat.c_str());
  EXPECT_EQ(kCode, error.code());
  EXPECT_NE(std::string::npos, std::string{ error.what() }.find_first_of(kWhat));

  error = TypeParam(1, kCode.category());
  EXPECT_EQ(kCode, error.code());
  EXPECT_EQ(kCode.message(), error.what());

  EXPECT_EQ(error.code(), MakeError(typename EnumClass<TypeParam>::type(1)).code());

  EXPECT_TRUE(std::is_error_code_enum<typename EnumClass<TypeParam>::type>::value);
}

}  // namespace test

}  // namespace detail

}  // namespace maidsafe
