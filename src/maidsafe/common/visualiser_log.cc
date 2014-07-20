/*  Copyright 2014 MaidSafe.net limited

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

#include "maidsafe/common/visualiser_log.h"

#include "maidsafe/common/log.h"
#include "maidsafe/common/utils.h"

namespace maidsafe {

namespace log {

namespace {

std::string UrlEncode(const std::string& value) {
  static const std::string kUrlEncodedAlphabet[] = {
    "%00", "%01", "%02", "%03", "%04", "%05", "%06", "%07", "%08", "%09", "%0a", "%0b", "%0c",
    "%0d", "%0e", "%0f", "%10", "%11", "%12", "%13", "%14", "%15", "%16", "%17", "%18", "%19",
    "%1a", "%1b", "%1c", "%1d", "%1e", "%1f", "+", "%21", "%22", "%23", "%24", "%25", "%26", "%27",
    "%28", "%29", "%2a", "%2b", "%2c", "-", ".", "%2f", "0", "1", "2", "3", "4", "5", "6", "7", "8",
    "9", "%3a", "%3b", "%3c", "%3d", "%3e", "%3f", "%40", "A", "B", "C", "D", "E", "F", "G", "H",
    "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "%5b",
    "%5c", "%5d", "%5e", "_", "%60", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l",
    "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "%7b", "%7c", "%7d", "~",
    "%7f", "%80", "%81", "%82", "%83", "%84", "%85", "%86", "%87", "%88", "%89", "%8a", "%8b",
    "%8c", "%8d", "%8e", "%8f", "%90", "%91", "%92", "%93", "%94", "%95", "%96", "%97", "%98",
    "%99", "%9a", "%9b", "%9c", "%9d", "%9e", "%9f", "%a0", "%a1", "%a2", "%a3", "%a4", "%a5",
    "%a6", "%a7", "%a8", "%a9", "%aa", "%ab", "%ac", "%ad", "%ae", "%af", "%b0", "%b1", "%b2",
    "%b3", "%b4", "%b5", "%b6", "%b7", "%b8", "%b9", "%ba", "%bb", "%bc", "%bd", "%be", "%bf",
    "%c0", "%c1", "%c2", "%c3", "%c4", "%c5", "%c6", "%c7", "%c8", "%c9", "%ca", "%cb", "%cc",
    "%cd", "%ce", "%cf", "%d0", "%d1", "%d2", "%d3", "%d4", "%d5", "%d6", "%d7", "%d8", "%d9",
    "%da", "%db", "%dc", "%dd", "%de", "%df", "%e0", "%e1", "%e2", "%e3", "%e4", "%e5", "%e6",
    "%e7", "%e8", "%e9", "%ea", "%eb", "%ec", "%ed", "%ee", "%ef", "%f0", "%f1", "%f2", "%f3",
    "%f4", "%f5", "%f6", "%f7", "%f8", "%f9", "%fa", "%fb", "%fc", "%fd", "%fe", "%ff"
  };
  std::string encoded;
  encoded.reserve(value.size() * 3);
  for (const char& c : value)
    encoded += kUrlEncodedAlphabet[static_cast<unsigned char>(c)];
  return encoded;
}

std::string EncodeIdentityOrInt(const std::string& value, bool debug_format) {
  // If the value is 64 chars, assume it's an Identity.
  if (value.size() == crypto::SHA512::DIGESTSIZE)
    return debug_format ? HexSubstr(value) : HexEncode(value);
  return value;
}

}  // unnamed namespace

VisualiserLogMessage::~VisualiserLogMessage() {
  if (kSessionId_.empty())
    return;
  SendToServer();
  WriteToFile();
}

void VisualiserLogMessage::SendVaultStoppedMessage(const std::string& vault_debug_id,
                                                   const std::string& session_id, int exit_code) {
  try {
    std::string message{ "ts=" + UrlEncode(detail::GetUTCTime()) + "&vault_id=" + vault_debug_id +
        "&session_id=" + session_id + "&action_id=18&value1=" + std::to_string(exit_code) };
    auto post_functor([message] { Logging::Instance().WriteToVisualiserServer(message); });
    Logging::Instance().Send(post_functor);
  }
  catch (const std::exception& e) {
    LOG(kError) << "Error writing VLOG to file: " << boost::diagnostic_information(e);
  }
}

std::string VisualiserLogMessage::GetPostRequestBody() const {
  return std::string {
      "ts=" + UrlEncode(kTimestamp_) +
      "&vault_id=" + kVaultId_ +
      "&session_id=" + kSessionId_ +
      (kPersonaId_.name.empty() ? "" : "&persona_id=" + kPersonaId_.value) +
      "&action_id=" + kActionId_.value +
      "&value1=" + EncodeIdentityOrInt(kValue1_, false) +
      (kValue2_.empty() ? "" : "&value2=" + EncodeIdentityOrInt(kValue2_, false)) };
}

void VisualiserLogMessage::SendToServer() const {
  try {
    std::string message{ GetPostRequestBody() };
    auto post_functor([message] { Logging::Instance().WriteToVisualiserServer(message); });
    Logging::Instance().Send(post_functor);
  }
  catch (const std::exception& e) {
    LOG(kError) << "Error writing VLOG to file: " << boost::diagnostic_information(e);
  }
}

void VisualiserLogMessage::WriteToFile() const {
  try {
    std::string log_entry{
        kTimestamp_ + ',' +
        kVaultId_ + ',' +
        kSessionId_ + ',' +
        (kPersonaId_.name.empty() ? "" : kPersonaId_.name + ',') +
        kActionId_.name + ',' +
        EncodeIdentityOrInt(kValue1_, true) +
        (kValue2_.empty() ? "" : "," + EncodeIdentityOrInt(kValue2_, true)) + '\n' };
    auto print_functor([log_entry] { Logging::Instance().WriteToVisualiserLogfile(log_entry); });
    Logging::Instance().Async() ? Logging::Instance().Send(print_functor) : print_functor();
  }
  catch (const std::exception& e) {
    LOG(kError) << "Error writing VLOG to file: " << boost::diagnostic_information(e);
  }
}

}  // namespace log

}  // namespace maidsafe
