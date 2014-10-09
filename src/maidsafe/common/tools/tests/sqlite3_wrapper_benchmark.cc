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
  EndpointStringsConcurrentInsertions();
  EndpointStringsConcurrentDeletes();

  // PmidManager and MaidManager use GroupDb which have key length to be around 130
  //                             and value length to be around 20
  // Datamanager use Db which have key length to be around 64, but the value is vector
  //             of IDs, miniumn to be 4, makes the value length to be at least 256
  for (int i(0); i < 10000; ++i)
    key_value_pairs[RandomAlphaNumericString(130)] = RandomAlphaNumericString(512);

  KeyValueIndividualTransaction();
  // When SQLite bing used for personas, each persona can has its own table or even database.
  // So the concurrent situation depending on the program configuration only,
  // the chance of high number of concurrency is low, so only tested with 4 threads.
  KeyValueConcurrentInsertions();
  KeyValueConcurrentUpdates();
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
    AddRemoveEndpointStrings(
        database, ten_thousand_strings,
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
      AddRemoveEndpointStrings(
          database, endpoint_string_vector,
          "INSERT OR REPLACE INTO EndpointStringsIndividualTransaction (ENDPOINT) VALUES (?)");
      transaction.Commit();
    }
  }
  TLOG(kGreen) << "test completed in " << ticking_clock.elapsed() << " seconds\n";
  CheckEndpointStringsTestResult(ten_thousand_strings,
                                 "SELECT * from EndpointStringsIndividualTransaction");
}

void Sqlite3WrapperBenchmark::EndpointStringsConcurrentInsertions() {
  TLOG(kGreen) << "\nInserting 10k endpoint strings with 20 concurrent threads,"
               << " and individual transaction for each string\n";

  ticking_clock.restart();
  sqlite::Database database(database_path, sqlite::Mode::kReadWriteCreate);
  std::string query(
      "CREATE TABLE IF NOT EXISTS EndpointStringsConcurrentInsertions ("
      "ENDPOINT TEXT  PRIMARY KEY NOT NULL);");
  PrepareTable(database, query);

  std::mutex mutex;
  size_t thread_count(20), index(0);
  maidsafe::test::RunInParallel(static_cast<int>(thread_count - 1), [&] {
    for (size_t i(0); i < (ten_thousand_strings.size() / thread_count); ++i) {
      std::vector<std::string> endpoint_string_vector;
      {
        std::lock_guard<std::mutex> lock{mutex};
        endpoint_string_vector.push_back(ten_thousand_strings.at(index));
        ++index;
      }
      sqlite::Tranasction transaction{database};
      AddRemoveEndpointStrings(
          database, endpoint_string_vector,
          "INSERT OR REPLACE INTO EndpointStringsConcurrentInsertions (ENDPOINT) VALUES (?)");
      transaction.Commit();
    }
  });
  LOG(kVerbose) << "index : " << index;
  TLOG(kGreen) << "test completed in " << ticking_clock.elapsed() << " seconds\n";
  CheckEndpointStringsTestResult(ten_thousand_strings,
                                 "SELECT * from EndpointStringsConcurrentInsertions", false);
}

void Sqlite3WrapperBenchmark::EndpointStringsConcurrentDeletes() {
  TLOG(kGreen) << "\nConcurrent Deletion (20 threads) from the database"
               << " containing 10k endpoint strings\n";
  sqlite::Database database(database_path, sqlite::Mode::kReadWriteCreate);
  std::string query(
      "CREATE TABLE IF NOT EXISTS EndpointStringsConcurrentDeletes ("
      "ENDPOINT TEXT  PRIMARY KEY NOT NULL);");
  PrepareTable(database, query);
  {
    // populate the database with 10k entries
    sqlite::Tranasction transaction{database};
    AddRemoveEndpointStrings(
        database, ten_thousand_strings,
        "INSERT OR REPLACE INTO EndpointStringsConcurrentDeletes (ENDPOINT) VALUES (?)");
    transaction.Commit();
  }

  ticking_clock.restart();
  std::mutex mutex;
  size_t thread_count(20), index(0);
  maidsafe::test::RunInParallel(static_cast<int>(thread_count - 1), [&] {
    for (size_t i(0); i < (ten_thousand_strings.size() / thread_count); ++i) {
      std::vector<std::string> endpoint_string_vector;
      {
        std::lock_guard<std::mutex> lock{mutex};
        LOG(kVerbose) << index;
        endpoint_string_vector.push_back(ten_thousand_strings.at(index));
        ++index;
      }
      sqlite::Tranasction transaction{database};
      AddRemoveEndpointStrings(database, endpoint_string_vector,
                               "DELETE From EndpointStringsConcurrentDeletes WHERE ENDPOINT=?");
      transaction.Commit();
    }
  });
  LOG(kVerbose) << "index : " << index;
  TLOG(kGreen) << "test completed in " << ticking_clock.elapsed() << " seconds\n";
  CheckEndpointStringsTestResult(std::vector<std::string>(),
                                 "SELECT * from EndpointStringsConcurrentDeletes");
}

