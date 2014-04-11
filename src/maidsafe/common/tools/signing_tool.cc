/*  Copyright 2012 MaidSafe.net limited

    This MaidSafe Software is licensed to you under (1) the MaidSafe.net Commercial License,
    version 1.0 or later, or (2) The General Public License (GPL), version 3, depending on which
    licence you accepted on initial access to the Software (the "Licences").

    By contributing code to the MaidSafe Software, or to this project generally, you agree to be
    bound by the terms of the MaidSafe Contributor Agreement, version 1.0, found in the root
    directory of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also
    available at: http://www.maidsafe.net/licenses

    Unless required by applicable law or agreed to in writing, the MaidSafe Software distributed
    under the GPL Licence is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
    OF ANY KIND, either express or implied.

    See the Licences for the specific language governing permissions and limitations relating to
    use of the MaidSafe Software.                                                                 */

#if defined MAIDSAFE_WIN32
#include <windows.h>
#else
#include <unistd.h>
#if defined MAIDSAFE_LINUX
#include <termio.h>
#elif defined MAIDSAFE_APPLE
#include <termios.h>
#endif
#endif

#include <cstdint>
#include <fstream>
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <vector>
#include "boost/filesystem.hpp"
#include "maidsafe/common/utils.h"
#include "maidsafe/common/crypto.h"
#include "maidsafe/common/rsa.h"

namespace fs = boost::filesystem;

static std::string prompt(">> ");
static maidsafe::asymm::Keys Keys;
static bool have_private_key(false);
static bool have_public_key(false);
static bool group_signed_in;

template <class T>
T Get(std::string display_message, bool echo_input = true);

void Echo(bool enable = true) {
#ifdef WIN32
  HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
  DWORD mode;
  GetConsoleMode(hStdin, &mode);

  if (!enable)
    mode &= ~ENABLE_ECHO_INPUT;
  else
    mode |= ENABLE_ECHO_INPUT;

  SetConsoleMode(hStdin, mode);
#else
  struct termios tty;
  tcgetattr(STDIN_FILENO, &tty);
  if (!enable)
    tty.c_lflag &= ~ECHO;
  else
    tty.c_lflag |= ECHO;

  (void)tcsetattr(STDIN_FILENO, TCSANOW, &tty);
#endif
}

std::string GetPasswd(bool repeat = true) {
  std::string passwd("r"), passwd2("s");
  do {
    passwd = Get<std::string>("please Enter passwd \n", false);
    if (repeat)
      passwd2 = Get<std::string>("please Re-Enter same passwd \n", false);
  } while ((passwd != passwd2) && (repeat));
  return maidsafe::crypto::Hash<maidsafe::crypto::SHA512>(passwd).string();
}

std::vector<std::string> TokeniseLine(std::string line) {
  std::vector<std::string> args;
  line = std::string("--") + line;
  boost::char_separator<char> sep(" ");
  boost::tokenizer<boost::char_separator<char>> tokens(line, sep);
  for (const auto& t : tokens)  // NOLINT (Fraser)
    args.push_back(t);
  return args;
}

void CreateKeys() {
  std::cout << "Creating keys \nPlease wait !!\n";
  Keys = maidsafe::asymm::GenerateKeyPair();
  have_public_key = true;
  have_private_key = true;
  std::cout << "Creating keys sucessful\n";
}

void SavePrivateKey() {
  if (group_signed_in)
    return;
  std::string filename = Get<std::string>("please enter filename to save the private key to\n");
  if (!have_private_key) {
    std::cout << "You have not loaded or created a Private Key\nAborting!\n";
  } else {
    fs::path file(filename);
    std::string priv_key(maidsafe::asymm::EncodeKey(Keys.private_key).string());
    if (!maidsafe::WriteFile(file, priv_key))
      std::cout << "error writing file\n";
    else
      std::cout << "Stored private key in " << filename << "\n";
  }
}

