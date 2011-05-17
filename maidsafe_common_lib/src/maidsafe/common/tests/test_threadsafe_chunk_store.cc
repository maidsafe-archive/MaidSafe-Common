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

namespace test_tscs {
  const size_t kIterationSize = 13;
}

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
        threadsafe_chunk_store_(),
        total_chunk_size_(),
        mutex_() {
    std::shared_ptr<MemoryChunkStore> chunk_store;
    chunk_store.reset(new MemoryChunkStore(false, std::bind(
        &crypto::Hash<crypto::SHA512>, std::placeholders::_1)));
    threadsafe_chunk_store_.reset(new ThreadsafeChunkStore(false, chunk_store));
    thread_pool_.reset(new Threadpool(30));
  }

  ~ThreadsafeChunkStoreTest() {}
  void SetUp() {
    StoreContents(17, false);
    StoreFromSourceFile(13, false);
  }
  template <class HashType>
  void InitChunkStore(std::shared_ptr<ChunkStore> *chunk_store,
                      bool reference_counting,
                      const fs::path &chunk_dir);
  void StoreContents(const size_t &num, const bool &check_flag) {
    for (uint16_t i = 1; i < num + 1; ++i) {
      std::string contents = RandomString(64 * i);
      std::string chunk_name = crypto::Hash<crypto::SHA512>(contents);
      threadsafe_chunk_store_->Store(chunk_name, contents);

      if (check_flag) {
        EXPECT_TRUE(threadsafe_chunk_store_->Has(chunk_name));
      } else {
        chunkname_.push_back(chunk_name);
        total_chunk_size_+= (64 * i);
      }
    }
  }
  void StoreFromSourceFile(const size_t &num, const bool &check_flag) {
    for (uint16_t i = 1; i < num + 1; ++i) {
      std::string chunk(RandomAlphaNumericString(6));
      fs::path path(*test_dir_ / chunk);
      CreateRandomFile(path, 177 * i);
      std::string file_name = crypto::HashFile<crypto::SHA512>(path);
      threadsafe_chunk_store_->Store(file_name, path, true);

      if (check_flag) {
        EXPECT_TRUE(threadsafe_chunk_store_->Has(file_name));
      } else {
        chunkname_.push_back(file_name);
        total_chunk_size_+= (177 * i);
      }
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

  void DeleteChunk(const std::string &chunk_name) {
    EXPECT_TRUE(threadsafe_chunk_store_->Delete(chunk_name));
    EXPECT_FALSE(threadsafe_chunk_store_->Has(chunk_name));
  }

  void ChunkSize(const std::string &chunk_name, uint64_t *total_size) {
    uint64_t chunk_size = threadsafe_chunk_store_->Size(chunk_name);

    boost::mutex::scoped_lock lock(mutex_);
    *total_size = *total_size + chunk_size;
  }

  void ValidateChunk(const std::string &chunk_name) {
    EXPECT_TRUE(threadsafe_chunk_store_->Validate(chunk_name));
  }

  void Size() {
    EXPECT_EQ(total_chunk_size_, threadsafe_chunk_store_->Size());
  }

  void ChunkStoreCapacity(const uint64_t &capacity) {
    EXPECT_EQ(capacity, threadsafe_chunk_store_->Capacity());
  }

  void SetCapacity(const size_t &index, const std::vector<uint32_t> &capacity) {
    threadsafe_chunk_store_->SetCapacity(capacity[index]);
    auto it = std::find(capacity.begin(), capacity.end(),
                        threadsafe_chunk_store_->Capacity());
    EXPECT_NE(it, capacity.end());
  }

  void Vacant(const uint64_t &required_size) {
    EXPECT_TRUE(threadsafe_chunk_store_->Vacant(required_size));
  }

  void ChunkCount(const std::string &chunk_name) {
    EXPECT_EQ(uintmax_t(1), threadsafe_chunk_store_->Count(chunk_name));
  }

  void TotalChunk() {
    EXPECT_EQ(uintmax_t(chunkname_.size()), threadsafe_chunk_store_->Count());
  }

  void EmptyChunk() {
    EXPECT_FALSE(threadsafe_chunk_store_->Empty());
  }

  void ClearChunk() {
    threadsafe_chunk_store_->Clear();
    EXPECT_TRUE(threadsafe_chunk_store_->Empty());
  }

  void MoveChunk(const std::string &chunk_name,
                 ChunkStore *another_chunk_store) {
    EXPECT_TRUE(threadsafe_chunk_store_->MoveTo(chunk_name,
                                                another_chunk_store));
    EXPECT_FALSE(threadsafe_chunk_store_->Has(chunk_name));
    EXPECT_TRUE(another_chunk_store->Has(chunk_name));
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
  uint64_t total_chunk_size_;
  boost::mutex mutex_;
};

TEST_F(ThreadsafeChunkStoreTest, FUNC_TSCS_Get) {
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
  this->thread_pool_->Stop();
}

TEST_F(ThreadsafeChunkStoreTest, BEH_TSCS_Has) {
  size_t entry_size = this->chunkname_.size();
  uint32_t index = RandomUint32();
  for (size_t i = 0; i < entry_size; ++i) {
    auto it = this->chunkname_.at(index % entry_size);
    this->thread_pool_->EnqueueTask(
        std::bind(&ThreadsafeChunkStoreTest::HasChunk, this, it));
  }
  this->thread_pool_->Stop();
}

TEST_F(ThreadsafeChunkStoreTest, BEH_TSCS_Delete) {
  size_t entry_size = this->chunkname_.size();
  uint32_t index = RandomUint32();
  for (size_t i = 0; i < entry_size; ++i) {
    auto it = this->chunkname_.at(index % entry_size);
    this->thread_pool_->EnqueueTask(
        std::bind(&ThreadsafeChunkStoreTest::DeleteChunk, this, it));
  }
  this->thread_pool_->Stop();
}

TEST_F(ThreadsafeChunkStoreTest, BEH_TSCS_Validate) {
  size_t entry_size = this->chunkname_.size();
  uint32_t index = RandomUint32();
  for (size_t i = 0; i < entry_size; ++i) {
    auto it = this->chunkname_.at(index % entry_size);
    this->thread_pool_->EnqueueTask(
        std::bind(&ThreadsafeChunkStoreTest::ValidateChunk, this, it));
  }
  this->thread_pool_->Stop();
}

TEST_F(ThreadsafeChunkStoreTest, BEH_TSCS_Size_For_Chunk) {
  size_t entry_size = this->chunkname_.size();
  uint64_t total_size(0);
  for (size_t i = 0; i < entry_size; ++i) {
    auto it = this->chunkname_.at(i);
    this->thread_pool_->EnqueueTask(
        std::bind(&ThreadsafeChunkStoreTest::ChunkSize, this, it, &total_size));
  }
  this->thread_pool_->Stop();
  EXPECT_EQ(this->total_chunk_size_, total_size);
}

TEST_F(ThreadsafeChunkStoreTest, BEH_TSCS_Size) {
  size_t entry_size = this->chunkname_.size();
  for (size_t i = 0; i < entry_size; ++i) {
    this->thread_pool_->EnqueueTask(
        std::bind(&ThreadsafeChunkStoreTest::Size, this));
  }
  this->thread_pool_->Stop();
}

TEST_F(ThreadsafeChunkStoreTest, BEH_TSCS_Capacity) {
  size_t entry_size = this->chunkname_.size();
  uint64_t capacity = total_chunk_size_ * 3;
  this->threadsafe_chunk_store_->SetCapacity(capacity);
  for (size_t i = 0; i < entry_size; ++i) {
    this->thread_pool_->EnqueueTask(
        std::bind(&ThreadsafeChunkStoreTest::ChunkStoreCapacity, this,
        capacity));
  }
  this->thread_pool_->Stop();
}

TEST_F(ThreadsafeChunkStoreTest, BEH_TSCS_SetCapacity) {
  std::vector<uint32_t> capacity;
  for (size_t i = 0; i < test_tscs::kIterationSize; ++i)
    capacity.push_back(RandomUint32());

  for (size_t i = 0; i < test_tscs::kIterationSize; ++i) {
    this->thread_pool_->EnqueueTask(
        std::bind(&ThreadsafeChunkStoreTest::SetCapacity, this, i, capacity));
  }
  this->thread_pool_->Stop();
}

TEST_F(ThreadsafeChunkStoreTest, BEH_TSCS_Vacant) {
  uint64_t capacity = total_chunk_size_ * 3;
  this->threadsafe_chunk_store_->SetCapacity(capacity);
  for (size_t i = 0; i < test_tscs::kIterationSize; ++i) {
    this->thread_pool_->EnqueueTask(
        std::bind(&ThreadsafeChunkStoreTest::Vacant, this, total_chunk_size_));
  }
  this->thread_pool_->Stop();
}

TEST_F(ThreadsafeChunkStoreTest, BEH_TSCS_Count) {
  size_t entry_size = this->chunkname_.size();
  uint32_t index = RandomUint32();
  for (size_t i = 0; i < entry_size; ++i) {
    auto it = this->chunkname_.at(index % entry_size);
    this->thread_pool_->EnqueueTask(
        std::bind(&ThreadsafeChunkStoreTest::ChunkCount, this, it));
  }
  this->thread_pool_->Stop();
}

TEST_F(ThreadsafeChunkStoreTest, BEH_TSCS_Count_Total) {
  for (size_t i = 0; i < test_tscs::kIterationSize; ++i) {
    this->thread_pool_->EnqueueTask(
        std::bind(&ThreadsafeChunkStoreTest::TotalChunk, this));
  }
  this->thread_pool_->Stop();
}

TEST_F(ThreadsafeChunkStoreTest, BEH_TSCS_Empty) {
  for (size_t i = 0; i < test_tscs::kIterationSize; ++i) {
    this->thread_pool_->EnqueueTask(
        std::bind(&ThreadsafeChunkStoreTest::EmptyChunk, this));
  }
  this->thread_pool_->Stop();
}

TEST_F(ThreadsafeChunkStoreTest, BEH_TSCS_Clear) {
  for (size_t i = 0; i < test_tscs::kIterationSize; ++i) {
    this->thread_pool_->EnqueueTask(
        std::bind(&ThreadsafeChunkStoreTest::ClearChunk, this));
  }
  this->thread_pool_->Stop();
}

TEST_F(ThreadsafeChunkStoreTest, BEH_TSCS_MoveTo) {
  MemoryChunkStore another_chunk_store(
      false, std::bind(&crypto::Hash<crypto::SHA512>, std::placeholders::_1));
  size_t entry_size = this->chunkname_.size();
  for (size_t i = 0; i < entry_size; ++i) {
    auto it = this->chunkname_.at(i);
    this->thread_pool_->EnqueueTask(
        std::bind(&ThreadsafeChunkStoreTest::MoveChunk, this, it,
                  &another_chunk_store));
  }
  this->thread_pool_->Stop();
}

TEST_F(ThreadsafeChunkStoreTest, BEH_TSCS_Store) {
  for (size_t i = 0; i < test_tscs::kIterationSize; ++i) {
    this->thread_pool_->EnqueueTask(
        std::bind(&ThreadsafeChunkStoreTest::StoreContents, this, i, true));
    this->thread_pool_->EnqueueTask(
        std::bind(&ThreadsafeChunkStoreTest::StoreFromSourceFile, this, i,
                  true));
  }
  this->thread_pool_->Stop();
}

TEST_F(ThreadsafeChunkStoreTest, BEH_TSCS_Misc) {
  std::vector<std::string> delete_chunknames, moveto_chunknames;
  for (uint16_t i = 0; i < 17; ++i) {
    std::string contents = RandomString(64 * i);
    std::string chunk_name = crypto::Hash<crypto::SHA512>(contents);
    threadsafe_chunk_store_->Store(chunk_name, contents);
    if (i < 10)
      delete_chunknames.push_back(chunk_name);
    else
      moveto_chunknames.push_back(chunk_name);
  }
  typedef std::function<void()> functor;
  std::vector<functor> functors;

  // Create Delete functor
  for (size_t i = 0; i < delete_chunknames.size(); ++i) {
    functor f1 = std::bind(&ThreadsafeChunkStoreTest::DeleteChunk, this,
                           delete_chunknames[i]);
    functors.push_back(f1);
  }

  // Create MoveTo functor
  MemoryChunkStore another_chunk_store(
      false, std::bind(&crypto::Hash<crypto::SHA512>, std::placeholders::_1));
  for (size_t i = 0; i < moveto_chunknames.size(); ++i) {
    functor f1 = std::bind(&ThreadsafeChunkStoreTest::MoveChunk, this,
                           moveto_chunknames[i], &another_chunk_store);
    functors.push_back(f1);
  }

  //  create Store functor
  for (size_t i = 0; i < test_tscs::kIterationSize; ++i) {
    functor f1 = std::bind(&ThreadsafeChunkStoreTest::StoreContents, this, i,
                           true);
    functor f2 = std::bind(&ThreadsafeChunkStoreTest::StoreFromSourceFile,
                           this, i, true);
    functors.push_back(f1);
    functors.push_back(f2);
  }

  //  create Has functor
  for (size_t i = 0; i < chunkname_.size(); ++i) {
    functor f1 = std::bind(&ThreadsafeChunkStoreTest::HasChunk, this,
                           chunkname_[i]);
    functors.push_back(f1);
  }

  // create Get functor
  for (size_t i = 0; i < chunkname_.size(); ++i) {
    std::string chunk(RandomAlphaNumericString(6));
    fs::path path(*test_dir_ / chunk);
    functor f1 = std::bind(&ThreadsafeChunkStoreTest::GetFileChunk, this,
                            chunkname_[i], path);
    functor f2 = std::bind(&ThreadsafeChunkStoreTest::GetMemChunk, this,
                           chunkname_[i]);
    functors.push_back(f1);
    functors.push_back(f2);
  }

  //  create Size functor
  uint64_t total_size(0);
  for (size_t i = 0; i < chunkname_.size(); ++i) {
    functor f1 = std::bind(&ThreadsafeChunkStoreTest::ChunkSize, this,
                           chunkname_[i], &total_size);
    functors.push_back(f1);
  }

  //  create Validate functor
  for (size_t i = 0; i < chunkname_.size(); ++i) {
    functor f1 = std::bind(&ThreadsafeChunkStoreTest::ValidateChunk, this,
                           chunkname_[i]);
    functors.push_back(f1);
  }

  //  create Count functor
  for (size_t i = 0; i < chunkname_.size(); ++i) {
    functor f1 = std::bind(&ThreadsafeChunkStoreTest::ChunkCount, this,
                           chunkname_[i]);
    functors.push_back(f1);
  }

  //  create Empty functor
  for (size_t i = 0; i < chunkname_.size(); ++i) {
    functor f1 = std::bind(&ThreadsafeChunkStoreTest::EmptyChunk, this);
    functors.push_back(f1);
  }

  //  create Capacity functor
  for (size_t i = 0; i < test_tscs::kIterationSize; ++i) {
    functor f1 = std::bind(&ThreadsafeChunkStoreTest::ChunkStoreCapacity, this,
                           0);
    functors.push_back(f1);
  }

  //  create vacant functor
  for (size_t i = 0; i < test_tscs::kIterationSize; ++i) {
    functor f1 = std::bind(&ThreadsafeChunkStoreTest::Vacant, this, (i * 13));
    functors.push_back(f1);
  }

  std::random_shuffle(functors.begin(), functors.end());

  for (size_t i = 0; i < functors.size(); ++i) {
    this->thread_pool_->EnqueueTask(functors[i]);
  }
  this->thread_pool_->Stop();
  // Check for Chunksizes
  EXPECT_EQ(this->total_chunk_size_, total_size);
}

}  // namespace test

}  // namespace maidsafe