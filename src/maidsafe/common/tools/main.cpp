#include <array>
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <chrono>
#include <fstream>
#include <future>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

#include "boost/algorithm/string.hpp"
#include "boost/asio/io_service.hpp"
#include "boost/filesystem.hpp"
#include "docopt/docopt.h"

#include "maidsafe/common/application_support_directories.h"
#include "maidsafe/common/crypto.h"
#include "maidsafe/common/on_scope_exit.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/common/stores.pb.h"

namespace fs = boost::filesystem;

using Args = std::map<std::string, docopt::value>;

namespace maidsafe {

// ============================== COMMON ==========================================================
static const char kUsage[] =
    R"(Usage:
  surefile_cracker <first-word> <last-word> [--surefile=<path>]
  surefile_cracker --password-file=<path> [--surefile=<path>]
  surefile_cracker --help

Options:
  -p <path>, --password-file=<path>   Path to password list.
  -s <path>, --surefile=<path>    Path to surefile [default: ./surefile].
  -h, --help                      Display this help message and exit.

The first usage employs brute force to try all passwords between <first> and
<last> inclusive.

The second usage tries various modifications to each of the passwords in the
given file.
)";

const std::chrono::time_point<std::chrono::steady_clock> kStart = std::chrono::steady_clock::now();
boost::asio::io_service asio_service;
std::unique_ptr<boost::asio::io_service::work> work(
    new boost::asio::io_service::work(asio_service));
std::condition_variable cond_var;
std::mutex mutex;
std::size_t task_count = 0;

void Stop() {
  work.reset();
  asio_service.stop();
  cond_var.notify_one();
}

crypto::CipherText ReadSurefile(const Args& args) {
  try {
    auto cipher_text = ReadFile(args.at("--surefile").asString());
    // assert(HexEncode(crypto::Hash<crypto::SHA512>(cipher_text)) ==
    //  "31f6aa36890eddda04c7b1d095470defc979151e8f76a7878afa298c458ae2f826f98026dadf85e080dd352da3f16259f5c02e59684181b2dc0083224fe06d12");
    return cipher_text;
  } catch (const std::exception& e) {
    std::cout << "Failed to parse " << args.at("--surefile") << ": " << e.what() << std::endl;
    throw 4;
  }
}

crypto::PlainText Decrypt(const crypto::CipherText& cipher_text, const std::string& password) {
  crypto::SecurePassword secure_password(crypto::Hash<crypto::SHA512>(password));
  crypto::AES256Key key(secure_password.string().substr(0, crypto::AES256_KeySize));
  crypto::AES256InitialisationVector iv(
      secure_password.string().substr(crypto::AES256_KeySize, crypto::AES256_IVSize));
  return crypto::SymmDecrypt(cipher_text, key, iv);
}

void Login(const crypto::CipherText& cipher_text, const std::string& password) {
  try {
    const auto plain_text = Decrypt(cipher_text, password);
    surefile::protobuf::Stores proto_stores;
    if (!proto_stores.ParseFromString(plain_text.string()) || proto_stores.store_size() == 0)
      return;

    for (int i = 0; i != proto_stores.store_size(); ++i) {
      if (proto_stores.store(i).IsInitialized()) {
        std::string title(proto_stores.store(i).title());
        if (title != "local" && title != "cloud")
          return;
      } else {
        return;
      }
    }

    std::unique_lock<std::mutex> lock(mutex);
    std::cout << proto_stores.DebugString() << std::endl;
    std::chrono::duration<double> diff = std::chrono::steady_clock::now() - kStart;
    std::cout << "Password: " << password << std::endl;
    std::cout << "Total time: " << diff.count() << "s" << std::endl;
  } catch (...) {
    return;
  }
  Stop();
  throw 0;
}