void SavePublicKey() {
  std::string filename = Get<std::string>("please enter filename to save the public key to\n");
  if (!have_public_key) {
    std::cout << "You have not loaded or created a Public Key\nAborting!\n";
  } else {
    fs::path file(filename);
    std::string pub_key(maidsafe::asymm::EncodeKey(Keys.public_key).string());
    if (!maidsafe::WriteFile(file, pub_key))
      std::cout << "error writing file\n";
    else
      std::cout << "Stored public key in " << filename << "\n";
  }
}

void LoadPrivateKey() {
  std::string filename = Get<std::string>("please enter filename to load private key from\n");
  fs::path file(filename);
  std::string priv_key;
  if (!maidsafe::ReadFile(file, &priv_key)) {
    std::cout << "error reading file\n";
    return;
  }
  Keys.private_key = maidsafe::asymm::DecodeKey(maidsafe::asymm::EncodedPrivateKey(priv_key));

  if (maidsafe::asymm::ValidateKey(Keys.private_key))
    std::cout << "private key loaded and valid \n";
  else
    std::cout << "private key invalid !! \n";
}

void LoadPublicKey() {
  std::string filename = Get<std::string>("please enter filename to load public key from\n");
  fs::path file(filename);
  std::string pub_key;
  if (!maidsafe::ReadFile(file, &pub_key)) {
    std::cout << "error reading file\n";
    return;
  }
  std::cout << maidsafe::HexEncode(pub_key) << "\n";
  Keys.public_key = maidsafe::asymm::DecodeKey(maidsafe::asymm::EncodedPublicKey(pub_key));

  if (maidsafe::asymm::ValidateKey(Keys.public_key))
    std::cout << "public key loaded and valid \n";
  else
    std::cout << "public key invalid !! \n";
}

void SignFile() {
  std::string filename = Get<std::string>("please enter filename to sign");
  fs::path file(filename);
  if (!maidsafe::asymm::ValidateKey(Keys.private_key)) {
    std::cout << "private key invalid, aborting!!\n";
  }

  maidsafe::asymm::Signature signature(maidsafe::asymm::SignFile(file, Keys.private_key));
  fs::path sigfile(filename + ".sig");
  if (!maidsafe::WriteFile(sigfile, signature.string()))
    std::cout << "error writing file\n";
  else
    std::cout << "Stored signature in " << sigfile << "\n";
}

void ValidateSignature() {
  std::string filename = Get<std::string>(
      "please enter filename to validate \n We will read the "
      "filename.sig as signature file\n");
  fs::path file(filename);
  fs::path sigfile(filename + ".sig");

  std::string signature;
  if (!maidsafe::ReadFile(sigfile, &signature)) {
    std::cout << "error reading file\n";
    return;
  }
  if (!maidsafe::asymm::ValidateKey(Keys.public_key)) {
    std::cout << "public key invalid, aborting!!\n";
  }

  if (maidsafe::asymm::CheckFileSignature(file, maidsafe::asymm::Signature(signature),
                                          Keys.public_key)) {
    std::cout << "Signature valid\n";
  } else {
    std::cout << "Invalid signature !! \n";
  }
}

void EncryptFile() {
  std::string filename = Get<std::string>("please enter filename to encrypt");
  fs::path file(filename);
  std::string passwd = GetPasswd();
  maidsafe::crypto::AES256Key key(passwd.substr(0, maidsafe::crypto::AES256_KeySize));
  maidsafe::crypto::AES256InitialisationVector iv(
      passwd.substr(maidsafe::crypto::AES256_KeySize, maidsafe::crypto::AES256_IVSize));

  std::string data;
  if (!maidsafe::ReadFile(file, &data)) {
    std::cout << "error reading file\n";
    return;
  }

  if (!maidsafe::WriteFile(file, maidsafe::crypto::SymmEncrypt(maidsafe::crypto::PlainText(data),
                                                               key, iv)->string()))
    std::cout << "error writing file\n";
  else
    std::cout << "File is now encrypted " << filename << "\n";
}

