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

#include "maidsafe/common/sqlite3_wrapper.h"

extern "C" {
#include <sqlite3.h>
}

#include <limits>
#include <string>

#include "maidsafe/common/log.h"
#include "maidsafe/common/utils.h"

namespace maidsafe {

namespace sqlite {

Database::Database(const boost::filesystem::path& filename, Mode mode) : database(nullptr) {
  auto flags = static_cast<int>(mode);
  if (sqlite3_open_v2(filename.string().c_str(), &database, flags, NULL) != SQLITE_OK) {
    LOG(kError) << "Could not open DB at: " << filename << ".  Error: " << sqlite3_errmsg(database);
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::db_not_present));
  }
  assert(sqlite3_threadsafe());
  char* error_message = 0;
  sqlite3_exec(database, "PRAGMA synchronous = OFF", NULL, 0, &error_message);
  sqlite3_exec(database, "PRAGMA journal_mode = WAL", NULL, 0, &error_message);
  sqlite3_exec(database, "PRAGMA wal_autocheckpoint = 0", NULL, 0, &error_message);
  sqlite3_busy_timeout(database, 250);
}

Database::~Database() {
  int result = sqlite3_close(database);
  if (result != SQLITE_OK)
    LOG(kError) << "Failed to close DB.  Error: " << result << " - " << sqlite3_errmsg(database);
}

void Database::CheckPoint() {
  if (sqlite3_wal_checkpoint(database, NULL) != SQLITE_OK)
    LOG(kError) << "CheckPoint error: " << sqlite3_errmsg(database);
  //    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::db_error));
}


Transaction::Transaction(Database& database_in) : kAttempts(200), database(database_in) {
  std::string query("BEGIN IMMEDIATE TRANSACTION");  // immediate or exclusive transaction
  for (int i(0); i != kAttempts; ++i) {
    try {
      Execute(query);
      return;
    } catch (const maidsafe_error& error) {
      LOG(kWarning) << "Transaction constructor failed in attempt " << i << " with error "
                    << boost::diagnostic_information(error);
      if (error.code() == make_error_code(CommonErrors::db_not_present))
        throw;
      else
        std::this_thread::sleep_for(
            std::chrono::milliseconds(RandomUint32() % 200 + RandomUint32() % ((i + 1) * 10) + 10));
    }
  }
  LOG(kError) << "Failed to acquire DB lock in " << kAttempts << " attempts";
  BOOST_THROW_EXCEPTION(MakeError(CommonErrors::unable_to_handle_request));
}

Transaction::~Transaction() {
  if (committed)
    return;
  try {
    Execute("ROLLBACK TRANSACTION");
  } catch (const std::exception& error) {
    LOG(kError) << "Error on ROLLBACK TRANSACTION: " << error.what();
  }
}

void Transaction::Commit() {
  for (int i(0); i != kAttempts; ++i) {
    try {
      Execute("COMMIT TRANSACTION");
      committed = true;
      return;
    } catch (const std::exception& e) {
      LOG(kWarning) << "Transaction::Commit failed in attempt " << i << " with error "
                    << boost::diagnostic_information(e);
      std::this_thread::sleep_for(
          std::chrono::milliseconds(RandomUint32() % 200 + RandomUint32() % ((i + 1) * 10) + 10));
    }
  }
  LOG(kError) << "Failed to acquire DB lock in " << kAttempts << " attempts";
  BOOST_THROW_EXCEPTION(MakeError(CommonErrors::unable_to_handle_request));
}

void Transaction::Execute(const std::string& query) {
  char* error_message = 0;
  int result = sqlite3_exec(database.database, query.c_str(), NULL, 0, &error_message);
  assert(result != SQLITE_ROW);

  if (result != SQLITE_OK) {
    if (result == SQLITE_BUSY) {
      LOG(kWarning) << "DB busy: " << error_message << ".  Query: " << query;
      sqlite3_free(error_message);
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::db_busy));
    } else if (result == SQLITE_NOTADB) {
      LOG(kError) << "DB not present";
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::db_not_present));
    } else {
      LOG(kError) << "SQL error: " << error_message << ".  Return value: " << result
                  << ".  Query: " << query;
      sqlite3_free(error_message);
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::db_error));
    }
  }
}

Statement::Statement(Database& database_in, const std::string& query)
    : database(database_in), statement() {
  assert(query.size() < std::numeric_limits<int>::max() - 1);
  auto return_value = sqlite3_prepare_v2(database.database, query.c_str(),
                                         static_cast<int>(query.size() + 1), &statement, 0);
  if (return_value != SQLITE_OK) {
    LOG(kError) << "sqlite3_prepare_v2 returned: " << return_value << " - "
                << sqlite3_errmsg(database.database);
    if (return_value == SQLITE_NOTADB)
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::db_not_present));
    else
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::db_error));
  }
}

Statement::~Statement() {
  auto return_value = sqlite3_finalize(statement);
  if (return_value != SQLITE_OK) {
    LOG(kError) << "sqlite3_finalize returned: " << return_value << " - "
                << sqlite3_errmsg(database.database);
  }
}

void Statement::BindText(int row_index, const std::string& text) {
  assert(text.size() < std::numeric_limits<int>::max());
  auto return_value = sqlite3_bind_text(statement, row_index, text.c_str(),
                                        static_cast<int>(text.size()), SQLITE_TRANSIENT);
  if (return_value != SQLITE_OK) {
    LOG(kError) << "sqlite3_bind_text returned: " << return_value << " - "
                << sqlite3_errmsg(database.database);
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::db_error));
  }
}

void Statement::BindBlob(int row_index, const SerialisedData& blob) {
  assert(blob.size() < std::numeric_limits<int>::max());
  auto return_value = sqlite3_bind_blob(statement, row_index, &blob[0],
                                        static_cast<int>(blob.size()), SQLITE_TRANSIENT);
  if (return_value != SQLITE_OK) {
    LOG(kError) << "sqlite3_bind_blob returned: " << return_value << " - "
                << sqlite3_errmsg(database.database);
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::db_error));
  }
}

StepResult Statement::Step() {
  auto return_value = sqlite3_step(statement);
  if ((return_value == SQLITE_DONE) || (return_value == SQLITE_ROW)) {
    return StepResult(return_value);
  } else {
    LOG(kError) << "sqlite3_step returned: " << return_value << " - "
                << sqlite3_errmsg(database.database);
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::db_error));
  }
}

std::string Statement::ColumnText(int col_index) {
  auto column_text = reinterpret_cast<const char*>(sqlite3_column_text(statement, col_index));
  int bytes = sqlite3_column_bytes(statement, col_index);
  return std::string(column_text, bytes);
}

SerialisedData Statement::ColumnBlob(int col_index) {
  auto column_blob =
      reinterpret_cast<const unsigned char*>(sqlite3_column_blob(statement, col_index));
  int bytes = sqlite3_column_bytes(statement, col_index);
  return SerialisedData(column_blob, column_blob + bytes);
}

void Statement::Reset() {
  auto return_value = sqlite3_reset(statement);
  if (return_value != SQLITE_OK) {
    LOG(kError) << "sqlite3_reset returned: " << return_value << " - "
                << sqlite3_errmsg(database.database);
    BOOST_THROW_EXCEPTION(MakeError(CommonErrors::db_error));
  }
}


}  // namespace sqlite

}  // namespace maidsafe
