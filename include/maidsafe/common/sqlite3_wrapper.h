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

#ifndef MAIDSAFE_SQLITE3_WRAPPER_H_
#define MAIDSAFE_SQLITE3_WRAPPER_H_

#include <string>
#include <memory>

#include "boost/filesystem/path.hpp"

struct sqlite3;
struct sqlite3_stmt;

namespace maidsafe {

namespace sqlite {

struct Statement;

// Modes for file open operations
enum class Mode {
  kReadOnly = 0x00000001,  // SQLITE_OPEN_READONLY
  kReadWrite = 0x00000002,  // SQLITE_OPEN_READWRITE
  kReadWriteCreate = 0x00000002 | 0x00000004  // SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE
};

struct Database {
  Database(const boost::filesystem::path& filename, Mode mode);
  ~Database();
  Database(const Database&) = delete;
  Database(Database&&) = delete;
  Database& operator=(Database) = delete;


  void Execute(const std::string& query);
  friend class Statement;

 private:
  sqlite3 *database;
};

struct Tranasction {
  Tranasction(Database& database_in);
  ~Tranasction();
  Tranasction(const Tranasction&) = delete;
  Tranasction(Tranasction&&) = delete;
  Tranasction& operator=(Tranasction) = delete;

  void Commit();

 private:

  const int kAttempts;
  bool committed;
  Database& database;
};


enum class StepResult {
  kSqliteRow = 100,  // SQLITE_ROW
  kSqliteDone = 101  // SQLITE_DONE
};

struct Statement {

  Statement(Database& database_in, const std::string& query);
  ~Statement();
  Statement(const Statement&) = delete;
  Statement(Statement&&) = delete;
  Statement& operator=(Statement) = delete;

  void BindText(int index, const std::string& text);
  StepResult Step();
  void Reset();

  std::string ColumnText(int col_index);

 private :
  Database& database;
  sqlite3_stmt* statement;
};

}  // namespace sqlite

}  // namespace maidsafe

#endif  // MAIDSAFE_SQLITE3_WRAPPER_H_