void DecryptFile() {
  std::string filename = Get<std::string>("please enter filename to decrypt");
  fs::path file(filename);
  std::string passwd = GetPasswd();
  maidsafe::crypto::AES256Key key(passwd.substr(0, maidsafe::crypto::AES256_KeySize));
  maidsafe::crypto::AES256InitialisationVector iv(
      passwd.substr(maidsafe::crypto::AES256_KeySize, maidsafe::crypto::AES256_IVSize));

  std::string data;
  if (!maidsafe::ReadFile(file, &data)) {
    std::cout << "error reading file\n";
    return;
  }
  if (!maidsafe::WriteFile(file, maidsafe::crypto::SymmDecrypt(maidsafe::crypto::CipherText(
                                                               maidsafe::NonEmptyString(data)), key,
                                                               iv).string()))
    std::cout << "error writing file\n";
  else
    std::cout << "File is now decrypted " << filename << "\n";
}

void CreateKeyGroup() {
  int32_t max = Get<int32_t>("please Enter total number of people \n");
  int32_t min = Get<int32_t>("please Enter number of people required to sign\n");
  if (max < min) {
    std::cout << "required must be smaller or equal to total\n";
    return;
  }
  if (min < 2) {
    std::cout << "smallest required group is 2";
    return;
  }
  std::string location = Get<std::string>("please enter location of files");
  if (!have_private_key) {
    std::cout << " No Private key found, creating now\n";
    CreateKeys();
    std::cout << " You can still load another private key from disk if you wish\n";
  }

  // create the chunks of the private key.
  std::string priv_key(maidsafe::asymm::EncodeKey(Keys.private_key).string());
  std::vector<std::string> chunks(maidsafe::crypto::SecretShareData(min, max, priv_key));

  std::map<std::string, std::string> users;
  std::pair<std::map<std::string, std::string>::iterator, bool> ret;
  for (int i = 0; i < max; ++i) {
    std::string name;
    std::cout << "please Enter unique name \n";
    std::getline(std::cin, name);
    std::string passwd = GetPasswd();
    if (i < (max - 1))
      std::cout << "Password Successfull next person please\n ==================================\n";
    ret = users.insert(std::pair<std::string, std::string>(name, passwd));
    if (!ret.second) {
      std::cout << "Error, are you sure you used a unique name, retry !\n";
      --i;
    } else {
      maidsafe::crypto::AES256Key key(passwd.substr(0, maidsafe::crypto::AES256_KeySize));
      maidsafe::crypto::AES256InitialisationVector iv(
          passwd.substr(maidsafe::crypto::AES256_KeySize, maidsafe::crypto::AES256_IVSize));
      fs::path file(location + name + ".keyfile");
      if (!maidsafe::WriteFile(
               file, maidsafe::crypto::SymmEncrypt(maidsafe::crypto::PlainText(chunks.at(i)), key,
                                                   iv)->string())) {
        std::cout << "error writing file\n";
        --i;
        std::cout << "Error, are you sure you used a unique name, retry !\n";
      } else {
        std::cout << "File is now encrypted and saved as " << file.c_str() << "\n"
                  << "for " << name << "\n";
      }
    }
  }
  SavePublicKey();
}

