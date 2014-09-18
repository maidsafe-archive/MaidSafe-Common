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
  : database_path(), ticking_clock(), ten_thousand_strings() {}

void Sqlite3WrapperBenchmark::Run() {
  maidsafe::test::TestPath test_path(maidsafe::test::CreateTestPath("MaidSafe_TestUtils"));
  database_path = boost::filesystem::path(*test_path / "sqlite_wrapper_benchmark");
  for (int i(0); i < 10000; ++i)
    ten_thousand_strings.push_back(RandomAlphaNumericString(20));

  EndpointStringsSingleTransaction();
  EndpointStringsIndividualTransaction();
  EndpointStringsParallelTransaction();
  EndpointStringsParallelDelete();
}

void Sqlite3WrapperBenchmark::EndpointStringsSingleTransaction() {
  TLOG(kGreen) << "\nInserting 10k endpoint strings within one transaction\n";
  {
    ticking_clock.restart();
    sqlite::Database database(database_path, sqlite::Mode::kReadWriteCreate);
    std::string query(
        "CREATE TABLE IF NOT EXISTS EndpointStringsSingleTransaction ("
        "ENDPOINT TEXT  PRIMARY KEY NOT NULL);");
    PrepareTable(database, query);
    sqlite::Tranasction transaction{database};
    UpdateEndpointStrings(database, ten_thousand_strings,
      "INSERT OR REPLACE INTO EndpointStringsSingleTransaction (ENDPOINT) VALUES (?)");
    transaction.Commit();
  }
  TLOG(kGreen) << "test completed in " << ticking_clock.elapsed() << " seconds\n";
  CheckEndpointStringsTestResult(ten_thousand_strings,
                                 "SELECT * from EndpointStringsSingleTransaction");
}

void Sqlite3WrapperBenchmark::EndpointStringsIndividualTransaction() {
  TLOG(kGreen) << "\nInserting 10k endpoint strings, individual transaction for each\n";
  {
    ticking_clock.restart();
    sqlite::Database database(database_path, sqlite::Mode::kReadWriteCreate);
    std::string query(
        "CREATE TABLE IF NOT EXISTS EndpointStringsIndividualTransaction ("
        "ENDPOINT TEXT  PRIMARY KEY NOT NULL);");
    PrepareTable(database, query);
    for (const auto& endpoint_string : ten_thousand_strings) {
      sqlite::Tranasction transaction{database};
      std::vector<std::string> endpoint_string_vector(1, endpoint_string);
      UpdateEndpointStrings(database, endpoint_string_vector,
        "INSERT OR REPLACE INTO EndpointStringsIndividualTransaction (ENDPOINT) VALUES (?)");
      transaction.Commit();
    }
  }
  TLOG(kGreen) << "test completed in " << ticking_clock.elapsed() << " seconds\n";
  CheckEndpointStringsTestResult(ten_thousand_strings,
                                 "SELECT * from EndpointStringsIndividualTransaction");
}

// OK with 1000 and 5000 entries, getting missing entries with 10k entries,
// but no corrupted entry in database (only missing)
void Sqlite3WrapperBenchmark::EndpointStringsParallelTransaction() {
  TLOG(kGreen) << "\nInserting 10k endpoint strings with 20 parallel threads,"
               << " and individual transaction for each string\n";

  ticking_clock.restart();
  sqlite::Database database(database_path, sqlite::Mode::kReadWriteCreate);
  std::string query(
      "CREATE TABLE IF NOT EXISTS EndpointStringsParallelTransaction ("
      "ENDPOINT TEXT  PRIMARY KEY NOT NULL);");
  PrepareTable(database, query);

  std::mutex mutex;
  size_t thread_count(20), index(0);
  ::maidsafe::test::RunInParallel(thread_count, [&] {
    std::vector<std::string> endpoint_string_vector;
    for (size_t i(0); i < (ten_thousand_strings.size() / thread_count); ++i) {
      {
        std::lock_guard<std::mutex> lock{ mutex };
        endpoint_string_vector.push_back(ten_thousand_strings.at(index));
        ++index;
      }
      sqlite::Tranasction transaction{database};
      UpdateEndpointStrings(database, endpoint_string_vector,
        "INSERT OR REPLACE INTO EndpointStringsParallelTransaction (ENDPOINT) VALUES (?)");
      transaction.Commit();
    }
  });
  LOG(kVerbose) << "index : " << index;
  TLOG(kGreen) << "test completed in " << ticking_clock.elapsed() << " seconds\n";
  CheckEndpointStringsTestResult(ten_thousand_strings,
                                 "SELECT * from EndpointStringsParallelTransaction",
                                 false);
}

