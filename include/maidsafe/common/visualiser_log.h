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

#ifndef MAIDSAFE_COMMON_VISUALISER_LOG_H_
#define MAIDSAFE_COMMON_VISUALISER_LOG_H_

#include <sstream>
#include <string>
#include <type_traits>

#include "cereal/cereal.hpp"
#include <cereal/archives/json.hpp>

#include "maidsafe/common/error.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/types.h"
#include "maidsafe/common/utils.h"

#ifdef USE_VLOGGING
#define VLOG maidsafe::log::VisualiserLogMessage
#else
#define VLOG(...) true ? static_cast<void>(19) : \
                         (maidsafe::log::VisualiserLogMessage(__VA_ARGS__), static_cast<void>(19))
#endif

namespace maidsafe {

namespace test { class VisualiserLogTest; }

namespace log {

template<typename T>
struct is_string : public std::integral_constant<
    bool, std::is_same<char*, typename std::decay<T>::type>::value ||
          std::is_same<const char*, typename std::decay<T>::type>::value> {};

template<>
struct is_string<std::string> : std::true_type {};

class VisualiserLogMessage {
 public:
  template <typename PersonaEnum, typename ActionEnum>
  VisualiserLogMessage(PersonaEnum persona, ActionEnum action, Identity value1,
                       Identity value2 = Identity{})
      : kTimestamp_(detail::GetUTCTime()),
        kVaultId_(Logging::Instance().VlogPrefix()),
        kSessionId_(Logging::Instance().VlogSessionId()),
        kValue1_(value1.string()),
        kValue2_(value2.IsInitialised() ? value2.string() : std::string()),
        kPersonaId_(persona),
        kActionId_(action) {}

  template <typename PersonaEnum, typename ActionEnum, typename T,
            typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
  VisualiserLogMessage(PersonaEnum persona, ActionEnum action, T value)
      : kTimestamp_(detail::GetUTCTime()),
        kVaultId_(Logging::Instance().VlogPrefix()),
        kSessionId_(Logging::Instance().VlogSessionId()),
        kValue1_(std::to_string(value)),
        kValue2_(),
        kPersonaId_(persona),
        kActionId_(action) {}

  template <typename ActionEnum>
  VisualiserLogMessage(ActionEnum action, Identity value1, Identity value2 = Identity{})
      : kTimestamp_(detail::GetUTCTime()),
        kVaultId_(Logging::Instance().VlogPrefix()),
        kSessionId_(Logging::Instance().VlogSessionId()),
        kValue1_(value1.string()),
        kValue2_(value2.IsInitialised() ? value2.string() : std::string()),
        kPersonaId_(),
        kActionId_(action) {}

  template <typename ActionEnum, typename T,
            typename std::enable_if<is_string<T>::value>::type* = nullptr>
  VisualiserLogMessage(ActionEnum action, Identity value1, T value2)
      : kTimestamp_(detail::GetUTCTime()),
        kVaultId_(Logging::Instance().VlogPrefix()),
        kSessionId_(Logging::Instance().VlogSessionId()),
        kValue1_(value1.string()),
        kValue2_(value2),
        kPersonaId_(),
        kActionId_(action) {}

  template <typename ActionEnum, typename T,
            typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
  VisualiserLogMessage(ActionEnum action, T value)
      : kTimestamp_(detail::GetUTCTime()),
        kVaultId_(Logging::Instance().VlogPrefix()),
        kSessionId_(Logging::Instance().VlogSessionId()),
        kValue1_(std::to_string(value)),
        kValue2_(),
        kPersonaId_(),
        kActionId_(action) {}

  ~VisualiserLogMessage();

  // Nasty workaround to allow VaultManager to pretend to be a given crashed vault in order to send
  // a VLOG kVaultStopped message.  Assumes kVaultStopped has value of 18.
  static void SendVaultStoppedMessage(const std::string& vault_debug_id,
                                      const std::string& session_id, int exit_code);


  friend class cereal::access;
  friend class test::VisualiserLogTest;

 private:
  struct Enum {
    template <typename EnumType>
    explicit Enum(EnumType e)
        : value(std::to_string(static_cast<typename std::underlying_type<EnumType>::type>(e))),
          name([e]()->std::string {
            std::ostringstream stream;
            stream << e;
            return stream.str();
          }()) {
      if (!IsValid(e))
        BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_parameter));
    }
    Enum() = default;
    Enum(const Enum&) = default;
    Enum(Enum&& other) : value(std::move(other.value)), name(std::move(other.name)) {}
    Enum& operator=(const Enum&) = default;

    template <class Archive>
    void serialize( Archive & ar ) {
      ar( CEREAL_NVP(value), CEREAL_NVP(name));
    }
    std::string value, name;
  };

  template <class Archive>
  void serialize(Archive & archive) {
    archive(cereal::make_nvp("ts", kTimestamp_),
            cereal::make_nvp("vault_id", kVaultId_),
            cereal::make_nvp("session_id", kSessionId_),
            cereal::make_nvp("value1", kValue1_),
            cereal::make_nvp("value2", kValue2_),
            cereal::make_nvp("persona_id", kPersonaId_),
            cereal::make_nvp("action_id", kActionId_));
  }

  std::string GetPostRequestBody() const;
  void SendToServer() const;
  void WriteToFile() const;

  const std::string kTimestamp_, kVaultId_, kSessionId_, kValue1_, kValue2_;
  const Enum kPersonaId_, kActionId_;
};

}  // namespace log

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_VISUALISER_LOG_H_
