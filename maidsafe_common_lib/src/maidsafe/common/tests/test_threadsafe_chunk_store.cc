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
#include <functional>
#include <memory>
#include <cstring>
#include "boost/filesystem.hpp"
#include "boost/filesystem/fstream.hpp"
#include "boost/thread.hpp"
#include "boost/thread/thread.hpp"
#include "boost/scoped_ptr.hpp"

#include "maidsafe/common/crypto.h"
#include "maidsafe/common/tests/test_chunk_store_api.h"
#include "maidsafe/common/threadsafe_chunk_store.h"
#include "maidsafe/common/memory_chunk_store.h"
#include "maidsafe/common/threadpool.h"
#include "maidsafe/common/utils.h"

namespace maidsafe {

namespace test {

template <> template <class HashType>
void ChunkStoreTest<ThreadsafeChunkStore>::InitChunkStore(
    std::shared_ptr<ChunkStore> *chunk_store, bool reference_counting,
    const fs::path&) {
  std::shared_ptr<MemoryChunkStore> memory_chunk_store;
  memory_chunk_store.reset(new MemoryChunkStore(
      reference_counting,
      std::bind(&crypto::Hash<HashType>, std::placeholders::_1)));
  chunk_store->reset(new ThreadsafeChunkStore(reference_counting,
                                              memory_chunk_store));
}

INSTANTIATE_TYPED_TEST_CASE_P(Threadsafe, ChunkStoreTest, ThreadsafeChunkStore);

class ThreadsafeChunkStoreTest: public testing::Test {
 public:
  ThreadsafeChunkStoreTest()
      : test_dir_(CreateTestPath("MaidSafe_TestFileChunkStore")),
        chunkname_(),
        thread_pool_(),
        threadsafe_chunk_store_() {
    std::shared_ptr<MemoryChunkStore> chunk_store;
    chunk_store.reset(new MemoryChunkStore(false, std::bind(
        &crypto::Hash<crypto::SHA512>, std::placeholders::_1)));
    threadsafe_chunk_store_.reset(new ThreadsafeChunkStore(false, chunk_store));
    thread_pool_.reset(new Threadpool(30));
  }

  ~ThreadsafeChunkStoreTest() {}
  void SetUp() {
    StoreContents(17);
    StoreFromSourceFile(13);
  }
  template <class HashType>
  void InitChunkStore(std::shared_ptr<ChunkStore> *chunk_store,
                      bool reference_counting,
                      const fs::path &chunk_dir);
  void StoreContents(const uint16_t &num) {
    for (uint16_t i = 0; i < num; ++i) {
      std::string contents = RandomString(64);
      std::string chunk_name = crypto::Hash<crypto::SHA512>(contents);
      chunkname_.push_back(chunk_name);
      threadsafe_chunk_store_->Store(chunk_name, contents);
    }
  }
  void StoreFromSourceFile(const uint16_t &num) {
    for (uint16_t i = 0; i < num; ++i) {
      fs::path path(*test_dir_ / "chunk.dat");
      CreateRandomFile(path, 177);
      std::string file_name = crypto::HashFile<crypto::SHA512>(path);
      threadsafe_chunk_store_->Store(file_name, path, true);
      chunkname_.push_back(file_name);
    }
  }

  void GetMemChunk(const std::string &chunk_name) {
    std::string content = threadsafe_chunk_store_->Get(chunk_name);
    auto it = std::find(chunkname_.begin(), chunkname_.end(), chunk_name);
    EXPECT_NE(it, chunkname_.end());

    if (it != chunkname_.end()) {
      EXPECT_EQ((*it), chunk_name);
      EXPECT_EQ((*it), crypto::Hash<crypto::SHA512>(content));
    }
  }

  void GetFileChunk(const std::string &chunk_name, const fs::path &path) {
    EXPECT_TRUE(threadsafe_chunk_store_->Get(chunk_name, path));
    EXPECT_TRUE(fs::exists(path));
    auto it = std::find(chunkname_.begin(), chunkname_.end(), chunk_name);
    EXPECT_NE(it, chunkname_.end());

    if (it != chunkname_.end()) {
      EXPECT_EQ((*it), chunk_name);
      EXPECT_EQ((*it), crypto::HashFile<crypto::SHA512>(path));
    }
  }

  void HasChunk(const std::string &chunk_name) {
    EXPECT_TRUE(threadsafe_chunk_store_->Has(chunk_name));
  }

 protected:
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
  std::vector<std::string> chunkname_;
  std::shared_ptr<Threadpool> thread_pool_;
  std::shared_ptr<ThreadsafeChunkStore> threadsafe_chunk_store_;
};

TEST_F(ThreadsafeChunkStoreTest, BEH_TSCS_Get) {
  size_t entry_size = this->chunkname_.size();
  uint32_t index = RandomUint32();
  for (size_t i = 0; i < entry_size; ++i) {
    auto it = this->chunkname_.at(index % entry_size);
    std::string chunk(RandomAlphaNumericString(6));
    fs::path path(*test_dir_ / chunk);
    this->thread_pool_->EnqueueTask(
      std::bind(&ThreadsafeChunkStoreTest::GetFileChunk, this, it, path));
    this->thread_pool_->EnqueueTask(
        std::bind(&ThreadsafeChunkStoreTest::GetMemChunk, this, it));
  }
  thread_pool_->Stop();
}
TEST_F(ThreadsafeChunkStoreTest, BEH_TSCS_Has) {
  size_t entry_size = this->chunkname_.size();
  uint32_t index = RandomUint32();
  for (size_t i = 0; i < entry_size; ++i) {
    auto it = this->chunkname_.at(index % entry_size);
    this->thread_pool_->EnqueueTask(
        std::bind(&ThreadsafeChunkStoreTest::HasChunk, this, it));
  }
  thread_pool_->Stop();
}


}  // namespace test

}  // namespace maidsafe
