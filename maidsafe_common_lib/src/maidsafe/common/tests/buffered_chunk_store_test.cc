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

#include <memory>

#include "boost/filesystem.hpp"
#include "boost/filesystem/fstream.hpp"
#include "boost/thread.hpp"

#include "maidsafe/common/file_chunk_store.h"
#include "maidsafe/common/memory_chunk_store.h"
#include "maidsafe/common/buffered_chunk_store.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"

namespace maidsafe {

namespace test {

class BufferedChunkStoreTest: public testing::Test {
 public:
  BufferedChunkStoreTest()
      : test_dir_(CreateTestPath("MaidSafe_TestFileChunkStore")),
        chunk_dir_(*test_dir_ / "chunks"),
        ref_chunk_dir_(*test_dir_ / "ref_chunks"),
        alt_chunk_dir_(*test_dir_ / "alt_chunks"),
        asio_service_(),
        work_(),
        t_group_(),
        chunk_store_(),
        alt_chunk_store_(),
        ref_chunk_store_() {
    work_.reset(new boost::asio::io_service::work(asio_service_));
    for (int i = 0; i < 1; ++i)
      t_group_.create_thread(std::bind(static_cast<
          std::size_t(boost::asio::io_service::*)()>
             (&boost::asio::io_service::run), &asio_service_));
  }
  ~BufferedChunkStoreTest() {}
  void DeleteOperation(const std::string &name_mem,
                       const std::string &name_file) {
    EXPECT_FALSE(chunk_store_->Empty());
    EXPECT_EQ(2, chunk_store_->Count());
    EXPECT_EQ(579, chunk_store_->Size());
    EXPECT_TRUE(chunk_store_->Has(name_mem));
    EXPECT_EQ(1, chunk_store_->Count(name_mem));
    EXPECT_EQ(123, chunk_store_->Size(name_mem));
    EXPECT_TRUE(chunk_store_->Has(name_file));
    EXPECT_EQ(1, chunk_store_->Count(name_file));
    EXPECT_EQ(456, chunk_store_->Size(name_file));

    // Delete existing chunks
    EXPECT_TRUE(chunk_store_->Delete(name_file));
    EXPECT_FALSE(chunk_store_->Has(name_file));
    EXPECT_EQ(0, chunk_store_->Count(name_file));
    EXPECT_EQ(0, chunk_store_->Size(name_file));
    EXPECT_TRUE(chunk_store_->Get(name_file).empty());
    EXPECT_EQ(1, chunk_store_->Count());
    EXPECT_EQ(123, chunk_store_->Size());
    EXPECT_TRUE(chunk_store_->Delete(name_mem));
    EXPECT_FALSE(chunk_store_->Has(name_mem));
    EXPECT_EQ(0, chunk_store_->Count(name_mem));
    EXPECT_EQ(0, chunk_store_->Size(name_mem));
    EXPECT_TRUE(chunk_store_->Get(name_mem).empty());

    EXPECT_TRUE(chunk_store_->Empty());
    EXPECT_EQ(0, chunk_store_->Count());
    EXPECT_EQ(0, chunk_store_->Size());
  }

  void ClearOperation(const std::vector<std::string> &chunks) {
    for (auto it = chunks.begin(); it != chunks.end(); ++it)
      EXPECT_TRUE(chunk_store_->Has(*it));
    EXPECT_FALSE(chunk_store_->Empty());
    EXPECT_EQ(20, chunk_store_->Count());
    EXPECT_EQ(2000, chunk_store_->Size());

    chunk_store_->Clear();

    for (auto it = chunks.begin(); it != chunks.end(); ++it)
      EXPECT_FALSE(chunk_store_->Has(*it));
    EXPECT_TRUE(chunk_store_->Empty());
    EXPECT_EQ(0, chunk_store_->Count());
    EXPECT_EQ(0, chunk_store_->Size());
  }

  bool StoreDone(const std::string &name) {
    return chunk_store_->Has(name);
  }
 protected:
  void SetUp() {
    fs::create_directories(chunk_dir_);
    fs::create_directories(ref_chunk_dir_);
    fs::create_directories(alt_chunk_dir_);
    InitChunkStore(&chunk_store_, false, chunk_dir_);
    InitChunkStore(&alt_chunk_store_, false, alt_chunk_dir_);
    InitChunkStore(&ref_chunk_store_, true, ref_chunk_dir_);
  }

