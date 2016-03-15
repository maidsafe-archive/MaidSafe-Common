#include <array>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <chrono>
#include <future>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "boost/asio/io_service.hpp"
#include "boost/filesystem.hpp"

#include "maidsafe/common/application_support_directories.h"
#include "maidsafe/common/crypto.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/common/stores.pb.h"

namespace fs = boost::filesystem;
namespace po = boost::program_options;

namespace maidsafe {

const std::array<char, 62> kAlphabet = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
const std::chrono::time_point<std::chrono::steady_clock> kStart = std::chrono::steady_clock::now();
std::condition_variable cond_var;
std::mutex mutex;
std::size_t task_count = 0;

crypto::PlainText Decrypt(const crypto::CipherText& cipher_text, const std::string& password) {
  crypto::SecurePassword secure_password(crypto::Hash<crypto::SHA512>(password));
  crypto::AES256Key key(secure_password.string().substr(0, crypto::AES256_KeySize));
  crypto::AES256InitialisationVector iv(
      secure_password.string().substr(crypto::AES256_KeySize, crypto::AES256_IVSize));
  return crypto::SymmDecrypt(cipher_text, key, iv);
}

void Login(const crypto::CipherText& cipher_text, const std::string& password) {
  const auto plain_text = Decrypt(cipher_text, password);
  try {
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

    std::cout << proto_stores.DebugString() << std::endl;
    std::chrono::duration<double> diff = std::chrono::steady_clock::now() - kStart;
    std::cout << "Password: " << password << std::endl;
    std::cout << "Total time: " << diff.count() << "s" << std::endl;
  } catch (...) {
    return;
  }
  throw 0;
}

void HandleLastChar(boost::asio::io_service& asio_service, const crypto::CipherText& cipher_text,
                    std::string& password, std::size_t index, const std::string* const last) {
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

void HandlePenultimateChar(boost::asio::io_service& asio_service,
                           const crypto::CipherText& cipher_text, std::string& password,
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
      HandleLastChar(asio_service, cipher_text, password, index + 1, nullptr);
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
      HandleLastChar(asio_service, cipher_text, password, index + 1, last);
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
    cond_var.wait(lock, [&asio_service]() { return task_count == 0 || asio_service.stopped(); });
  }
  password[index] = password[index + 1] = kAlphabet[0];
}

void Update(boost::asio::io_service& asio_service, const crypto::CipherText& cipher_text,
            std::string& password, std::size_t index, const std::string* const last) {
  if (asio_service.stopped())
    return;

  if (index == password.size() - 1)
    return HandleLastChar(asio_service, cipher_text, password, index, last);

  if (index == password.size() - 2)
    return HandlePenultimateChar(asio_service, cipher_text, password, index, last);

  if (index == password.size() - 3)
    std::cout << "Checking: " << password << std::endl;

  auto itr = std::find(kAlphabet.begin(), kAlphabet.end(), password[index]);
  auto end_itr = (last == nullptr ? kAlphabet.end() :
                                    std::find(kAlphabet.begin(), kAlphabet.end(), last->at(index)));
  while (itr != end_itr) {
    password[index] = *itr++;
    Update(asio_service, cipher_text, password, index + 1, nullptr);
  }
  if (last) {
    password[index] = last->at(index);
    Update(asio_service, cipher_text, password, index + 1, last);
  }
  password[index] = kAlphabet[0];
}

}  // namespace maidsafe


int main(int argc, char* argv[]) {
  // Declare the supported options.
  po::options_description description("Allowed options");
  std::string first, last, surefile_path = (fs::current_path() / "surefile").string();
  description.add_options()
      ("first,f", po::value<std::string>(&first)->default_value("0"), "Initial password to try")
      ("last,l", po::value<std::string>(&last)->default_value("ZZZZZZZZZZ"), "Last password to try")
      ("path,p", po::value<std::string>(&surefile_path)->default_value(surefile_path), "Path to surefile")
      ("help,h", "Display this help message");

  po::variables_map variables_map;
  po::store(po::parse_command_line(argc, argv, description), variables_map);
  po::notify(variables_map);

  if (variables_map.count("help")) {
    std::cout << description << "\n";
    return 1;
  }

  // Validate `first` input comes before `last` input
  if (first.size() > last.size()) {
    std::cout << "\"" << first << "\" does not come before \"" << last << "\"\n";
    return 2;
  } else if (first.size() == last.size()) {
    auto first_itr = first.begin();
    auto last_itr = last.begin();
    while (first_itr != first.end()) {
      if (*first_itr != *last_itr) {
        auto firsts_position_in_alphabet =
            std::find(maidsafe::kAlphabet.begin(), maidsafe::kAlphabet.end(), *first_itr);
        auto lasts_position_in_alphabet =
            std::find(maidsafe::kAlphabet.begin(), maidsafe::kAlphabet.end(), *last_itr);
        if (firsts_position_in_alphabet < lasts_position_in_alphabet)
          break;
        std::cout << "\"" << first << "\" does not come before \"" << last << "\"\n";
        return 3;
      }
      ++first_itr;
      ++last_itr;
    }
  }
  std::cout << "Checking passwords from \"" << first << "\" to \"" << last << " from file \""
            << surefile_path << "\"\n";

  auto cipher_text = maidsafe::crypto::CipherText(" ");
  try {
    cipher_text = maidsafe::ReadFile(surefile_path);
  } catch (const std::exception& e) {
    std::cout << "Failed to parse \"" << surefile_path << "\": " << e.what() << std::endl;
    return 4;
  }
  //assert(maidsafe::HexEncode(maidsafe::crypto::Hash<maidsafe::crypto::SHA512>(cipher_text)) ==
  //  "31f6aa36890eddda04c7b1d095470defc979151e8f76a7878afa298c458ae2f826f98026dadf85e080dd352da3f16259f5c02e59684181b2dc0083224fe06d12");

  // Start workers for Asio service
  boost::asio::io_service asio_service;
  std::unique_ptr<boost::asio::io_service::work> work(
      new boost::asio::io_service::work(asio_service));
  auto stop = [&]() {
    work.reset();
    asio_service.stop();
    maidsafe::cond_var.notify_one();
  };
  std::vector<std::thread> workers;
  for (unsigned i = 0; i < std::thread::hardware_concurrency(); ++i) {
    workers.emplace_back([&] {
      try {
        asio_service.run();
      } catch (int) {
        stop();
      }
    });
  }

  std::string password = first;
  for (;;) {
    auto iteration_start = std::chrono::steady_clock::now();
    if (password.size() == last.size()) {
      maidsafe::Update(asio_service, cipher_text, password, 0, &last);
      stop();
      std::chrono::duration<double> diff = std::chrono::steady_clock::now() - iteration_start;
      std::cout << "\nFailed to find password.  Took " << diff.count() << "s" << std::endl;
    } else {
      maidsafe::Update(asio_service, cipher_text, password, 0, nullptr);
    }

    if (asio_service.stopped())
      break;
    std::chrono::duration<double> diff = std::chrono::steady_clock::now() - iteration_start;
    std::cout << "\nCompleted password length: " << password.size() << " took " << diff.count()
              << "s" << std::endl;
    password += maidsafe::kAlphabet[0];
  }

  for (auto& worker : workers)
    worker.join();

  return 0;
}
