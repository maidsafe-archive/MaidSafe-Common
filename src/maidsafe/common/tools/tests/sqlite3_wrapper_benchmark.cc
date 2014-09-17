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

#include "maidsafe/common/tools/sqlite3_wrapper_benchmark.h"

// #include "boost/filesystem/operations.hpp"

#include "maidsafe/common/test.h"

namespace maidsafe {

namespace benchmark {

Sqlite3WrapperBenchmark::Sqlite3WrapperBenchmark()
  : database_path(), ticking_clock() {}

void Sqlite3WrapperBenchmark::Run() {
  maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_TestUtils"));
  database_path = boost::filesystem::path(*test_path / "sqlite_wrapper_benchmark");
  TenThousandEndpointString();
}

void Sqlite3WrapperBenchmark::TenThousandEndpointString() {
  TLOG(kGreen) << "Inserting 10000 endpoint strings within one transaction\n";
  std::vector<std::string> ten_thousand_strings;
  for (int i(0); i < 10000; ++i)
    ten_thousand_strings.push_back(RandomAlphaNumericString(20));

  {
    ticking_clock.restart();
    sqlite::Database database(database_path, sqlite::Mode::kReadWriteCreate);
    sqlite::Tranasction transaction(database);
    std::string query(
        "CREATE TABLE IF NOT EXISTS BOOTSTRAP_CONTACTS("
        "ENDPOINT TEXT  PRIMARY KEY NOT NULL);");
    PrepareTable(database, query);
    InsertEndpointStrings(database, ten_thousand_strings);
    transaction.Commit();
  }
  auto time_taken(ticking_clock.elapsed());
  TLOG(kGreen) << "test completed in " << time_taken << " seconds\n";

  std::vector<std::string> result;
  ReadEndpointStrings(result);
  if (result.size() == ten_thousand_strings.size()) {
    for (int i(0); i < 10000; ++i)
      if (result.at(i) != ten_thousand_strings.at(i)) {
        TLOG(kRed) << "entry stored with dis-order\n";
        return;
      }
  } else {
      TLOG(kRed) << "inserted 10000 strings, only got "
               << result.size() << " in database\n";
  }
}

void Sqlite3WrapperBenchmark::PrepareTable(sqlite::Database& database,
                                           const std::string& query) {
  sqlite::Statement statement{database, query};
  statement.Step();
  statement.Reset();
}

void Sqlite3WrapperBenchmark::InsertEndpointStrings(sqlite::Database& database,
    const std::vector<std::string>& endpoint_strings) {
  std::string query("INSERT OR REPLACE INTO BOOTSTRAP_CONTACTS (ENDPOINT) VALUES (?)");
  sqlite::Statement statement{database, query};
  for (const auto& endpoint_string : endpoint_strings) {
    statement.BindText(1, endpoint_string);
    statement.Step();
    statement.Reset();
  }
}

void Sqlite3WrapperBenchmark::ReadEndpointStrings(std::vector<std::string>& result) {
  sqlite::Database database(database_path, sqlite::Mode::kReadOnly);
  std::string query("SELECT * from BOOTSTRAP_CONTACTS");
  sqlite::Statement statement{database, query};
  for (;;) {
    if (statement.Step() == sqlite::StepResult::kSqliteRow) {
      std::string endpoint_string = statement.ColumnText(0);
      result.push_back(endpoint_string);
    } else {
      break;
    }
  }
}

}  // namespace benchmark

}  // namespace maidsafe