  void TearDown() {
    work_.reset();
    asio_service_.stop();
    t_group_.join_all();
  }

  void InitChunkStore(
    std::shared_ptr<ChunkStore> *chunk_store, bool reference_counting,
    const fs::path &chunk_dir) {
    chunk_store->reset(new BufferedChunkStore(reference_counting,
                                              asio_service_));
    if (!chunk_dir.empty())
      reinterpret_cast<BufferedChunkStore*>(chunk_store->get())->Init(
          chunk_dir);
  }
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
  std::shared_ptr<fs::path> test_dir_;
  fs::path chunk_dir_, ref_chunk_dir_, alt_chunk_dir_;
  boost::asio::io_service asio_service_;
  std::shared_ptr<boost::asio::io_service::work> work_;
  boost::thread_group t_group_;
  std::shared_ptr<ChunkStore> chunk_store_;
  std::shared_ptr<ChunkStore> alt_chunk_store_;
  std::shared_ptr<ChunkStore> ref_chunk_store_;
};

TEST_F(BufferedChunkStoreTest, BEH_Init) {
  EXPECT_EQ(0, this->chunk_store_->Size());
  EXPECT_EQ(0, this->chunk_store_->Capacity());
  EXPECT_EQ(0, this->chunk_store_->Count());
  EXPECT_TRUE(this->chunk_store_->Empty());
  EXPECT_FALSE(this->chunk_store_->Has(""));
  EXPECT_FALSE(this->chunk_store_->Has("something"));
}

TEST_F(BufferedChunkStoreTest, BEH_Get) {
  std::string content(RandomString(100));
  std::string name(crypto::Hash<crypto::SHA512>(content));
  fs::path path(*this->test_dir_ / "chunk.dat");
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

TEST_F(BufferedChunkStoreTest, BEH_Validate) {
  std::string content1(RandomString(123)), content2(RandomString(213));
  std::string name1(crypto::Hash<crypto::SHA512>(content1)),
              name2(crypto::Hash<crypto::Tiger>(content2));

  EXPECT_FALSE(this->chunk_store_->Validate(""));
  EXPECT_FALSE(this->chunk_store_->Validate(name1));
  EXPECT_FALSE(this->chunk_store_->Validate(name2));

  ASSERT_TRUE(this->chunk_store_->Store(name1, content1));
  ASSERT_TRUE(this->chunk_store_->Store(name2, content2));

  EXPECT_TRUE(this->chunk_store_->Validate(name1));
  EXPECT_FALSE(this->chunk_store_->Validate(name2));

  ASSERT_TRUE(this->chunk_store_->Delete(name1));
  ASSERT_TRUE(this->chunk_store_->Store(name1, "this won't validate"));
  EXPECT_FALSE(this->chunk_store_->Validate(name1));
  EXPECT_TRUE(this->chunk_store_->Has(name1));
}

TEST_F(BufferedChunkStoreTest, BEH_SmallName) {
  EXPECT_FALSE(this->chunk_store_->Has("x"));
  EXPECT_EQ(0, this->chunk_store_->Count("x"));
  EXPECT_TRUE(this->chunk_store_->Get("x").empty());
  EXPECT_TRUE(this->chunk_store_->Store("x", "dummy"));
  // EXPECT_TRUE(this->chunk_store_->Has("x"));
  EXPECT_EQ(1, this->chunk_store_->Count("x"));
  EXPECT_EQ("dummy", this->chunk_store_->Get("x"));
  EXPECT_TRUE(this->chunk_store_->MoveTo("x", this->alt_chunk_store_.get()));
  EXPECT_FALSE(this->chunk_store_->Has("x"));
  EXPECT_TRUE(this->alt_chunk_store_->Has("x"));
  EXPECT_FALSE(this->alt_chunk_store_->Validate("x"));
}

TEST_F(BufferedChunkStoreTest, BEH_Delete) {
  std::string content(RandomString(123));
  std::string name_mem(crypto::Hash<crypto::SHA512>(content));
  fs::path path(*this->test_dir_ / "chunk.dat");
  this->CreateRandomFile(path, 456);
  std::string name_file(crypto::HashFile<crypto::SHA512>(path));
  ASSERT_NE(name_mem, name_file);

  // invalid input
  EXPECT_FALSE(this->chunk_store_->Delete(""));

  // non-existing chunk
  EXPECT_TRUE(this->chunk_store_->Delete(name_mem));

  ASSERT_TRUE(this->chunk_store_->Store(name_mem, content));
  ASSERT_TRUE(this->chunk_store_->Store(name_file, path, true));
  asio_service_.post(std::bind(&BufferedChunkStoreTest::DeleteOperation, this,
                               name_mem, name_file));
  /*
  EXPECT_FALSE(this->chunk_store_->Empty());
  EXPECT_EQ(2, this->chunk_store_->Count());
  EXPECT_EQ(579, this->chunk_store_->Size());
  EXPECT_TRUE(this->chunk_store_->Has(name_mem));
  EXPECT_EQ(1, this->chunk_store_->Count(name_mem));
  EXPECT_EQ(123, this->chunk_store_->Size(name_mem));
  EXPECT_TRUE(this->chunk_store_->Has(name_file));
  EXPECT_EQ(1, this->chunk_store_->Count(name_file));
  EXPECT_EQ(456, this->chunk_store_->Size(name_file));

  // Delete existing chunks
  EXPECT_TRUE(this->chunk_store_->Delete(name_file));
  EXPECT_FALSE(this->chunk_store_->Has(name_file));
  EXPECT_EQ(0, this->chunk_store_->Count(name_file));
  EXPECT_EQ(0, this->chunk_store_->Size(name_file));
  EXPECT_TRUE(this->chunk_store_->Get(name_file).empty());
  EXPECT_EQ(1, this->chunk_store_->Count());
  EXPECT_EQ(123, this->chunk_store_->Size());
  EXPECT_TRUE(this->chunk_store_->Delete(name_mem));
  EXPECT_FALSE(this->chunk_store_->Has(name_mem));
  EXPECT_EQ(0, this->chunk_store_->Count(name_mem));
  EXPECT_EQ(0, this->chunk_store_->Size(name_mem));
  EXPECT_TRUE(this->chunk_store_->Get(name_mem).empty());

  EXPECT_TRUE(this->chunk_store_->Empty());
  EXPECT_EQ(0, this->chunk_store_->Count());
  EXPECT_EQ(0, this->chunk_store_->Size()); */
}

TEST_F(BufferedChunkStoreTest, BEH_Clear) {
  std::vector<std::string> chunks;
  for (int i = 0; i < 20; ++i) {
    std::string content(RandomString(100));
    std::string name(crypto::Hash<crypto::SHA512>(content));
    chunks.push_back(name);
    EXPECT_TRUE(this->chunk_store_->Store(name, content));
  }
  asio_service_.post(std::bind(&BufferedChunkStoreTest::ClearOperation, this,
                               chunks));
}

TEST_F(BufferedChunkStoreTest, BEH_Store) {
  std::string content(RandomString(123));
  std::string name_mem(crypto::Hash<crypto::SHA512>(content));
  fs::path path(*this->test_dir_ / "chunk.dat");
  this->CreateRandomFile(path, 456);
  std::string name_file(crypto::HashFile<crypto::SHA512>(path));
  ASSERT_NE(name_mem, name_file);

  // invalid input
  EXPECT_FALSE(this->chunk_store_->Store(name_mem, ""));
  EXPECT_FALSE(this->chunk_store_->Store("", content));
  EXPECT_FALSE(this->chunk_store_->Store(name_file, "", false));
  EXPECT_FALSE(this->chunk_store_->Store(name_file, *this->test_dir_ / "fail",
                                         false));
  EXPECT_FALSE(this->chunk_store_->Store("", path, false));
  EXPECT_TRUE(this->chunk_store_->Empty());
  EXPECT_EQ(0, this->chunk_store_->Count());
  EXPECT_EQ(0, this->chunk_store_->Size());
  EXPECT_FALSE(this->chunk_store_->Has(name_mem));
  EXPECT_EQ(0, this->chunk_store_->Count(name_mem));
  EXPECT_EQ(0, this->chunk_store_->Size(name_mem));
  EXPECT_FALSE(this->chunk_store_->Has(name_file));
  EXPECT_EQ(0, this->chunk_store_->Count(name_file));
  EXPECT_EQ(0, this->chunk_store_->Size(name_file));

  // store from string
  EXPECT_TRUE(this->chunk_store_->Store(name_mem, content));
  while (!StoreDone(name_mem))
    Sleep(boost::posix_time::milliseconds(1));
  EXPECT_FALSE(this->chunk_store_->Empty());
  EXPECT_EQ(1, this->chunk_store_->Count());
  EXPECT_EQ(123, this->chunk_store_->Size());
  EXPECT_TRUE(this->chunk_store_->Has(name_mem));
  EXPECT_EQ(1, this->chunk_store_->Count(name_mem));
  EXPECT_EQ(123, this->chunk_store_->Size(name_mem));
  EXPECT_FALSE(this->chunk_store_->Has(name_file));
  EXPECT_EQ(0, this->chunk_store_->Count(name_file));
  EXPECT_EQ(0, this->chunk_store_->Size(name_file));

  ASSERT_EQ(name_mem,
            crypto::Hash<crypto::SHA512>(this->chunk_store_->Get(name_mem)));

  // store from file
  EXPECT_TRUE(this->chunk_store_->Store(name_file, path, false));
  while (!StoreDone(name_file))
    Sleep(boost::posix_time::milliseconds(1));
  EXPECT_FALSE(this->chunk_store_->Empty());
  EXPECT_EQ(2, this->chunk_store_->Count());
  EXPECT_EQ(579, this->chunk_store_->Size());
  EXPECT_TRUE(this->chunk_store_->Has(name_mem));
  EXPECT_EQ(1, this->chunk_store_->Count(name_mem));
  EXPECT_EQ(123, this->chunk_store_->Size(name_mem));
  EXPECT_TRUE(this->chunk_store_->Has(name_file));
  EXPECT_EQ(1, this->chunk_store_->Count(name_file));
  EXPECT_EQ(456, this->chunk_store_->Size(name_file));

  ASSERT_EQ(name_file,
            crypto::Hash<crypto::SHA512>(this->chunk_store_->Get(name_file)));

  fs::path new_path(*this->test_dir_ / "chunk2.dat");
  this->CreateRandomFile(new_path, 333);
  std::string new_name(crypto::HashFile<crypto::SHA512>(new_path));

  // overwrite existing, should be ignored
  EXPECT_TRUE(this->chunk_store_->Store(name_mem, ""));
  EXPECT_TRUE(this->chunk_store_->Store(name_mem, RandomString(222)));
  EXPECT_TRUE(this->chunk_store_->Store(name_file, "", false));
  EXPECT_TRUE(this->chunk_store_->Store(name_file, new_path, false));
  while (!StoreDone(name_file))
    Sleep(boost::posix_time::milliseconds(1));
  EXPECT_FALSE(this->chunk_store_->Empty());
  EXPECT_EQ(2, this->chunk_store_->Count());
  EXPECT_EQ(579, this->chunk_store_->Size());
  EXPECT_TRUE(this->chunk_store_->Has(name_mem));
  EXPECT_EQ(1, this->chunk_store_->Count(name_mem));
  EXPECT_EQ(123, this->chunk_store_->Size(name_mem));
  EXPECT_TRUE(this->chunk_store_->Has(name_file));
  EXPECT_EQ(1, this->chunk_store_->Count(name_file));
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
  while (!StoreDone(new_name))
    Sleep(boost::posix_time::milliseconds(1));
  EXPECT_EQ(new_name,
            crypto::Hash<crypto::SHA512>(this->chunk_store_->Get(new_name)));
  EXPECT_FALSE(fs::exists(path));
  EXPECT_TRUE(this->chunk_store_->Store(new_name, new_path, true));
  while (!StoreDone(new_name))
    Sleep(boost::posix_time::milliseconds(1));
  EXPECT_FALSE(this->chunk_store_->Empty());
  EXPECT_EQ(3, this->chunk_store_->Count());
  EXPECT_EQ(912, this->chunk_store_->Size());
  EXPECT_TRUE(this->chunk_store_->Has(new_name));
  EXPECT_EQ(1, this->chunk_store_->Count(new_name));
  EXPECT_EQ(333, this->chunk_store_->Size(new_name));
}

}  // namespace test

}  // namespace maidsafe
