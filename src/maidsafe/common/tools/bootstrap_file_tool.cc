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

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <ios>
#include <iostream>
#include <limits>
#include <string>

#include "boost/asio/ip/udp.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"

#include "maidsafe/common/log.h"
#include "maidsafe/common/utils.h"

#include "maidsafe/common/serialisation/serialisation.h"
#include "maidsafe/common/tools/bootstrap_file_tool_cereal.h"

namespace fs = boost::filesystem;

bool g_running(true), g_out_of_date(false);
std::vector<boost::asio::ip::udp::endpoint> g_bootstrap_endpoints;

bool ConfirmChoice(bool exiting) {
  TLOG(kYellow) << "\nThe loaded bootstrap endpoints have changed since last saved.\n"
                << "Do you really wish to " << (exiting ? "exit" : "overwrite these from file")
                << "? (enter \"y\" to confirm) >>";
  TLOG(kDefaultColour) << " ";
  std::string input;
  std::getline(std::cin, input);
  return input == "y" || input == "Y";
}

// Host class
template <int command_index, typename MessagePolicy, typename InputPolicy, typename HandlePolicy>
class Choice : private MessagePolicy, private InputPolicy, private HandlePolicy {
 public:
  enum { index = command_index };
  static void PrintCommandPreamble() { MessagePolicy::PrintCommandPreamble(command_index); }
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
  void Execute() const {
    MessagePolicy::PrintMessage();
    typename InputPolicy::InputType input;
    for (;;) {
      std::string error_info;
      if (InputPolicy::GetInput(input, error_info))
        break;
      MessagePolicy::PrintErrorMessage(error_info);
    }
    HandlePolicy::HandleInput(input);
  }
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
};

// Message Policies
template <bool loading_file>
class MessagePolicyGetPath {
 protected:
  virtual ~MessagePolicyGetPath() {}
  static void PrintCommandPreamble(int index) {
    TLOG(kDefaultColour) << "Enter " << index
                         << (loading_file ? " to load an existing" : " to save to a")
                         << " bootstrap file\n";
  }
  void PrintMessage() const { TLOG(kDefaultColour) << "Enter path to bootstrap file >> "; }
  void PrintErrorMessage(const std::string& error_info) const {
    if (!error_info.empty())
      TLOG(kRed) << '\n' << error_info << "\n\n";
    PrintMessage();
  }
};

class MessagePolicyPrependEndpoint {
 protected:
  virtual ~MessagePolicyPrependEndpoint() {}
  static void PrintCommandPreamble(int index) {
    TLOG(kDefaultColour) << "Enter " << index << " to prepend a bootstrap endpoint\n";
  }
  void PrintMessage() const {
    TLOG(kDefaultColour) << "Enter endpoint to be prepended using format x.x.x.x:port >> ";
  }
  void PrintErrorMessage(const std::string& error_info) const {
    if (!error_info.empty())
      TLOG(kRed) << '\n' << error_info << "\n\n";
    PrintMessage();
  }
};

class MessagePolicyAppendEndpoint {
 protected:
  virtual ~MessagePolicyAppendEndpoint() {}
  static void PrintCommandPreamble(int index) {
    TLOG(kDefaultColour) << "Enter " << index << " to append a bootstrap endpoint\n";
  }
  void PrintMessage() const {
    TLOG(kDefaultColour) << "Enter endpoint to be appended using format x.x.x.x:port >> ";
  }
  void PrintErrorMessage(const std::string& error_info) const {
    if (!error_info.empty())
      TLOG(kRed) << '\n' << error_info << "\n\n";
    PrintMessage();
  }
};

class MessagePolicyRemoveEndpoint {
 protected:
  virtual ~MessagePolicyRemoveEndpoint() {}
  static void PrintCommandPreamble(int index) {
    TLOG(kDefaultColour) << "Enter " << index << " to remove a bootstrap endpoint\n";
  }
  void PrintMessage() const {
    TLOG(kDefaultColour) << "Enter endpoint to be removed using format x.x.x.x:port >> ";
  }
  void PrintErrorMessage(const std::string& error_info) const {
    if (!error_info.empty())
      TLOG(kRed) << '\n' << error_info << "\n\n";
    PrintMessage();
  }
};