void Sqlite3WrapperBenchmark::EndpointStringsParallelDelete() {
  TLOG(kGreen) << "\nParallel Deletion (20 threads) from the database"
               << " containing 10k endpoint strings\n";
  sqlite::Database database(database_path, sqlite::Mode::kReadWriteCreate);
  std::string query(
      "CREATE TABLE IF NOT EXISTS EndpointStringsParallelDelete ("
      "ENDPOINT TEXT  PRIMARY KEY NOT NULL);");
  PrepareTable(database, query);
  {
    // populate the database with 10k entries
    sqlite::Tranasction transaction{database};
    UpdateEndpointStrings(database, ten_thousand_strings,
      "INSERT OR REPLACE INTO EndpointStringsParallelDelete (ENDPOINT) VALUES (?)");
    transaction.Commit();
  }

  ticking_clock.restart();
  std::mutex mutex;
  size_t thread_count(20), index(0);
  ::maidsafe::test::RunInParallel(thread_count, [&] {
    std::vector<std::string> endpoint_string_vector;
    for (size_t i(0); i < (ten_thousand_strings.size() / thread_count); ++i) {
      {
        std::lock_guard<std::mutex> lock{ mutex };
        endpoint_string_vector.push_back(ten_thousand_strings.at(index));
        ++index;
      }
      sqlite::Tranasction transaction{database};
      UpdateEndpointStrings(database, endpoint_string_vector,
        "DELETE From EndpointStringsParallelDelete WHERE ENDPOINT=?");
      transaction.Commit();
    }
  });
  LOG(kVerbose) << "index : " << index;
  TLOG(kGreen) << "test completed in " << ticking_clock.elapsed() << " seconds\n";
  CheckEndpointStringsTestResult(std::vector<std::string>(),
                                 "SELECT * from EndpointStringsParallelDelete");
}

void Sqlite3WrapperBenchmark::CheckEndpointStringsTestResult(
    const std::vector<std::string>& expected_result, std::string query,
    bool check_order, bool check_content, bool check_size) {
  std::vector<std::string> result;
  ReadEndpointStrings(result, query);

  if ((check_size) && (result.size() != expected_result.size())) {
    TLOG(kRed) << "inserted " << expected_result.size()
               << " endpoint strings, got " << result.size() << " in database\n";
  }

  if (check_content) {
    if (check_order) {
      for (size_t i(0); i < std::min(result.size(), expected_result.size()); ++i)
        if (result.at(i) != ten_thousand_strings.at(i)) {
          TLOG(kRed) << "entry stored with dis-order\n";
          break;
        }
    } else {
      for (auto& entry : expected_result) {
        if (std::find(result.begin(), result.end(), entry) == result.end()) {
          TLOG(kRed) << "cannot find " << entry << " in database\n";
          break;
        }
      }
      for (auto& entry : result) {
        if (std::find(expected_result.begin(), expected_result.end(), entry) ==
            expected_result.end()) {
          TLOG(kRed) << "database has an entry " << entry << " not expected\n";
          break;
        }
      }
    }
  }
}

void Sqlite3WrapperBenchmark::PrepareTable(sqlite::Database& database,
                                           std::string query) {
  sqlite::Tranasction transaction{database};
  sqlite::Statement statement{database, query};
  statement.Step();
  statement.Reset();
  transaction.Commit();
}

// Insert or Delete
void Sqlite3WrapperBenchmark::UpdateEndpointStrings(sqlite::Database& database,
    const std::vector<std::string>& endpoint_strings, std::string query) {
  sqlite::Statement statement{database, query};
  for (const auto& endpoint_string : endpoint_strings) {
    statement.BindText(1, endpoint_string);
    statement.Step();
    statement.Reset();
  }
}

void Sqlite3WrapperBenchmark::ReadEndpointStrings(std::vector<std::string>& result,
                                                  std::string query) {
  sqlite::Database database{database_path, sqlite::Mode::kReadOnly};
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