void GroupSignIn() {
  std::string total;
  std::string quorum;
  std::cout << "please Enter number of people required to sign\n";
  std::getline(std::cin, quorum);
  int min = atoi(quorum.c_str());
  std::vector<std::string> chunks;
  std::string enc_data;
  std::string priv_key;
  std::string location = Get<std::string>("please enter location of files");

  for (int i = 0; i < min; ++i) {
    enc_data.clear();
    std::string name = Get<std::string>("please Enter name \n");
    std::string passwd = GetPasswd(false);
    std::cout << "Password captured next person please\n ==================================\n";

    maidsafe::crypto::AES256Key key(passwd.substr(0, maidsafe::crypto::AES256_KeySize));
    maidsafe::crypto::AES256InitialisationVector iv(
        passwd.substr(maidsafe::crypto::AES256_KeySize, maidsafe::crypto::AES256_IVSize));
    fs::path file(location + name + ".keyfile");
    if (!maidsafe::ReadFile(file, &enc_data)) {
      std::cout << "error reading file\n";
      --i;
      std::cout << "Error, are you sure you used a correct name/password, retry !\n";
    } else {
      chunks.push_back(
          maidsafe::crypto::SymmDecrypt(maidsafe::crypto::CipherText(
              maidsafe::NonEmptyString(enc_data)), key, iv).string());
      enc_data.clear();
    }
  }
  priv_key = maidsafe::crypto::SecretRecoverData(min, chunks);
  Keys.private_key = maidsafe::asymm::DecodeKey(maidsafe::asymm::EncodedPrivateKey(priv_key));

  if (maidsafe::asymm::ValidateKey(Keys.private_key)) {
    std::cout << "private key loaded and valid \n";
    group_signed_in = true;
  } else {
    std::cout << "private key invalid !! \n";
  }
}

void Exit() { exit(0); }

void Help() {
  std::cout << "\t\tMaidSafe Encryption Tool \n"
            << "_________________________________________________________________\n"
            << "1:  CreateKeys   \t \t Creates an RSA keypair (2048)\t |\n";
  if (!group_signed_in)
    std::cout << "2:  SavePrivateKey \t\t Stores private key to file  \t |\n";
  std::cout << "3:  SavePublicKey \t\t Stores public key to file    \t |\n"
            << "4:  LoadPrivateKey \t\t Retrieve private key from file\t |\n"
            << "5:  LoadPublicKey \t\t Retrieve public key from file \t |\n"
            << "6:  CreateKeyGroup \t\t Group to manage keys (n+p )   \t |\n"
            << "7:  GroupSignIn    \t\t Sign in and load private key  \t |\n"
            << "8:  SignFile  \t\t\t Sign a file                  \t |\n"
            << "9:  ValidateSignature \t\t Validate signature of file \t |\n"
            << "10: EncryptFile  \t\t Encrypt (AES256) a file       \t |\n"
            << "11: DecryptFile  \t\t Decrypt (AES256) a file       \t |\n"
            << "_________________________________________________________________|\n"
            << "0:  Exit the system;";
}

void Process(int command) {
  switch (command) {
    case 0:
      Exit();
      break;
    case 1:
      CreateKeys();
      break;
    case 2:
      SavePrivateKey();
      break;
    case 3:
      SavePublicKey();
      break;
    case 4:
      LoadPrivateKey();
      break;
    case 5:
      LoadPublicKey();
      break;
    case 6:
      CreateKeyGroup();
      break;
    case 7:
      GroupSignIn();
      break;
    case 8:
      SignFile();
      break;
    case 9:
      ValidateSignature();
      break;
    case 10:
      EncryptFile();
      break;
    case 11:
      DecryptFile();
      break;
    default:
      std::cout << "unknown option \n";
      std::cout << prompt << std::flush;
      Help();
  }
}

#if defined MAIDSAFE_WIN32
#pragma warning(push)
#pragma warning(disable : 4701)
#endif
template <class T>
T Get(std::string display_message, bool echo_input) {
  Echo(echo_input);
  std::cout << display_message << "\n";
  std::cout << prompt << std::flush;
  T command;
  std::string input;
  while (std::getline(std::cin, input, '\n')) {
    std::cout << prompt << std::flush;
    if (std::stringstream(input) >> command) {
      Echo(true);
      return command;
    } else {
      Echo(true);
      std::cout << "invalid option\n";
      std::cout << prompt << std::flush;
    }
  }
  return command;
}
#if defined MAIDSAFE_WIN32
#pragma warning(pop)
#endif

int main() {
  for (;;) {
    Echo(true);
    std::cout << "_________________________________________________________________\n";
    Help();
    Process(Get<int>("", true));
    std::cout << "_________________________________________________________________\n";
  }
}
