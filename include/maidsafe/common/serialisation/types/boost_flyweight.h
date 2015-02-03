/*  Copyright 2015 MaidSafe.net limited

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

#ifndef MAIDSAFE_COMMON_SERIALISATION_TYPES_BOOST_FLYWEIGHT_H_
#define MAIDSAFE_COMMON_SERIALISATION_TYPES_BOOST_FLYWEIGHT_H_

#include <cassert>
#include <memory>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4520)
#endif
#include "boost/flyweight.hpp"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "cereal/cereal.hpp"

namespace cereal {

template <typename Archive, typename Type, typename... Params>
void save(Archive& archive, const boost::flyweight<Type, Params...>& flyweight) {
  const uint32_t id = archive.registerSharedPointer(std::addressof(flyweight.get()));
  archive(make_nvp("id", id));

  if (id & detail::msb_32bit) {
    archive(make_nvp("data", flyweight.get()));
  }
}

template <typename Archive, typename Type, typename... Params>
void load(Archive& archive, boost::flyweight<Type, Params...>& flyweight) {
  struct Helper {
    explicit Helper(boost::flyweight<Type, Params...> value_in)
      : value(std::move(value_in)) {
    }
    boost::flyweight<Type, Params...> value;
  };

  std::shared_ptr<Helper> ptr{};

  uint32_t id{};
  archive(make_nvp("id", id));

  if (id & detail::msb_32bit) {
    Type object{};
    archive(make_nvp("data", object));
    ptr = std::make_shared<Helper>(boost::flyweight<Type, Params...>{object});
    archive.registerSharedPointer(id, ptr);
  } else {
    ptr = std::static_pointer_cast<Helper>(archive.getSharedPointer(id));
  }

  assert(ptr != nullptr);
  flyweight = ptr->value;
}

}  // namespace cereal

#endif  // MAIDSAFE_COMMON_SERIALISATION_TYPES_BOOST_FLYWEIGHT_H_