void Sqlite3WrapperBenchmark::CheckEndpointStringsTestResult(
    const std::vector<std::string>& expected_result, std::string query, bool check_order,
    bool check_content, bool check_size) {
  std::vector<std::string> result;
  ReadEndpointStrings(result, query);

  if ((check_size) && (result.size() != expected_result.size()))
    TLOG(kRed) << "inserted " << expected_result.size() << " endpoint strings, got "
               << result.size() << " in database\n";

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

void Sqlite3WrapperBenchmark::PrepareTable(sqlite::Database& database, std::string query) {
  sqlite::Tranasction transaction{database};
  sqlite::Statement statement{database, query};
  statement.Step();
  statement.Reset();
  transaction.Commit();
}

// Insert or Delete
void Sqlite3WrapperBenchmark::AddRemoveEndpointStrings(
    sqlite::Database& database, const std::vector<std::string>& endpoint_strings,
    std::string query) {
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
  for (;;)
    if (statement.Step() == sqlite::StepResult::kSqliteRow)
      result.push_back(statement.ColumnText(0));
    else
      break;
}

void Sqlite3WrapperBenchmark::KeyValueIndividualTransaction() {
  TLOG(kGreen) << "\nInserting 10k key_value_pairs, individual transaction for each\n";
  {
    ticking_clock.restart();
    sqlite::Database database(database_path, sqlite::Mode::kReadWriteCreate);
    std::string query(
        "CREATE TABLE IF NOT EXISTS KeyValueIndividualTransaction ("
        "KEY TEXT  PRIMARY KEY NOT NULL, VALUE TEXT NOT NULL);");
    PrepareTable(database, query);
    for (const auto& key_value_pair : key_value_pairs) {
      sqlite::Tranasction transaction{database};
      InsertKeyValuePair(
          database, key_value_pair,
          "INSERT OR REPLACE INTO KeyValueIndividualTransaction (KEY, VALUE) VALUES (?, ?)");
      transaction.Commit();
    }
  }
  TLOG(kGreen) << "test completed in " << ticking_clock.elapsed() << " seconds\n";
  CheckKeyValueTestResult(key_value_pairs, "SELECT * from KeyValueIndividualTransaction");
}

void Sqlite3WrapperBenchmark::KeyValueConcurrentInsertions() {
  TLOG(kGreen) << "\nInserting 10k key_value pairs with 4 concurrent threads,"
               << " and individual transaction for each string\n";

  ticking_clock.restart();
  sqlite::Database database(database_path, sqlite::Mode::kReadWriteCreate);
  std::string query(
      "CREATE TABLE IF NOT EXISTS KeyValueConcurrentInsertions ("
      "KEY TEXT  PRIMARY KEY NOT NULL, VALUE TEXT NOT NULL);");
  PrepareTable(database, query);

  std::mutex mutex;
  size_t thread_count(4), index(0);
  maidsafe::test::RunInParallel(static_cast<int>(thread_count - 1), [&] {
    for (size_t i(0); i < (key_value_pairs.size() / thread_count); ++i) {
      auto itr(key_value_pairs.begin());
      {
        std::lock_guard<std::mutex> lock{mutex};
        LOG(kVerbose) << index;
        std::advance(itr, index);
        ++index;
      }
      sqlite::Tranasction transaction{database};
      InsertKeyValuePair(
          database, *itr,
          "INSERT OR REPLACE INTO KeyValueConcurrentInsertions (KEY, VALUE) VALUES (?, ?)");
      transaction.Commit();
    }
  });
  LOG(kVerbose) << "index : " << index;
  TLOG(kGreen) << "test completed in " << ticking_clock.elapsed() << " seconds\n";
  CheckKeyValueTestResult(key_value_pairs, "SELECT * from KeyValueConcurrentInsertions");
}

void Sqlite3WrapperBenchmark::KeyValueConcurrentUpdates() {
  TLOG(kGreen) << "\nUpdating 10k times with 4 concurrent threads,"
               << " inside a database containing 10k key_vaule pairs\n";

  sqlite::Database database(database_path, sqlite::Mode::kReadWriteCreate);
  std::string query(
      "CREATE TABLE IF NOT EXISTS KeyValueConcurrentUpdates ("
      "KEY TEXT  PRIMARY KEY NOT NULL, VALUE TEXT NOT NULL);");
  PrepareTable(database, query);
  for (const auto& key_value_pair : key_value_pairs) {
    sqlite::Tranasction transaction{database};
    InsertKeyValuePair(
        database, key_value_pair,
        "INSERT OR REPLACE INTO KeyValueConcurrentUpdates (KEY, VALUE) VALUES (?, ?)");
    transaction.Commit();
  }

  ticking_clock.restart();
  std::mutex mutex;
  size_t thread_count(4), index(0);
  maidsafe::test::RunInParallel(static_cast<int>(thread_count - 1), [&] {
    for (size_t i(0); i < (key_value_pairs.size() / thread_count); ++i) {
      auto itr(key_value_pairs.begin());
      {
        std::lock_guard<std::mutex> lock{mutex};
        LOG(kVerbose) << index;
        std::advance(itr, RandomInt32() % key_value_pairs.size());
        key_value_pairs[itr->first] = RandomAlphaNumericString(512);
        ++index;
      }
      sqlite::Tranasction transaction{database};
      UpdateKeyValuePair(database, *itr,
                         "UPDATE KeyValueConcurrentUpdates SET VALUE=? WHERE KEY=?");
      transaction.Commit();
    }
  });
  LOG(kVerbose) << "index : " << index;
  TLOG(kGreen) << "test completed in " << ticking_clock.elapsed() << " seconds\n";
  CheckKeyValueTestResult(key_value_pairs, "SELECT * from KeyValueConcurrentUpdates");
}

void Sqlite3WrapperBenchmark::InsertKeyValuePair(sqlite::Database& database,
                                                 std::pair<std::string, std::string> key_value_pair,
                                                 std::string query) {
  sqlite::Statement statement{database, query};
  statement.BindText(1, key_value_pair.first);
  statement.BindText(2, key_value_pair.second);
  statement.Step();
  statement.Reset();
}

void Sqlite3WrapperBenchmark::ReadKeyValuePairs(std::map<std::string, std::string>& result,
                                                std::string query) {
  sqlite::Database database{database_path, sqlite::Mode::kReadOnly};
  sqlite::Statement statement{database, query};
  for (;;)
    if (statement.Step() == sqlite::StepResult::kSqliteRow)
      result[statement.ColumnText(0)] = statement.ColumnText(1);
    else
      break;
}

void Sqlite3WrapperBenchmark::UpdateKeyValuePair(sqlite::Database& database,
                                                 std::pair<std::string, std::string> key_value_pair,
                                                 std::string query) {
  sqlite::Statement statement{database, query};
  statement.BindText(1, key_value_pair.second);  // set column VALUE to value
  statement.BindText(2, key_value_pair.first);   // set WHERE KEY = to key
  statement.Step();
  statement.Reset();
}

void Sqlite3WrapperBenchmark::CheckKeyValueTestResult(
    const std::map<std::string, std::string>& expected_result, std::string query) {
  std::map<std::string, std::string> result;
  ReadKeyValuePairs(result, query);

  if (result.size() != expected_result.size())
    TLOG(kRed) << "inserted " << expected_result.size() << " key_value pairs, got " << result.size()
               << " in database\n";

  for (auto& entry : expected_result) {
    auto itr(result.find(entry.first));
    if (itr == result.end()) {
      TLOG(kRed) << "cannot find " << HexSubstr(entry.first) << " in database\n";
      break;
    } else if (itr->second != entry.second) {
      TLOG(kRed) << "value of " << HexSubstr(entry.first) << " expected to be "
                 << HexSubstr(entry.second) << " in database, but turned out to be "
                 << HexSubstr(itr->second) << "\n";
      break;
    }
  }
  for (auto& entry : result) {
    if (expected_result.find(entry.first) == expected_result.end()) {
      TLOG(kRed) << "database has an entry " << HexSubstr(entry.first) << " not expected\n";
      break;
    }
  }
}

}  // namespace benchmark

}  // namespace maidsafe
