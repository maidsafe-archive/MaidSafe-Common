/* Copyright 2012 MaidSafe.net limited

This MaidSafe Software is licensed under the MaidSafe.net Commercial License, version 1.0 or later,
and The General Public License (GPL), version 3. By contributing code to this project You agree to
the terms laid out in the MaidSafe Contributor Agreement, version 1.0, found in the root directory
of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also available at:

http://www.novinet.com/license

Unless required by applicable law or agreed to in writing, software distributed under the License is
distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. See the License for the specific language governing permissions and limitations under the
License.
*/

#ifndef MAIDSAFE_COMMON_TAGGED_VALUE_H_
#define MAIDSAFE_COMMON_TAGGED_VALUE_H_


namespace maidsafe {

template<typename T, typename Tag>
struct TaggedValue {
  typedef T value_type;
  typedef Tag tag_type;
  explicit TaggedValue(T const& data_): data(data_) {}
  TaggedValue(): data() {}
  operator T() const { return data; }
  T const* operator->() const { return &data; }
  T* operator->() { return &data; }
  T data;
};

template<typename T, typename Tag>
inline bool operator==(const TaggedValue<T, Tag>& lhs, const TaggedValue<T, Tag>& rhs) {
  return lhs.data == rhs.data;
}

template<typename T, typename Tag>
inline bool operator!=(const TaggedValue<T, Tag>& lhs, const TaggedValue<T, Tag>& rhs) {
  return lhs.data != rhs.data;
}

template<typename T, typename Tag>
inline bool operator<(const TaggedValue<T, Tag>& lhs, const TaggedValue<T, Tag>& rhs) {
  return lhs.data < rhs.data;
}

template<typename T, typename Tag>
inline bool operator>(const TaggedValue<T, Tag>& lhs, const TaggedValue<T, Tag>& rhs) {
  return lhs.data > rhs.data;
}

template<typename T, typename Tag>
inline bool operator<=(const TaggedValue<T, Tag>& lhs, const TaggedValue<T, Tag>& rhs) {
  return lhs.data <= rhs.data;
}

template<typename T, typename Tag>
inline bool operator>=(const TaggedValue<T, Tag>& lhs, const TaggedValue<T, Tag>& rhs) {
  return lhs.data >= rhs.data;
}

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_TAGGED_VALUE_H_
