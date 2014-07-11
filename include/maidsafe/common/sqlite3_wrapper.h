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

namespace maidsafe {

namespace sqlite3 {

struct Statement;

struct Database {
  Database(const boost::filesystem::path& filename, int flags);
  ~Database();

  void Execute(const std::string& query);
  friend class Sqlite3Statement;

 private:
  sqlite3 *database;  // FIXME  consider unique_ptr , delete constructors
};


// Tranasction
struct Tranasction {
  Tranasction(Database& database_in);
  ~Tranasction();
  void Commit();

 private:
  Tranasction(const Tranasction&);
  Tranasction(Tranasction&&);
  Tranasction& operator=(Tranasction);

  const int kAttempts;
  bool committed;
  Database& database;
};


enum class StepResult {
  kSqliteRow = SQLITE_ROW,
  kSqliteDone = SQLITE_DONE
};

struct Statement {

  Statement(Database& database_in, const std::string& query);
  ~Statement();
  void BindText(int index, const std::string& text);
  StepResult Step();
  void Reset();

  std::string ColumnText(int col_index);

 private :
  Database& database;
  sqlite3_stmt* statement;  // FIXME  consider unique_ptr
};


}  // namespace sqlite3

}  // namespace maidsafe

#endif  // MAIDSAFE_SQLITE3_WRAPPER_H_
