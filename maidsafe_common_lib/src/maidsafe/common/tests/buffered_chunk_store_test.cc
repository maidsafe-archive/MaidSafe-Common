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
    for (int i = 0; i < 7; ++i)
      t_group_.create_thread(std::bind(static_cast<
          std::size_t(boost::asio::io_service::*)()>
             (&boost::asio::io_service::run), &asio_service_));
  }
  ~BufferedChunkStoreTest() {}
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

}  // namespace test

}  // namespace maidsafe