// ============================== BRUTE FORCE =====================================================
namespace brute_force {

// clang-format off
const std::array<char, 62> kAlphabet = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
// clang-format on

void HandleLastChar(const crypto::CipherText& cipher_text, std::string& password, std::size_t index,
                    const std::string* const last) {
  auto itr = std::find(kAlphabet.begin(), kAlphabet.end(), password[index]);
  auto end_itr = (last == nullptr ? kAlphabet.end() :
                                    std::find(kAlphabet.begin(), kAlphabet.end(), last->at(index)));
  while (itr != end_itr) {
    password.back() = *itr++;
    if (asio_service.stopped())
      return;
    Login(cipher_text, password);
  }
  if (last) {
    password.back() = last->at(index);
    Login(cipher_text, password);
  }
  password.back() = kAlphabet[0];
}

void HandlePenultimateChar(const crypto::CipherText& cipher_text, std::string& password,
                           std::size_t index, const std::string* const last) {
  auto itr = std::find(kAlphabet.begin(), kAlphabet.end(), password[index]);
  auto end_itr = kAlphabet.end();
  task_count = std::distance(itr, end_itr);
  if (last) {
    end_itr = std::find(kAlphabet.begin(), kAlphabet.end(), last->at(index));
    task_count = std::distance(itr, end_itr) + 1;
  } else {
    task_count = std::distance(itr, end_itr);
  }
  while (itr != end_itr) {
    password[index] = *itr++;
    if (asio_service.stopped())
      return;
    asio_service.post([&, password, index]() mutable {
      HandleLastChar(cipher_text, password, index + 1, nullptr);
      std::size_t count = 0;
      {
        std::unique_lock<std::mutex> lock(mutex);
        count = --task_count;
      }
      if (count == 0)
        cond_var.notify_one();
    });
  }
  if (last) {
    password[index] = last->at(index);
    if (asio_service.stopped())
      return;
    asio_service.post([&, password, index]() mutable {
      HandleLastChar(cipher_text, password, index + 1, last);
      std::size_t count = 0;
      {
        std::unique_lock<std::mutex> lock(mutex);
        count = --task_count;
      }
      if (count == 0)
        cond_var.notify_one();
    });
  }
  {
    std::unique_lock<std::mutex> lock(mutex);
    cond_var.wait(lock, []() { return task_count == 0 || asio_service.stopped(); });
  }
  password[index] = password[index + 1] = kAlphabet[0];
}

void Update(const crypto::CipherText& cipher_text, std::string& password, std::size_t index,
            const std::string* const last) {
  if (asio_service.stopped())
    return;

  if (index == password.size() - 1)
    return HandleLastChar(cipher_text, password, index, last);

  if (index == password.size() - 2)
    return HandlePenultimateChar(cipher_text, password, index, last);

  if (index == password.size() - 3)
    std::cout << "Checking: " << password << std::endl;

  auto itr = std::find(kAlphabet.begin(), kAlphabet.end(), password[index]);
  auto end_itr = (last == nullptr ? kAlphabet.end() :
                                    std::find(kAlphabet.begin(), kAlphabet.end(), last->at(index)));
  while (itr != end_itr) {
    password[index] = *itr++;
    Update(cipher_text, password, index + 1, nullptr);
  }
  if (last) {
    password[index] = last->at(index);
    Update(cipher_text, password, index + 1, last);
  }
  password[index] = kAlphabet[0];
}

void Attack(const Args& args) {
  const auto cipher_text = ReadSurefile(args);

  // Validate `first` input comes before `last` input
  const auto& first = args.at("<first-word>").asString();
  const auto& last = args.at("<last-word>").asString();
  if (first.size() > last.size()) {
    std::cout << "\"" << first << "\" does not come before \"" << last << "\"\n";
    throw 2;
  } else if (first.size() == last.size()) {
    auto first_itr = first.begin();
    auto last_itr = last.begin();
    while (first_itr != first.end()) {
      if (*first_itr != *last_itr) {
        auto firsts_position_in_alphabet =
            std::find(kAlphabet.begin(), kAlphabet.end(), *first_itr);
        auto lasts_position_in_alphabet = std::find(kAlphabet.begin(), kAlphabet.end(), *last_itr);
        if (firsts_position_in_alphabet < lasts_position_in_alphabet)
          break;
        std::cout << "\"" << first << "\" does not come before \"" << last << "\"\n";
        throw 3;
      }
      ++first_itr;
      ++last_itr;
    }
  }
  std::cout << "Checking passwords from \"" << first << "\" to \"" << last << " against "
            << args.at("--surefile") << "\n";

  std::string password = first;
  for (;;) {
    auto iteration_start = std::chrono::steady_clock::now();
    if (password.size() == last.size()) {
      Update(cipher_text, password, 0, &last);
      Stop();
      std::chrono::duration<double> diff = std::chrono::steady_clock::now() - iteration_start;
      std::cout << "\nFailed to find password.  Took " << diff.count() << "s" << std::endl;
    } else {
      Update(cipher_text, password, 0, nullptr);
    }

    if (asio_service.stopped())
      break;
    std::chrono::duration<double> diff = std::chrono::steady_clock::now() - iteration_start;
    std::cout << "\nCompleted password length: " << password.size() << " took " << diff.count()
              << "s" << std::endl;
    password += kAlphabet[0];
  }
}

}  // namespace brute_force