class MessagePolicyViewEndpoints {
 protected:
  virtual ~MessagePolicyViewEndpoints() {}
  static void PrintCommandPreamble(int index) {
    TLOG(kDefaultColour) << "Enter " << index << " to view currently loaded bootstrap endpoints\n";
  }
  void PrintMessage() const {}
  void PrintErrorMessage(const std::string&) const {}
};

class MessagePolicyExit {
 protected:
  virtual ~MessagePolicyExit() {}
  static void PrintCommandPreamble(int index) {
    TLOG(kDefaultColour) << "Enter " << index << " to exit\n";
  }
  void PrintMessage() const {}
  void PrintErrorMessage(const std::string&) const {}
};

// Input Policies
class InputPolicyNull {
 protected:
  virtual ~InputPolicyNull() {}
  typedef int InputType;
  bool GetInput(InputType&, std::string&) const { return true; }
};

template <bool loading_file>
class InputPolicyGetPath {
 protected:
  virtual ~InputPolicyGetPath() {}
  typedef fs::path InputType;
  bool GetInput(InputType& bootstrap_path, std::string& error_info) const {
    error_info.clear();
    std::string entered_path;
    std::getline(std::cin, entered_path);
    boost::system::error_code error_code;
    bootstrap_path = fs::path(entered_path);
    if (!fs::exists(bootstrap_path.parent_path(), error_code)) {
      error_info = "Parent path of " + bootstrap_path.string() + " doesn't exist.";
      return false;
    }
#ifdef MAIDSAFE_WIN32
#pragma warning(push)
#pragma warning(disable : 4127)
#endif
    if (loading_file) {
      if (!fs::exists(bootstrap_path, error_code)) {
        error_info = bootstrap_path.string() + " doesn't exist.";
        return false;
      }
      if (!fs::is_regular_file(bootstrap_path, error_code)) {
        error_info = bootstrap_path.string() + " isn't a regular file.";
        return false;
      }
    }
#ifdef MAIDSAFE_WIN32
#pragma warning(pop)
#endif
    return true;
  }
};

class InputPolicyGetEndpoint {
 protected:
  virtual ~InputPolicyGetEndpoint() {}
  typedef boost::asio::ip::udp::endpoint InputType;
  bool GetInput(InputType& endpoint, std::string& error_info) const {
    error_info.clear();
    std::string entered_endpoint;
    std::getline(std::cin, entered_endpoint);

    auto fail([&error_info, entered_endpoint](const std::string& error_finish) -> bool {
      error_info = "\"" + entered_endpoint + "\" is not a valid endpoint (" + error_finish;
      return false;
    });

    std::string entered_address, entered_port;
    if (!SplitInput(entered_endpoint, fail, entered_address, entered_port))
      return false;

    boost::asio::ip::address_v4 address;
    if (!GetAddress(entered_address, fail, address))
      return false;

    uint16_t port;
    if (!GetPort(entered_port, fail, port))
      return false;

    boost::system::error_code error_code;
    endpoint = InputType(address, port);
    return true;
  }

 private:
  bool SplitInput(const std::string& entered_endpoint,
                  std::function<bool(const std::string&)> fail_functor,
                  std::string& entered_address, std::string& entered_port) const {
    if (entered_endpoint.size() < 9)
      return fail_functor("string too small).");

    size_t colon_pos(entered_endpoint.find(':'));
    if (colon_pos > entered_endpoint.size() - 2)
      return fail_functor("missing \":<port>\").");

    entered_address = entered_endpoint.substr(0, colon_pos);
    entered_port = entered_endpoint.substr(colon_pos + 1);
    return true;
  }

  bool GetPort(const std::string& entered_port,
               std::function<bool(const std::string&)> fail_functor, uint16_t& port) const {
    try {
      int port_as_int = std::stoi(entered_port);
      if (port_as_int < 1025 || port_as_int > std::numeric_limits<uint16_t>::max())
        return fail_functor("invalid port " + entered_port + ").");
      port = static_cast<uint16_t>(port_as_int);
    } catch (const std::exception&) {
      return fail_functor("invalid port " + entered_port + ").");
    }
    return true;
  }
  bool GetAddress(const std::string& entered_address,
                  std::function<bool(const std::string&)> fail_functor,
                  boost::asio::ip::address_v4& address) const {
    boost::system::error_code error_code;
    address = boost::asio::ip::address_v4::from_string(entered_address, error_code);
    if (address.is_unspecified() || error_code)
      return fail_functor("invalid address " + entered_address + ").");
    return true;
  }
};

