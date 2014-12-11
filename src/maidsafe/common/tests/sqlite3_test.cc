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

#include <mutex>
#include <vector>

#include "boost/filesystem/operations.hpp"

#include "maidsafe/common/sqlite3_wrapper.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"


namespace fs = boost::filesystem;

namespace maidsafe {
namespace test {


TEST(Sqlite3WrapperTest, FUNC_ReadInvalidDataBase) {
  maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_TestUtils"));
  fs::path test_db(*test_path / "test_db-file");
  std::string file_content(RandomString(3000 + RandomUint32() % 1000));
  ASSERT_FALSE(fs::exists(test_db));
  EXPECT_TRUE(WriteFile(test_db, file_content));
  EXPECT_TRUE(fs::exists(test_db));

  sqlite::Database database(test_db, sqlite::Mode::kReadWriteCreate);
  std::string query(
      "CREATE TABLE IF NOT EXISTS TEST_ME("
      "TEST_DATA TEXT  PRIMARY KEY NOT NULL);");
  EXPECT_THROW(sqlite::Statement(database, query), std::exception);
}

TEST(Sqlite3WrapperTest, FUNC_WriteEmptDataBase) {
  maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_TestUtils"));
  fs::path test_db(*test_path / "test_db-file");
  ASSERT_FALSE(fs::exists(test_db));
  EXPECT_TRUE(WriteFile(test_db, ""));
  EXPECT_TRUE(fs::exists(test_db));

  sqlite::Database database(test_db, sqlite::Mode::kReadWriteCreate);
  std::string query(
      "CREATE TABLE IF NOT EXISTS TEST_ME("
      "TEST_DATA TEXT  PRIMARY KEY NOT NULL);");
  ASSERT_NO_THROW(sqlite::Statement(database, query));
  sqlite::Statement statement(database, query);
  statement.Step();
  statement.Reset();
}

TEST(Sqlite3WrapperTest, FUNC_WriteNewDataBase) {
  maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_TestUtils"));
  fs::path test_db(*test_path / "test_db-file");

  sqlite::Database database(test_db, sqlite::Mode::kReadWriteCreate);
  std::string query(
      "CREATE TABLE IF NOT EXISTS TEST_ME("
      "TEST_DATA TEXT  PRIMARY KEY NOT NULL);");
  ASSERT_NO_THROW(sqlite::Statement(database, query));
  sqlite::Statement statement(database, query);
  statement.Step();
  statement.Reset();
}

TEST(Sqlite3WrapperTest, FUNC_WriteExistingDataBase) {
  maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_TestUtils"));
  fs::path test_db(*test_path / "test_db-file");

  sqlite::Database database(test_db, sqlite::Mode::kReadWriteCreate);
  std::string create(
      "CREATE TABLE IF NOT EXISTS TEST_ME("
      "TEST_DATA TEXT  PRIMARY KEY NOT NULL);");
  ASSERT_NO_THROW(sqlite::Statement(database, create));
  sqlite::Statement statement(database, create);
  statement.Step();
  statement.Reset();

  std::string query("INSERT OR REPLACE INTO TEST_ME (TEST_DATA) VALUES (?)");
  sqlite::Statement command{database, query};
  // write 10i00 entries
  for (const auto& element : std::vector<std::string>(1000, RandomString(4))) {
    command.BindText(1, element);
    command.Step();
    command.Reset();
  }
}

TEST(Sqlite3WrapperTest, FUNC_WriteRead) {
  maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_TestUtils"));
  fs::path test_db(*test_path / "test_db-file");

  sqlite::Database database(test_db, sqlite::Mode::kReadWriteCreate);
  sqlite::Statement statement(database,
                              "CREATE TABLE IF NOT EXISTS TEST_ME("
                              "TEST_DATA TEXT  PRIMARY KEY NOT NULL);");
  statement.Step();
  statement.Reset();

  sqlite::Statement insert{database, "INSERT OR REPLACE INTO TEST_ME (TEST_DATA) VALUES (?)"};
  // write 1000 entries
  std::vector<std::string> test_data(1000, RandomString(4));

  for (const auto& element : test_data) {
    insert.BindText(1, element);
    insert.Step();
    insert.Reset();
  }


  for (const auto& element : test_data) {
    RunInParallel(3, [element, &database] {
      sqlite::Statement find{database, "SELECT * FROM TEST_ME WHERE TEST_DATA=?"};
      find.BindText(1, element);
      find.Step();  // == sqlite::StepResult::kSqliteRow;
      EXPECT_EQ(element, find.ColumnText(0));
      find.Reset();
    });
  }
}

}  // namespace test
}  // namespace maidsafe
