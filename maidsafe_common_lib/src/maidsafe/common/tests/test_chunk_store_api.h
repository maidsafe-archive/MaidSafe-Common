/* Copyright (c) 2011 maidsafe.net limited
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    * Neither the name of the maidsafe.net limited nor the names of its
    contributors may be used to endorse or promote products derived from this
    software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAIDSAFE_COMMON_TESTS_TEST_CHUNK_STORE_API_H_
#define MAIDSAFE_COMMON_TESTS_TEST_CHUNK_STORE_API_H_

#include <memory>
#include <string>
#include "gtest/gtest.h"
#include "boost/filesystem.hpp"
#include "boost/filesystem/fstream.hpp"
#include "maidsafe/common/chunk_store.h"
#include "maidsafe/common/crypto.h"
#include "maidsafe/common/utils.h"

namespace fs = boost::filesystem;

namespace maidsafe {

namespace test {

template <typename T>
class ChunkStoreTest: public testing::Test {
 public:
  ChunkStoreTest()
      : test_dir_(fs::unique_path(fs::temp_directory_path() /
                  "MaidSafe_TestChunkStore_%%%%-%%%%-%%%%")),
        chunk_dir_(test_dir_ / "chunks"),
        alt_chunk_dir_(test_dir_ / "chunks2"),
        chunk_store_(new T),
        alt_chunk_store_(new T) {}
  ~ChunkStoreTest() {}
 protected:
  void SetUp() {
    if (fs::exists(test_dir_))
      fs::remove_all(test_dir_);
    fs::create_directories(test_dir_);
    fs::create_directories(chunk_dir_);
    InitChunkStore(chunk_store_, chunk_dir_);
    InitChunkStore(alt_chunk_store_, alt_chunk_dir_);
  }
  void TearDown() {
    try {
      if (fs::exists(test_dir_))
        fs::remove_all(test_dir_);
    }
    catch(...) {}
  }
  void InitChunkStore(std::shared_ptr<ChunkStore> chunk_store,
                      const fs::path &chunk_dir);
  fs::path CreateRandomFile(const fs::path &file_path,
                            const std::uint64_t &file_size) {
    fs::ofstream ofs(file_path, std::ios::binary | std::ios::out |
                                std::ios::trunc);
    if (file_size != 0) {
      size_t string_size = (file_size > 100000) ? 100000 :
                          static_cast<size_t>(file_size);
      std::uint64_t remaining_size = file_size;
      std::string rand_str = RandomString(2 * string_size);
      std::string file_content;
      std::uint64_t start_pos = 0;
      while (remaining_size) {
        srand(17);
        start_pos = rand() % string_size;  // NOLINT (Fraser)
        if (remaining_size < string_size) {
          string_size = static_cast<size_t>(remaining_size);
          file_content = rand_str.substr(0, string_size);
        } else {
          file_content = rand_str.substr(static_cast<size_t>(start_pos),
                                        string_size);
        }
        ofs.write(file_content.c_str(), file_content.size());
        remaining_size -= string_size;
      }
    }
    ofs.close();
    return file_path;
  }
  fs::path test_dir_, chunk_dir_, alt_chunk_dir_;
  std::shared_ptr<ChunkStore> chunk_store_, alt_chunk_store_;
};

TYPED_TEST_CASE_P(ChunkStoreTest);

TYPED_TEST_P(ChunkStoreTest, BEH_CS_Init) {
  EXPECT_EQ(0, this->chunk_store_->Size());
  EXPECT_EQ(0, this->chunk_store_->Capacity());
  EXPECT_EQ(0, this->chunk_store_->Count());
  EXPECT_TRUE(this->chunk_store_->Empty());
}

TYPED_TEST_P(ChunkStoreTest, BEH_CS_Get) {
  std::string content(RandomString(100));
  std::string name(crypto::Hash<crypto::SHA512>(content));
  fs::path path(this->test_dir_ / "chunk.dat");
  ASSERT_FALSE(fs::exists(path));

  // non-existant chunk, should fail
  EXPECT_TRUE(this->chunk_store_->Get("").empty());
  EXPECT_TRUE(this->chunk_store_->Get(name).empty());
  EXPECT_FALSE(this->chunk_store_->Get(name, path));
  EXPECT_FALSE(fs::exists(path));

  ASSERT_TRUE(this->chunk_store_->Store(name, content));

  // existing chunk
  EXPECT_EQ(content, this->chunk_store_->Get(name));
  EXPECT_TRUE(this->chunk_store_->Get(name, path));
  EXPECT_TRUE(fs::exists(path));
  EXPECT_EQ(name, crypto::HashFile<crypto::SHA512>(path));

  // existing output file, should overwrite
  this->CreateRandomFile(path, 99);
  EXPECT_NE(name, crypto::HashFile<crypto::SHA512>(path));
  EXPECT_TRUE(this->chunk_store_->Get(name, path));
  EXPECT_EQ(name, crypto::HashFile<crypto::SHA512>(path));

  // invalid file name
  EXPECT_FALSE(this->chunk_store_->Get(name, ""));
}

TYPED_TEST_P(ChunkStoreTest, BEH_CS_Store) {
  std::string content(RandomString(123));
  std::string name_mem(crypto::Hash<crypto::SHA512>(content));
  fs::path path(this->test_dir_ / "chunk.dat");
  this->CreateRandomFile(path, 456);
  std::string name_file(crypto::HashFile<crypto::SHA512>(path));
  ASSERT_NE(name_mem, name_file);

  // invalid input
  EXPECT_FALSE(this->chunk_store_->Store(name_mem, ""));
  EXPECT_FALSE(this->chunk_store_->Store("", content));
  EXPECT_FALSE(this->chunk_store_->Store(name_file, "", false));
  EXPECT_FALSE(this->chunk_store_->Store(name_file, this->test_dir_ / "fail",
                                         false));
  EXPECT_FALSE(this->chunk_store_->Store("", path, false));
  EXPECT_TRUE(this->chunk_store_->Empty());
  EXPECT_EQ(0, this->chunk_store_->Count());
  EXPECT_EQ(0, this->chunk_store_->Size());
  EXPECT_FALSE(this->chunk_store_->Has(name_mem));
  EXPECT_EQ(0, this->chunk_store_->Size(name_mem));
  EXPECT_FALSE(this->chunk_store_->Has(name_file));
  EXPECT_EQ(0, this->chunk_store_->Size(name_file));

  // store from string
  EXPECT_TRUE(this->chunk_store_->Store(name_mem, content));
  EXPECT_FALSE(this->chunk_store_->Empty());
  EXPECT_EQ(1, this->chunk_store_->Count());
  EXPECT_EQ(123, this->chunk_store_->Size());
  EXPECT_TRUE(this->chunk_store_->Has(name_mem));
  EXPECT_EQ(123, this->chunk_store_->Size(name_mem));
  EXPECT_FALSE(this->chunk_store_->Has(name_file));
  EXPECT_EQ(0, this->chunk_store_->Size(name_file));

  ASSERT_EQ(name_mem,
            crypto::Hash<crypto::SHA512>(this->chunk_store_->Get(name_mem)));

  // store from file
  EXPECT_TRUE(this->chunk_store_->Store(name_file, path, false));
  EXPECT_FALSE(this->chunk_store_->Empty());
  EXPECT_EQ(2, this->chunk_store_->Count());
  EXPECT_EQ(579, this->chunk_store_->Size());
  EXPECT_TRUE(this->chunk_store_->Has(name_mem));
  EXPECT_EQ(123, this->chunk_store_->Size(name_mem));
  EXPECT_TRUE(this->chunk_store_->Has(name_file));
  EXPECT_EQ(456, this->chunk_store_->Size(name_file));

  ASSERT_EQ(name_file,
            crypto::Hash<crypto::SHA512>(this->chunk_store_->Get(name_file)));

  fs::path new_path(this->test_dir_ / "chunk2.dat");
  this->CreateRandomFile(new_path, 333);
  std::string new_name(crypto::HashFile<crypto::SHA512>(new_path));

  // overwrite existing, should be ignored
  EXPECT_TRUE(this->chunk_store_->Store(name_mem, ""));
  EXPECT_TRUE(this->chunk_store_->Store(name_mem, RandomString(222)));
  EXPECT_TRUE(this->chunk_store_->Store(name_file, "", false));
  EXPECT_TRUE(this->chunk_store_->Store(name_file, new_path, false));
  EXPECT_FALSE(this->chunk_store_->Empty());
  EXPECT_EQ(2, this->chunk_store_->Count());
  EXPECT_EQ(579, this->chunk_store_->Size());
  EXPECT_TRUE(this->chunk_store_->Has(name_mem));
  EXPECT_EQ(123, this->chunk_store_->Size(name_mem));
  EXPECT_TRUE(this->chunk_store_->Has(name_file));
  EXPECT_EQ(456, this->chunk_store_->Size(name_file));

  ASSERT_EQ(name_mem,
            crypto::Hash<crypto::SHA512>(this->chunk_store_->Get(name_mem)));
  ASSERT_EQ(name_file,
            crypto::Hash<crypto::SHA512>(this->chunk_store_->Get(name_file)));

  // delete input file (existing chunk)
  EXPECT_FALSE(this->chunk_store_->Store("", path, true));
  EXPECT_TRUE(fs::exists(path));
  EXPECT_TRUE(this->chunk_store_->Store(name_mem, path, true));
  EXPECT_FALSE(fs::exists(path));

  // delete input file (new chunk)
  EXPECT_TRUE(this->chunk_store_->Store(new_name, new_path, true));
  EXPECT_EQ(new_name,
            crypto::Hash<crypto::SHA512>(this->chunk_store_->Get(new_name)));
  EXPECT_FALSE(fs::exists(path));
  EXPECT_TRUE(this->chunk_store_->Store(new_name, new_path, true));
  EXPECT_FALSE(this->chunk_store_->Empty());
  EXPECT_EQ(3, this->chunk_store_->Count());
  EXPECT_EQ(912, this->chunk_store_->Size());
  EXPECT_TRUE(this->chunk_store_->Has(new_name));
  EXPECT_EQ(333, this->chunk_store_->Size(new_name));
}

TYPED_TEST_P(ChunkStoreTest, BEH_CS_Delete) {
  std::string content(RandomString(123));
  std::string name_mem(crypto::Hash<crypto::SHA512>(content));
  fs::path path(this->test_dir_ / "chunk.dat");
  this->CreateRandomFile(path, 456);
  std::string name_file(crypto::HashFile<crypto::SHA512>(path));
  ASSERT_NE(name_mem, name_file);

  // invalid input
  EXPECT_FALSE(this->chunk_store_->Delete(""));

  // non-existing chunk
  EXPECT_TRUE(this->chunk_store_->Delete(name_mem));

  ASSERT_TRUE(this->chunk_store_->Store(name_mem, content));
  ASSERT_TRUE(this->chunk_store_->Store(name_file, path, true));

  EXPECT_FALSE(this->chunk_store_->Empty());
  EXPECT_EQ(2, this->chunk_store_->Count());
  EXPECT_EQ(579, this->chunk_store_->Size());
  EXPECT_TRUE(this->chunk_store_->Has(name_mem));
  EXPECT_TRUE(this->chunk_store_->Has(name_file));

  // Delete existing chunks
  EXPECT_TRUE(this->chunk_store_->Delete(name_file));
  EXPECT_FALSE(this->chunk_store_->Has(name_file));
  EXPECT_TRUE(this->chunk_store_->Get(name_file).empty());
  EXPECT_EQ(1, this->chunk_store_->Count());
  EXPECT_EQ(123, this->chunk_store_->Size());
  EXPECT_TRUE(this->chunk_store_->Delete(name_mem));
  EXPECT_FALSE(this->chunk_store_->Has(name_mem));
  EXPECT_TRUE(this->chunk_store_->Get(name_mem).empty());

  EXPECT_TRUE(this->chunk_store_->Empty());
  EXPECT_EQ(0, this->chunk_store_->Count());
  EXPECT_EQ(0, this->chunk_store_->Size());
}

TYPED_TEST_P(ChunkStoreTest, BEH_CS_MoveTo) {
  std::string content1(RandomString(100));
  std::string name1(crypto::Hash<crypto::SHA512>(content1));
  std::string content2(RandomString(50));
  std::string name2(crypto::Hash<crypto::SHA512>(content2));
  std::string content3(RandomString(25));
  std::string name3(crypto::Hash<crypto::SHA512>(content3));

  // ( | )  ->  (1 2 | 2 3)
  EXPECT_TRUE(this->chunk_store_->Store(name1, content1));
  EXPECT_TRUE(this->chunk_store_->Store(name2, content2));
  EXPECT_EQ(2, this->chunk_store_->Count());
  EXPECT_EQ(150, this->chunk_store_->Size());
  EXPECT_TRUE(this->alt_chunk_store_->Store(name2, content2));
  EXPECT_TRUE(this->alt_chunk_store_->Store(name3, content3));
  EXPECT_EQ(2, this->alt_chunk_store_->Count());
  EXPECT_EQ(75, this->alt_chunk_store_->Size());

  // (1 2 | 2 3)  ->  (1 | 2 3)
  EXPECT_TRUE(this->chunk_store_->MoveTo(name2, this->alt_chunk_store_.get()));
  EXPECT_FALSE(this->chunk_store_->Has(name2));
  EXPECT_EQ(1, this->chunk_store_->Count());
  EXPECT_EQ(100, this->chunk_store_->Size());
  EXPECT_TRUE(this->alt_chunk_store_->Has(name2));
  EXPECT_EQ(2, this->alt_chunk_store_->Count());
  EXPECT_EQ(75, this->alt_chunk_store_->Size());

  // (1 | 2 3)  ->  (1 2 | 3)
  EXPECT_TRUE(this->alt_chunk_store_->MoveTo(name2, this->chunk_store_.get()));
  EXPECT_TRUE(this->chunk_store_->Has(name2));
  EXPECT_EQ(2, this->chunk_store_->Count());
  EXPECT_EQ(150, this->chunk_store_->Size());
  EXPECT_FALSE(this->alt_chunk_store_->Has(name2));
  EXPECT_EQ(1, this->alt_chunk_store_->Count());
  EXPECT_EQ(25, this->alt_chunk_store_->Size());

  // (1 2 | 3)  ->  (1 2 3 | )
  EXPECT_TRUE(this->alt_chunk_store_->MoveTo(name3, this->chunk_store_.get()));
  EXPECT_TRUE(this->chunk_store_->Has(name3));
  EXPECT_EQ(3, this->chunk_store_->Count());
  EXPECT_EQ(175, this->chunk_store_->Size());
  EXPECT_FALSE(this->alt_chunk_store_->Has(name3));
  EXPECT_EQ(0, this->alt_chunk_store_->Count());
  EXPECT_EQ(0, this->alt_chunk_store_->Size());
  EXPECT_TRUE(this->alt_chunk_store_->Empty());

  // failures
  EXPECT_FALSE(this->alt_chunk_store_->MoveTo(name1, this->chunk_store_.get()));
  EXPECT_FALSE(this->chunk_store_->MoveTo("", this->alt_chunk_store_.get()));
  EXPECT_FALSE(this->chunk_store_->MoveTo(name1, NULL));
}

TYPED_TEST_P(ChunkStoreTest, BEH_CS_Validate) {
  std::string content(RandomString(123));
  std::string name(crypto::Hash<crypto::SHA512>(content));

  EXPECT_FALSE(this->chunk_store_->Validate(""));
  EXPECT_FALSE(this->chunk_store_->Validate(name));
  ASSERT_TRUE(this->chunk_store_->Store(name, content));
  EXPECT_TRUE(this->chunk_store_->Validate(name));
  ASSERT_TRUE(this->chunk_store_->Delete(name));
  ASSERT_TRUE(this->chunk_store_->Store(name, "this won't validate"));
  EXPECT_FALSE(this->chunk_store_->Validate(name));
}

TYPED_TEST_P(ChunkStoreTest, BEH_CS_Capacity) {
  std::string content1(RandomString(100));
  std::string name1(crypto::Hash<crypto::SHA512>(content1));
  std::string content2(RandomString(50));
  std::string name2(crypto::Hash<crypto::SHA512>(content2));
  std::string content3(RandomString(25));
  std::string name3(crypto::Hash<crypto::SHA512>(content3));

  EXPECT_EQ(0, this->chunk_store_->Capacity());
  EXPECT_TRUE(this->chunk_store_->Vacant(0));
  EXPECT_TRUE(this->chunk_store_->Vacant(123456789));
  this->chunk_store_->SetCapacity(125);
  EXPECT_EQ(125, this->chunk_store_->Capacity());
  EXPECT_TRUE(this->chunk_store_->Vacant(125));
  EXPECT_FALSE(this->chunk_store_->Vacant(126));

  // store #1, space to 100
  EXPECT_TRUE(this->chunk_store_->Vacant(content1.size()));
  EXPECT_TRUE(this->chunk_store_->Store(name1, content1));
  EXPECT_EQ(100, this->chunk_store_->Size());

  // try storing #2, 25 over limit
  EXPECT_FALSE(this->chunk_store_->Vacant(content2.size()));
  EXPECT_FALSE(this->chunk_store_->Store(name2, content2));
  EXPECT_EQ(100, this->chunk_store_->Size());

  // store #3, space to 125, which equals limit
  EXPECT_TRUE(this->chunk_store_->Vacant(content3.size()));
  EXPECT_TRUE(this->chunk_store_->Store(name3, content3));
  EXPECT_EQ(125, this->chunk_store_->Size());

  this->chunk_store_->SetCapacity(150);

  // try storing #2, again 25 over limit
  EXPECT_FALSE(this->chunk_store_->Vacant(content2.size()));
  EXPECT_FALSE(this->chunk_store_->Store(name2, content2));
  EXPECT_EQ(125, this->chunk_store_->Size());

  // delete #3, space to 100
  EXPECT_TRUE(this->chunk_store_->Delete(name3));
  EXPECT_EQ(100, this->chunk_store_->Size());

  // store #2, space to 150, which equals limit
  EXPECT_TRUE(this->chunk_store_->Vacant(content2.size()));
  EXPECT_TRUE(this->chunk_store_->Store(name2, content2));
  EXPECT_EQ(150, this->chunk_store_->Size());

  // store #1 again, nothing changes
  EXPECT_FALSE(this->chunk_store_->Vacant(content1.size()));
  EXPECT_TRUE(this->chunk_store_->Store(name1, content1));
  EXPECT_EQ(150, this->chunk_store_->Size());

  // can't reduce capacity as space is taken
  EXPECT_EQ(150, this->chunk_store_->Capacity());
  this->chunk_store_->SetCapacity(125);
  EXPECT_EQ(150, this->chunk_store_->Capacity());

  EXPECT_TRUE(this->alt_chunk_store_->Store(name1, content1));
  EXPECT_TRUE(this->alt_chunk_store_->Store(name3, content3));

  // moving #1 succeeds since it already exists
  EXPECT_FALSE(this->chunk_store_->Vacant(content1.size()));
  EXPECT_TRUE(this->alt_chunk_store_->MoveTo(name1, this->chunk_store_.get()));
  EXPECT_FALSE(this->alt_chunk_store_->Has(name1));

  // moving #3 fails since we are full
  EXPECT_FALSE(this->chunk_store_->Vacant(content3.size()));
  EXPECT_FALSE(this->alt_chunk_store_->MoveTo(name3, this->chunk_store_.get()));
  EXPECT_FALSE(this->chunk_store_->Has(name3));
  EXPECT_TRUE(this->alt_chunk_store_->Has(name3));

  // delete #1, space to 50
  EXPECT_TRUE(this->chunk_store_->Delete(name1));
  EXPECT_EQ(50, this->chunk_store_->Size());

  // moving #3 succeeds now
  EXPECT_TRUE(this->chunk_store_->Vacant(content3.size()));
  EXPECT_TRUE(this->alt_chunk_store_->MoveTo(name3, this->chunk_store_.get()));
  EXPECT_TRUE(this->chunk_store_->Has(name3));
  EXPECT_FALSE(this->alt_chunk_store_->Has(name3));
  EXPECT_EQ(75, this->chunk_store_->Size());

  // reducing capacity succeeds now
  EXPECT_EQ(150, this->chunk_store_->Capacity());
  this->chunk_store_->SetCapacity(125);
  EXPECT_EQ(125, this->chunk_store_->Capacity());
}

TYPED_TEST_P(ChunkStoreTest, BEH_CS_Clear) {
  std::vector<std::string> chunks;
  for (int i = 0; i < 20; ++i) {
    std::string content(RandomString(100));
    std::string name(crypto::Hash<crypto::SHA512>(content));
    chunks.push_back(name);
    EXPECT_TRUE(this->chunk_store_->Store(name, content));
    EXPECT_TRUE(this->chunk_store_->Has(name));
  }
  EXPECT_FALSE(this->chunk_store_->Empty());
  EXPECT_EQ(20, this->chunk_store_->Count());
  EXPECT_EQ(2000, this->chunk_store_->Size());

  this->chunk_store_->Clear();

  for (auto it = chunks.begin(); it != chunks.end(); ++it)
    EXPECT_FALSE(this->chunk_store_->Has(*it));
  EXPECT_TRUE(this->chunk_store_->Empty());
  EXPECT_EQ(0, this->chunk_store_->Count());
  EXPECT_EQ(0, this->chunk_store_->Size());
}

REGISTER_TYPED_TEST_CASE_P(ChunkStoreTest,
                           BEH_CS_Init,
                           BEH_CS_Get,
                           BEH_CS_Store,
                           BEH_CS_Delete,
                           BEH_CS_MoveTo,
                           BEH_CS_Validate,
                           BEH_CS_Capacity,
                           BEH_CS_Clear);

}  // namespace test

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_TESTS_TEST_CHUNK_STORE_API_H_