// Handle Policies
class HandlePolicyLoadBootstrapFile {
 protected:
  virtual ~HandlePolicyLoadBootstrapFile() {}
  void HandleInput(const fs::path& bootstrap_file) const {
    if (g_out_of_date) {
      if (!ConfirmChoice(false)) {
        TLOG(kDefaultColour) << "\n\n";
        return;
      }
    }
    try {
      maidsafe::NonEmptyString contents(maidsafe::ReadFile(bootstrap_file).value());
      maidsafe::detail::BootstrapCereal parsed_endpoints;

      try {
        maidsafe::Parse(contents.string(), parsed_endpoints);
      } catch (...) {
        TLOG(kRed) << '\n' << bootstrap_file << " doesn't parse.\n\n";
        return;
      }

      if (parsed_endpoints.bootstrap_contacts_.size() == 0) {
        TLOG(kRed) << '\n' << bootstrap_file << " doesn't contain any endpoints.\n\n";
        return;
      }
      g_bootstrap_endpoints.clear();
      for (std::size_t i(0); i != parsed_endpoints.bootstrap_contacts_.size(); ++i) {
        g_bootstrap_endpoints.push_back(boost::asio::ip::udp::endpoint(
            boost::asio::ip::address_v4::from_string(parsed_endpoints.bootstrap_contacts_[i].ip_),
            static_cast<uint16_t>(parsed_endpoints.bootstrap_contacts_[i].port_)));
      }
      g_out_of_date = false;
    } catch (const std::exception& e) {
      TLOG(kRed) << "\nException while loading " << bootstrap_file << ": " << e.what() << "\n\n";
    }
    TLOG(kGreen) << "\nLoaded " << bootstrap_file << "\n\n";
  }
};

class HandlePolicySaveBootstrapFile {
 protected:
  virtual ~HandlePolicySaveBootstrapFile() {}
  void HandleInput(const fs::path& bootstrap_file) const {
    maidsafe::detail::BootstrapCereal serialised_endpoints;
    for (auto endpoint : g_bootstrap_endpoints) {
      serialised_endpoints.bootstrap_contacts_.emplace_back();
      auto pos_last(serialised_endpoints.bootstrap_contacts_.size() - 1);
      maidsafe::detail::EndpointCereal* serialised_endpoint =
          &serialised_endpoints.bootstrap_contacts_[pos_last];
      serialised_endpoint->ip_ = endpoint.address().to_string();
      serialised_endpoint->port_ = endpoint.port();
    }
    maidsafe::SerialisedData contents{maidsafe::Serialise(serialised_endpoints)};
    if (maidsafe::WriteFile(bootstrap_file, contents)) {
      g_out_of_date = false;
      TLOG(kGreen) << "\nSaved " << bootstrap_file << "\n\n";
    } else {
      TLOG(kRed) << "\nFailed to write to " << bootstrap_file << "\n\n";
    }
  }
};

class HandlePolicyPrependEndpoint {
 protected:
  virtual ~HandlePolicyPrependEndpoint() {}
  void HandleInput(const boost::asio::ip::udp::endpoint& endpoint) const {
    g_bootstrap_endpoints.insert(g_bootstrap_endpoints.begin(), endpoint);
    g_out_of_date = true;
    TLOG(kGreen) << "\nPrepended " << endpoint << "\n\n";
  }
};

class HandlePolicyAppendEndpoint {
 protected:
  virtual ~HandlePolicyAppendEndpoint() {}
  void HandleInput(const boost::asio::ip::udp::endpoint& endpoint) const {
    g_bootstrap_endpoints.push_back(endpoint);
    g_out_of_date = true;
    TLOG(kGreen) << "\nAppended " << endpoint << "\n\n";
  }
};

class HandlePolicyRemoveEndpoint {
 protected:
  virtual ~HandlePolicyRemoveEndpoint() {}
  void HandleInput(const boost::asio::ip::udp::endpoint& endpoint) const {
    auto itr(std::find(g_bootstrap_endpoints.begin(), g_bootstrap_endpoints.end(), endpoint));
    if (itr == g_bootstrap_endpoints.end()) {
      TLOG(kRed) << '\n' << endpoint << " is not one of the loaded endpoints.\n\n";
      return;
    }
    g_bootstrap_endpoints.erase(itr);
    g_out_of_date = true;
    TLOG(kGreen) << "\nRemoved " << endpoint << "\n\n";
  }
};

