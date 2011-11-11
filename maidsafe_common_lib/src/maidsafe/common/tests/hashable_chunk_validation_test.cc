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
#include <string>
#include "boost/filesystem.hpp"
#include "boost/filesystem/fstream.hpp"

#include "maidsafe/common/crypto.h"
#include "maidsafe/common/hashable_chunk_validation.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/common/test.h"

namespace maidsafe {

namespace test {

template <class ValidationType>
class HashableChunkValidationTest: public testing::Test {
 public:
  HashableChunkValidationTest()
      : test_dir_(CreateTestPath("MaidSafe_TestHashableChunkValidation")),
        chunk_validation_() {}
  ~HashableChunkValidationTest() {}
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
  TestPath test_dir_;
  HashableChunkValidation<ValidationType, crypto::Tiger> chunk_validation_;
};

TYPED_TEST_CASE_P(HashableChunkValidationTest);

TYPED_TEST_P(HashableChunkValidationTest, BEH_ValidName) {
  EXPECT_FALSE(this->chunk_validation_.ValidName(""));
  EXPECT_TRUE(this->chunk_validation_.ValidName("xyz"));
  EXPECT_TRUE(this->chunk_validation_.ValidName(
      crypto::Hash<TypeParam>("test")));
}

TYPED_TEST_P(HashableChunkValidationTest, BEH_Hashable) {
  EXPECT_FALSE(this->chunk_validation_.Hashable(""));
  EXPECT_FALSE(this->chunk_validation_.Hashable("abc"));
  EXPECT_TRUE(this->chunk_validation_.Hashable(
      crypto::Hash<TypeParam>("test")));
  EXPECT_TRUE(this->chunk_validation_.Hashable(
      RandomString(TypeParam::DIGESTSIZE)));
  EXPECT_FALSE(this->chunk_validation_.Hashable(
      RandomString(TypeParam::DIGESTSIZE + 1)));
}

TYPED_TEST_P(HashableChunkValidationTest, BEH_Modifiable) {
  EXPECT_FALSE(this->chunk_validation_.Modifiable(""));
  EXPECT_TRUE(this->chunk_validation_.Modifiable("abc"));
  EXPECT_FALSE(this->chunk_validation_.Modifiable(
      crypto::Hash<TypeParam>("test")));
  EXPECT_FALSE(this->chunk_validation_.Modifiable(
      RandomString(TypeParam::DIGESTSIZE)));
  EXPECT_TRUE(this->chunk_validation_.Modifiable(
      RandomString(TypeParam::DIGESTSIZE + 1)));
}

TYPED_TEST_P(HashableChunkValidationTest, BEH_ValidChunkString) {
  std::string data(RandomString(123)), name(crypto::Hash<TypeParam>(data));

  EXPECT_FALSE(this->chunk_validation_.ValidChunk("", data));
  EXPECT_FALSE(this->chunk_validation_.ValidChunk(
      RandomString(TypeParam::DIGESTSIZE), data));
  EXPECT_FALSE(this->chunk_validation_.ValidChunk(name, std::string()));
  EXPECT_FALSE(this->chunk_validation_.ValidChunk(name, std::string("fail")));
  EXPECT_TRUE(this->chunk_validation_.ValidChunk(name, data));
  EXPECT_TRUE(this->chunk_validation_.ValidChunk("test", data));
}

TYPED_TEST_P(HashableChunkValidationTest, BEH_ValidChunkFile) {
  fs::path path(CreateRandomFile(*(this->test_dir_) / "file1", 123));
  fs::path path2(CreateRandomFile(*(this->test_dir_) / "file2", 321));
  std::string name(crypto::HashFile<TypeParam>(path));

  EXPECT_FALSE(this->chunk_validation_.ValidChunk("", path));
  EXPECT_FALSE(this->chunk_validation_.ValidChunk(
      RandomString(TypeParam::DIGESTSIZE), path));
  EXPECT_FALSE(this->chunk_validation_.ValidChunk(name, path2));
  EXPECT_TRUE(this->chunk_validation_.ValidChunk(name, path));
  EXPECT_TRUE(this->chunk_validation_.ValidChunk("test", path));
}

TYPED_TEST_P(HashableChunkValidationTest, BEH_ChunkStringVersion) {
  std::string data(RandomString(123));
  std::string name1(crypto::Hash<TypeParam>(data));
  std::string name2(RandomString(TypeParam::DIGESTSIZE + 1));
  std::string version(crypto::Hash<crypto::Tiger>(data));

  EXPECT_TRUE(this->chunk_validation_.Version("", data).empty());
//   EXPECT_TRUE(this->chunk_validation_.Version(name1, std::string()).empty());
  EXPECT_EQ(name1, this->chunk_validation_.Version(name1, data));
  EXPECT_EQ(version, this->chunk_validation_.Version(name2, data));
}

TYPED_TEST_P(HashableChunkValidationTest, BEH_ChunkFileVersion) {
  fs::path path(CreateRandomFile(*(this->test_dir_) / "file", 123));
  std::string name1(crypto::HashFile<TypeParam>(path));
  std::string name2(RandomString(TypeParam::DIGESTSIZE + 1));
  std::string version(crypto::HashFile<crypto::Tiger>(path));

  EXPECT_TRUE(this->chunk_validation_.Version("", path).empty());
//   EXPECT_TRUE(this->chunk_validation_.Version(name1, fs::path()).empty());
  EXPECT_EQ(name1, this->chunk_validation_.Version(name1, path));
  EXPECT_EQ(version, this->chunk_validation_.Version(name2, path));
}

REGISTER_TYPED_TEST_CASE_P(HashableChunkValidationTest,
                           BEH_ValidName,
                           BEH_Hashable,
                           BEH_Modifiable,
                           BEH_ValidChunkString,
                           BEH_ValidChunkFile,
                           BEH_ChunkStringVersion,
                           BEH_ChunkFileVersion);

typedef testing::Types<crypto::SHA1,
                       crypto::SHA256,
                       crypto::SHA384,
                       crypto::SHA512,
                       crypto::Tiger> HashTestTypes;

INSTANTIATE_TYPED_TEST_CASE_P(Hash,
                              HashableChunkValidationTest,
                              HashTestTypes);

}  // namespace test

}  // namespace maidsafe
