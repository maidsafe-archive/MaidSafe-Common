#Account Transfer

##Introduction

SLite wrapper provides a SQLite database access API to upper layer MaidSafe projects.

##Naming Conventions



##Motivation

The motivation of this design is to provide a conven
ient SQLite access API to upper layer MaidSafe projects, whilst encapsulates as much of database handling code as possible.
It is aimed to make upper layer's database function call to be concise and neat.
An efficient mechanism of database access error handling is also one of the main concerns to be resolved by the wrapper.
SQLite was originally introduced to allow multiple vaults access the same bootstrap file. However it may have the potential to replace the current LLDB to be used by vault to store temporary persona account information.

##Overview

Access to SQLite database can be divided into three steps : database connection, transaction protection, statement execution .
1,  database connection
This is a mandatory but simple stage, the application needs to open a connection to the database to register its usage. Once the task is done, the connection needs to be closed properly to avoid any potential database corruption.

2, transaction protection
This step is optional when using SQLite database in a single thread environment. When used in multi-threading scenario, transaction provides critical protection against any potential racing and database corruption.
A transaction needs to be 'BEGIN' first to register a start in database. Once all actions completed, it needs to be 'COMMIT' as a finish signal to the database. In case any problem happened during the transaction protection span, a 'ROLLBACK' of the database to the previous savepoint shall be carried out.

3,  statement execution
This is the step carry out the real query (read or write) to the database. When guarded by transaction, there is no database access related error shall be thrown (syntax or query error is another story).
It is suggested to use sqlite3_prepare_v2 to prepare the statement with main query text, then use sqlite3_bind_text to assign value to any required variable. sqlite3_step will execute the statement by one step and sqlite3_column_text (and many other sqlite read access methods) can fetch the content from that step. Once query is completed, it is suggested to use sqlite3_reset and sqlite3_finalize to register the completion of the execution of current statement with the database.


The mechanism of error handling can be described as two main parts : fall-back and roll-back. The 'fall-back' mechanism means where there is database access error happens, the accessing thread wait for a short period then carry out a re-attempt of the failing action. The 'roll-back'  mechanism means the accessing thread roll-back to the last save-point and carry out all actions (cached) between failing-point and save-point.
The usage of SQLite for MaidSafe allows the sqlite_wrapper only need to care about the 'fall-back'. The missing entries in case of error and a database roll-back is acceptable. When transaction protection is being used, such re-attempt only need to be carried out inside transaction part.
##Assumptions

1. database is mainly used as a key-value storage or for bootstrap file (endpoint info)
2. no need to provide strong SQL type API
3. statements are always guarded by transaction
4. data lost in case of database roll-back is acceptable

##Implementation

The implementation of the sqlite_wrapper is presented in pseudo code below. 

// Modes for file open operations
enum class Mode {
  kReadOnly = 0x00000001,  // SQLITE_OPEN_READONLY
  kReadWrite = 0x00000002,  // SQLITE_OPEN_READWRITE
  kReadWriteCreate = 0x00000002 | 0x00000004  // SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE
};
maidsafe::sqlite::Database { 
 public :
  Database(boost::filesystem::path& file_name, Mode access_mode);  // startup a connection to the database
  ~Database();  // close the connection to the database
 private :
  sqlite3 *database;  // pointer to the sqlite3 database connection
}


maidsafe::sqlite::Transaction { 
 public :
  Transaction(Database& database);  // BEGIN a transaction
  Commit();  // COMMIT a transaction
  ~Transaction();  // roll-back to last save-point if previous Commit() failed
}
It needs to be pointed out that re-attempts will be carried out in all functions (include the constructor and destructor) of Transaction.


enum class StepResult {
  kSqliteRow = 100,  // SQLITE_ROW
  kSqliteDone = 101  // SQLITE_DONE
};
Maidsafe::sqlite::Statement {
 public :
  Statement(Database& database_in, const std::string& query);  // prepare a statement
  BindText(int index, const std::string& text);  // assign value to a variable
  StepResult Step();  // execute one step
  std::string ColumnText(int col_index); // read a column's content of current row
  Reset();  // register resetting of statement with the database
  ~Statement();  // register finalizing of statement with the database
 private :
  sqlite3_stmt* statement;  // pointer to the sqlite3  statement
}