class HandlePolicyViewEndpoints {
 protected:
  virtual ~HandlePolicyViewEndpoints() {}
  void HandleInput(int) const {  // NOLINT
    if (g_bootstrap_endpoints.empty()) {
      TLOG(kGreen) << "\nCurrently no endpoints are loaded.\n";
    } else {
      TLOG(kGreen) << "\nCurrent endpoints:\n";
      for (auto endpoint : g_bootstrap_endpoints)
        TLOG(kGreen) << endpoint.address().to_string() << ":" << endpoint.port() << '\n';
      TLOG(kGreen) << "\n\n";
    }
  }
};

class HandlePolicyExit {
 protected:
  virtual ~HandlePolicyExit() {}
  void HandleInput(int) const {  // NOLINT
    if (g_out_of_date) {
      if (ConfirmChoice(true))
        g_running = false;
    } else {
      g_running = false;
    }
    TLOG(kDefaultColour) << "\n\n";
  }
};

// Choice typedefs
typedef Choice<1, MessagePolicyGetPath<true>, InputPolicyGetPath<true>,
               HandlePolicyLoadBootstrapFile> LoadBootstrapChoice;
typedef Choice<2, MessagePolicyPrependEndpoint, InputPolicyGetEndpoint, HandlePolicyPrependEndpoint>
    PrependEndpointChoice;
typedef Choice<3, MessagePolicyAppendEndpoint, InputPolicyGetEndpoint, HandlePolicyAppendEndpoint>
    AppendEndpointChoice;
typedef Choice<4, MessagePolicyRemoveEndpoint, InputPolicyGetEndpoint, HandlePolicyRemoveEndpoint>
    RemoveEndpointChoice;
typedef Choice<5, MessagePolicyGetPath<false>, InputPolicyGetPath<false>,
               HandlePolicySaveBootstrapFile> SaveBootstrapChoice;
typedef Choice<6, MessagePolicyViewEndpoints, InputPolicyNull, HandlePolicyViewEndpoints>
    ViewEndpointsChoice;
typedef Choice<7, MessagePolicyExit, InputPolicyNull, HandlePolicyExit> ExitChoice;

// Helpers and main
void PrintCommands() {
  TLOG(kDefaultColour) << "\n=====================================================\n";
  LoadBootstrapChoice::PrintCommandPreamble();
  PrependEndpointChoice::PrintCommandPreamble();
  AppendEndpointChoice::PrintCommandPreamble();
  RemoveEndpointChoice::PrintCommandPreamble();
  SaveBootstrapChoice::PrintCommandPreamble();
  ViewEndpointsChoice::PrintCommandPreamble();
  ExitChoice::PrintCommandPreamble();
}

int GetChoice() {
  std::string input;
  int choice(-1);
  for (;;) {
    TLOG(kDefaultColour) << ">> ";
    std::getline(std::cin, input);
    try {
      choice = std::stoi(input);
    } catch (const std::exception&) {
    }

    if (choice > 0 && choice < 8)
      break;

    TLOG(kDefaultColour) << "\nEnter a single digit in the range [1,7] ";
  }
  return choice;
}

void HandleChoice(int choice) {
  switch (choice) {
    case LoadBootstrapChoice::index: {
      LoadBootstrapChoice choice;
      return choice.Execute();
    }
    case PrependEndpointChoice::index: {
      PrependEndpointChoice choice;
      return choice.Execute();
    }
    case AppendEndpointChoice::index: {
      AppendEndpointChoice choice;
      return choice.Execute();
    }
    case RemoveEndpointChoice::index: {
      RemoveEndpointChoice choice;
      return choice.Execute();
    }
    case SaveBootstrapChoice::index: {
      SaveBootstrapChoice choice;
      return choice.Execute();
    }
    case ViewEndpointsChoice::index: {
      ViewEndpointsChoice choice;
      return choice.Execute();
    }
    default: {
      ExitChoice choice;
      return choice.Execute();
    }
  }
}

int main(int argc, char* argv[]) {
  maidsafe::log::Logging::Instance().Initialise(argc, argv);
  while (g_running) {
    PrintCommands();
    HandleChoice(GetChoice());
  }
  return 0;
}
