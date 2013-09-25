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

#ifndef MAIDSAFE_COMMON_TAGGED_VALUE_H_
#define MAIDSAFE_COMMON_TAGGED_VALUE_H_

namespace maidsafe {

template <typename T, typename Tag>
struct TaggedValue {
  typedef T value_type;
  typedef Tag tag_type;
  explicit TaggedValue(T const& data_) : data(data_) {}
  TaggedValue() : data() {}
  operator T() const { return data; }
  T const* operator->() const { return &data; }
  T* operator->() { return &data; }
  T data;
};

template <typename T, typename Tag>
inline bool operator==(const TaggedValue<T, Tag>& lhs, const TaggedValue<T, Tag>& rhs) {
  return lhs.data == rhs.data;
}

template <typename T, typename Tag>
inline bool operator!=(const TaggedValue<T, Tag>& lhs, const TaggedValue<T, Tag>& rhs) {
  return lhs.data != rhs.data;
}

template <typename T, typename Tag>
inline bool operator<(const TaggedValue<T, Tag>& lhs, const TaggedValue<T, Tag>& rhs) {
  return lhs.data < rhs.data;
}

template <typename T, typename Tag>
inline bool operator>(const TaggedValue<T, Tag>& lhs, const TaggedValue<T, Tag>& rhs) {
  return lhs.data > rhs.data;
}

template <typename T, typename Tag>
inline bool operator<=(const TaggedValue<T, Tag>& lhs, const TaggedValue<T, Tag>& rhs) {
  return lhs.data <= rhs.data;
}

template <typename T, typename Tag>
inline bool operator>=(const TaggedValue<T, Tag>& lhs, const TaggedValue<T, Tag>& rhs) {
  return lhs.data >= rhs.data;
}

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_TAGGED_VALUE_H_
