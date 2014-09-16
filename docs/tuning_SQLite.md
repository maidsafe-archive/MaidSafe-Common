#Tuning SQLite

##Introduction

SQLite provides a set of PRAGMA options and query keywords allowing user to define different database behaviour to fit various usage patterns.
By just adjusting
 the database behaviour, huge performance improvement (sometime hundreds of times) can be easily achieved.

##Naming Conventions



##Motivation

To improve the performance of SQLite for our usage and also to be stable across various OS platform.

##Overview

The SQLite using default value delivered a disappointing performance when being tested against bootstrap functional test, especially the parallel tests.
For one of the parallel test, involves 20 concurrent threads each one have 20 separate writes to the database, it takes around 70 seconds to be completed in Linux, 3.5 seconds in OSX, and worst of all, Windows can't handle the test (raising system exception).
Given it is only max 20 concurrency and totally 400 writes, the poor performance and unstable status across different OS is not acceptable.
With tons of experimental, the following tuning methods proved to be able to improve the performance dramatically : use WAL (Write Ahead Log) journal_mode (instead of the default Roll-Back mode), disable checkpoint for WAL, turn off synchronous, use IMMEDIATE lock for transaction instead of EXCLUSIVE.
The result of performance is around 500 – 700 ms to complete the same parallel test across all OS platforms (yes, Windows can handle it properly). Considering this is in Debug build, it is acceptable.
It needs to be mentioned that using WAL, especially together with turn off synchronous and checkpoint, leaves the database vulnerable to any power failure or system crash. It turns out to be no harm to MaidSafe usage, as SQLite is only used to store temporary data and there is no recover from database when restarting vault.

##Assumptions

1. write / read operations happen in a chance of 50:50 rate
2. in case of restarting, recovery from database is not critical
3. all processes accessing the database sit on the same machine (no network filesystem)
4. the database cannot be read-only
5. transactions are small (less than 100 MB, cannot in excess of 1 GB)

##Implementation

1, sqlite3_exec(database, "PRAGMA journal_mode = WAL", NULL, 0, &error_message);
This turns on SQLite's WAL mode. The performance already seen a huge improvement in OSX, from 3.5 seconds to 800 ms. In Linux, it is from 70 seconds to 25 seconds. In Windows, the test can be completed in majority case (failing rate is around 2%) within 1 seconds.

2, sqlite3_exec(database, "PRAGMA synchronous = OFF", NULL, 0, &error_message);
This turns off synchronous for SQLite. The performance in Linux see a further dramatic improvement, from the above 25 seconds down to around 800 ms.

3, sqlite3_exec(database, "PRAGMA wal_autocheckpoint = 0", NULL, 0, &error_message);
This turns off auto checkpoint for WAL. Although it doesn't affect the performance much, it eliminates the failing when running test in Windows.

4, std::string query("BEGIN IMMEDIATE TRANSACTION");
This starts transaction using IMMEDIATE lock. Although only 5% performance improvement is observed during the test, which has a ratio of write : read to be 20 : 1 , it is believed this will help improve performance in real usage which has a higher rate of read opertions.