// ============================== PASSWORD LIST ===================================================
namespace password_list {

void ReplaceAll(std::string& word, char to_find, char replacement) {
  std::replace(word.begin(), word.end(), to_find, replacement);
}

void ReplaceAllOs(std::string& word) {
  ReplaceAll(word, 'o', '0');
}

void ReplaceAllIs(std::string& word) {
  ReplaceAll(word, 'i', '1');
}

void ReplaceAllEs(std::string& word) {
  ReplaceAll(word, 'e', '3');
}

void ReplaceAllAs(std::string& word) {
  ReplaceAll(word, 'a', '4');
}

void ReplaceFirst(std::string& word, char to_find, char replacement) {
  auto position = word.find_first_of(to_find);
  if (position != std::string::npos)
    word[position] = replacement;
}

void ReplaceFirstO(std::string& word) {
  ReplaceFirst(word, 'o', '0');
}

void ReplaceFirstI(std::string& word) {
  ReplaceFirst(word, 'i', '1');
}

void ReplaceFirstE(std::string& word) {
  ReplaceFirst(word, 'e', '3');
}

void ReplaceFirstA(std::string& word) {
  ReplaceFirst(word, 'a', '4');
}

// See https://www.question-defense.com/2012/04/21/hashcat-best64-rule-details-updated-after-the-best64-challenge
void CheckPermutations(const crypto::CipherText& cipher_text, const std::string& password) {
  if (password.empty())
    return;

  // First four rules
  // ================
  //   do nothing
  std::unordered_set<std::string> set({ password });
  //   reverse each combination
  auto word = password;
  std::reverse(word.begin(), word.end());
  set.insert(word);
  //   all uppercase characters
  set.emplace(boost::to_upper_copy<std::string>(password));
  //   toggle the case of char in position 0
  word = password;
  word.front() = static_cast<char>(std::isupper(word.front()) ? std::tolower(word.front()) : std::toupper(word.front()));
  set.insert(word);

  // Append numbers
  // ==============
  //   append 0 to 9 to the end of each combination
  word = password + '0';
  Login(cipher_text, word);
  for (char c = 49; c < 58; ++c) {
    word.back() = c;
    Login(cipher_text, word);
  }

  // Append various number combinations
  // ==================================
  //   append 00 to the end of each combination
  word = password + "00";
  Login(cipher_text, word);
  auto last = word.rbegin();
  auto second_last = last;
  ++second_last;
  //   append 01 to the end of each combination
  *last = '1';
  Login(cipher_text, word);
  //   append 22 to the end of each combination
  *second_last = *last = '2';
  Login(cipher_text, word);
  //   append 21 to the end of each combination
  *last = '1';
  Login(cipher_text, word);
  //   append 23 to the end of each combination
  *last = '3';
  Login(cipher_text, word);
  //   append 69 to the end of each combination
  *second_last = '6';
  *last = '9';
  Login(cipher_text, word);
  //   append 77 to the end of each combination
  *second_last = *last = '7';
  Login(cipher_text, word);
  //   append 88 to the end of each combination
  *second_last = *last = '8';
  Login(cipher_text, word);
  //   append 99 to the end of each combination
  *second_last = *last = '9';
  Login(cipher_text, word);
  //   append 11 to the end of each combination
  *second_last = *last = '1';
  Login(cipher_text, word);
  //   append 12 to the end of each combination
  *last = '2';
  Login(cipher_text, word);
  //   append 123 to the end of each combination
  word += '3';
  Login(cipher_text, word);

  // High frequency append
  // =====================
  //   append "s" to the end of each combination
  Login(cipher_text, password + 's');

  // High frequency overwrite at end
  // ===============================
  //   delete last two chars of each combination and append an "a", "er" and "ie" at the end of each combination
  if (password.size() > 2) {
    word = password.substr(0, password.size() - 2);
    Login(cipher_text, word + 'a');
    set.emplace(word + "er");
    set.emplace(word + "ie");
  //   delete last three chars of each combination and append an "o", "y", "123", and "man" at the end of each combination
    if (password.size() > 3) {
      word.pop_back();
      Login(cipher_text, word + 'o');
      Login(cipher_text, word + 'y');
      set.emplace(word + "123");
      set.emplace(word + "man");
    }
  }

  // High frequency prepend
  // ======================
  //   add a "1" and "the" to the beginning of each combination
  Login(cipher_text, "1" + password);
  Login(cipher_text, "the" + password);

  // Leetify
  // =======
  //   replace all instances of "o" with "0", of "i" with "1", and "e" with "3"
  //   I'm extending this to do more permutations and also replacing "a" with "4" since that's a known substitution
  //   replacing all "o"
  auto word_o = password;
  ReplaceAllOs(word_o);
  set.insert(word_o);
  //   replacing all "i"
  auto word_i = password;
  ReplaceAllIs(word_i);
  set.insert(word_i);
  //   replacing all "e"
  auto word_e = password;
  ReplaceAllEs(word_e);
  set.insert(word_e);
  //   replacing all "a"
  auto word_a = password;
  ReplaceAllAs(word_a);
  set.insert(word_a);
  //   replacing all "o" and "i"
  auto word_o_i = word_o;
  ReplaceAllIs(word_o_i);
  set.insert(word_o_i);
  //   replacing all "o" and "e"
  auto word_o_e = word_o;
  ReplaceAllEs(word_o_e);
  set.insert(word_o_e);
  //   replacing all "o" and "a"
  auto word_o_a = word_o;
  ReplaceAllAs(word_o_a);
  set.insert(word_o_a);
  //   replacing all "i" and "e"
  auto word_i_e = word_i;
  ReplaceAllEs(word_i_e);
  set.insert(word_i_e);
  //   replacing all "i" and "a"
  auto word_i_a = word_i;
  ReplaceAllAs(word_i_a);
  set.insert(word_i_a);
  //   replacing all "e" and "a"
  auto word_e_a = word_e;
  ReplaceAllEs(word_e_a);
  set.insert(word_e_a);
  //   replacing all "o", "i" and "e"
  auto word_o_i_e = word_o_i;
  ReplaceAllEs(word_o_i_e);
  set.insert(word_o_i_e);
  //   replacing all "o", "i" and "a"
  auto word_o_i_a = word_o_i;
  ReplaceAllAs(word_o_i_a);
  set.insert(word_o_i_a);
  //   replacing all "o", "e" and "a"
  auto word_o_e_a = word_o_e;
  ReplaceAllAs(word_o_e_a);
  set.insert(word_o_e_a);
  //   replacing all "i", "e" and "a"
  auto word_i_e_a = word_i_e;
  ReplaceAllAs(word_i_e_a);
  set.insert(word_i_e_a);
  //   replacing all "o", "i", "e" and "a"
  ReplaceAllOs(word_i_e_a);
  set.insert(word_i_e_a);
  //   replacing first "o"
  word_o = password;
  ReplaceFirstO(word_o);
  set.insert(word_o);
  //   replacing first "i"
  word_i = password;
  ReplaceFirstI(word_i);
  set.insert(word_i);
  //   replacing first "e"
  word_e = password;
  ReplaceFirstE(word_e);
  set.insert(word_e);
  //   replacing first "a"
  word_a = password;
  ReplaceFirstA(word_a);
  set.insert(word_a);
  //   replacing first "o" and "i"
  word_o_i = word_o;
  ReplaceFirstI(word_o_i);
  set.insert(word_o_i);
  //   replacing first "o" and "e"
  word_o_e = word_o;
  ReplaceFirstE(word_o_e);
  set.insert(word_o_e);
  //   replacing first "o" and "a"
  word_o_a = word_o;
  ReplaceFirstA(word_o_a);
  set.insert(word_o_a);
  //   replacing first "i" and "e"
  word_i_e = word_i;
  ReplaceFirstE(word_i_e);
  set.insert(word_i_e);
  //   replacing first "i" and "a"
  word_i_a = word_i;
  ReplaceFirstA(word_i_a);
  set.insert(word_i_a);
  //   replacing first "e" and "a"
  word_e_a = word_e;
  ReplaceFirstE(word_e_a);
  set.insert(word_e_a);
  //   replacing first "o", "i" and "e"
  word_o_i_e = word_o_i;
  ReplaceFirstE(word_o_i_e);
  set.insert(word_o_i_e);
  //   replacing first "o", "i" and "a"
  word_o_i_a = word_o_i;
  ReplaceFirstA(word_o_i_a);
  set.insert(word_o_i_a);
  //   replacing first "o", "e" and "a"
  word_o_e_a = word_o_e;
  ReplaceFirstA(word_o_e_a);
  set.insert(word_o_e_a);
  //   replacing first "i", "e" and "a"
  word_i_e_a = word_i_e;
  ReplaceFirstA(word_i_e_a);
  set.insert(word_i_e_a);
  //   replacing first "o", "i", "e" and "a"
  ReplaceFirstO(word_i_e_a);
  set.insert(word_i_e_a);

  // Simple extracts
  // ===============
  //   delete the fourth char
  word = password;
  try {
    word.erase(3, 1);
    Login(cipher_text, word);
    //   delete the third and the fourth char
    word.erase(2, 1);
    Login(cipher_text, word);
  } catch (...) {}
  //   delete the fifth char
  word = password;
  try {
    word.erase(4, 1);
    Login(cipher_text, word);
  } catch (...) {}

  // Undouble word
  // =============
  //   truncate the combination at 6 chars and then append "1"
  if (password.size() > 6) {
    word = password.substr(0, 6) + '1';
    Login(cipher_text, word);
  }

  // Removes suffixes from 'strongified' passwords in dict
  // =====================================================
  //   remove the last char
  if (password.size() > 1)
    Login(cipher_text, password.substr(0, password.size() - 1));
  //   remove the last two chars
  if (password.size() > 2)
    Login(cipher_text, password.substr(0, password.size() - 2));
  //   remove the last three chars
  if (password.size() > 3) {
    word = password.substr(0, password.size() - 3);
    Login(cipher_text, word);
  //   remove the last three chars and then duplicate the remaining word
    Login(cipher_text, word + word);
  }
  //   remove the last two chars, delete the second char, and then remove the last char
  if (password.size() > 4) {
    word = password.substr(0, password.size() - 3);
    word.erase(1, 1);
    Login(cipher_text, word);
  }

  // Rotates
  // =======
  //   increment the sixth char by one ascii value, delete the last char, move the last char to the first position four times, and cut the word off at the fifth char
  //   delete the first two chars and then rotate the first char to the end of the combination six times
  //   rotate the last char to the beginning, delete the last two chars, and then rotate the first char to the end of the combination
  //   rotate the last char to the beginning of the combination two times, lower the ascii value of the first char, and then delete the third and fourth char
  //   rotate the last char to the beginning four times and truncate the combination to five chars
  //   rotate the last char to the beginning five times and then truncate the combination at six chars
  //   rotate the last char to the beginning 6 times, duplicate the last four chars at the end of the combination, truncate the combination at 5 chars, and then duplicate the remaining combination

  // Unknown
  // =======
  //   swap the first char with the fifth char, increment the ascii value of the first char by one, and truncate the combination at 5 chars
  //   swap the first char with the sixth char, delete the fourth char, duplicate the remaining combination, truncate the combination at four chars, and then duplicate the combination once
  //   append 00000000
  Login(cipher_text, password + "00000000");
  //   duplicate the last char four times, truncate the combination at 9 chars, and then delete the third, fourth, fifth, and sixth chars
  //   duplicate the last char five times, swap the eighth and sixth char, truncate the combination at six chars, move the first char to the end of the combination, and then delete the first two chars
  //   duplicate the entire combination, delete the ninth and tenth chars, duplicate the last four chars, truncate the word at the fifth char, and then duplicate the remaining combination
  //   duplicate the combination reversed, replace the sixth char with the eleventh char, truncate the combination at the ninth char, and delete the fifth char
  //   duplicate the combination twice, truncate the combination at the eight letter, duplicate the combination, and then delete the ninth, tenth, eleventh, twelfth and thirteenth chars
  //   delete the fifth char, duplicate the combination, duplicate the new combination twice, and truncate the combination after the seventh char

  for (const auto& entry: set)
    Login(cipher_text, entry);
}

void Check(crypto::CipherText cipher_text, std::vector<std::string> passwords,
           std::uint64_t count) {
  if (asio_service.stopped())
    return;
  on_scope_exit strong_guarantee([] {
    {
      std::unique_lock<std::mutex> lock(mutex);
      --task_count;
    }
    cond_var.notify_one();
  });

  {
    std::unique_lock<std::mutex> lock(mutex);
    ++task_count;
    std::cout << count << ": " << passwords.back() << std::endl;
  }

  for (const auto& password : passwords) {
    if (asio_service.stopped())
      return;
    CheckPermutations(cipher_text, password);
  }
}

void Attack(const Args& args) {
  const auto cipher_text = ReadSurefile(args);
  if (!fs::is_regular_file(args.at("--password-file").asString())) {
    std::cout << "Failed to find file at " << args.at("--password-file") << std::endl;
    throw 10;
  }

  std::cout << "Checking passwords from " << args.at("--password-file") << " against "
            << args.at("--surefile") << "\n";

  std::ifstream infile(args.at("--password-file").asString(), std::ios::binary);
  std::string line;
  std::uint64_t count = 0;
  const std::size_t kBatchSize = 10000;
  for (;;) {
    std::vector<std::string> passwords;
    passwords.reserve(kBatchSize);
    while (std::getline(infile, line, '\n')) {
      passwords.emplace_back(line);
      if (++count % kBatchSize == 0)
        break;
    }
    asio_service.post([=]() { Check(cipher_text, passwords, count); });
    {
      std::unique_lock<std::mutex> lock(mutex);
      cond_var.wait(lock, []() {
        return task_count < std::thread::hardware_concurrency() || asio_service.stopped();
      });
    }
    if (asio_service.stopped())
      break;
  }
}

}  // namespace password_list

}  // namespace maidsafe


int main(int argc, const char* argv[]) {
  Args args = docopt::docopt(maidsafe::kUsage, {argv + 1, argv + argc});

  // Start workers for Asio service
  std::vector<std::thread> workers;
  for (unsigned i = 0; i < std::thread::hardware_concurrency(); ++i) {
    workers.emplace_back([&] {
      try {
        maidsafe::asio_service.run();
      } catch (int) {
        maidsafe::Stop();
      }
    });
  }

  int exit_code = 0;
  try {
    if (args.at("<first-word>")) {
      maidsafe::brute_force::Attack(args);
    } else {
      maidsafe::password_list::Attack(args);
    }
  } catch (int error_value) {
    maidsafe::Stop();
    exit_code = error_value;
  }

  for (auto& worker : workers)
    worker.join();

  return exit_code;
}
