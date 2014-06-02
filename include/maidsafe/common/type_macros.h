/*  Copyright 2013 MaidSafe.net limited

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

#ifndef MAIDSAFE_COMMON_TYPE_MACROS_H_
#define MAIDSAFE_COMMON_TYPE_MACROS_H_

#include <string>

#include "boost/preprocessor/arithmetic/dec.hpp"
#include "boost/preprocessor/cat.hpp"
#include "boost/preprocessor/comparison/not_equal.hpp"
#include "boost/preprocessor/punctuation/comma_if.hpp"
#include "boost/preprocessor/seq/elem.hpp"
#include "boost/preprocessor/seq/for_each.hpp"
#include "boost/preprocessor/seq/for_each_i.hpp"
#include "boost/preprocessor/seq/seq.hpp"
#include "boost/preprocessor/seq/size.hpp"
#include "boost/preprocessor/stringize.hpp"


#define GET_OSTREAMABLE_ENUM_VALUE(value) BOOST_PP_CAT(k, value)

#define GET_OSTREAMABLE_FIRST_WITH_COMMA(r, data, i, elem)                                         \
    GET_OSTREAMABLE_ENUM_VALUE(elem)                                                               \
    BOOST_PP_COMMA_IF(BOOST_PP_NOT_EQUAL(i, data))

#define X_DEFINE_OSTREAMABLE_ENUM_WITH_STRING_CONVERSIONS_TOSTRING_CASE(r, data, elem)             \
    case data::GET_OSTREAMABLE_ENUM_VALUE(elem) :                                                  \
      str = BOOST_PP_STRINGIZE(elem);                                                              \
      break;

// Defines:
//     enum class 'name' : 'type' { 'enumerators' };
// Each enumerator is prefixed with a 'k' char.
// Also defines a std::ostream operator<< for the enum class.
// Also defines a 'bool IsValid(name)' function which returns true iff 'name' is a valid value.
#define DEFINE_OSTREAMABLE_ENUM_VALUES(name, type, enumerators)                                    \
    enum class name : type {                                                                       \
      BOOST_PP_SEQ_FOR_EACH_I(GET_OSTREAMABLE_FIRST_WITH_COMMA,                                    \
                              BOOST_PP_DEC(BOOST_PP_SEQ_SIZE(enumerators)), enumerators)           \
    };                                                                                             \
                                                                                                   \
    template<typename Elem, typename Traits>                                                       \
    std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ostream,        \
                                                 name n) {                                         \
      std::string str;                                                                             \
      switch (n) {                                                                                 \
        BOOST_PP_SEQ_FOR_EACH(                                                                     \
            X_DEFINE_OSTREAMABLE_ENUM_WITH_STRING_CONVERSIONS_TOSTRING_CASE,                       \
            name,                                                                                  \
            enumerators)                                                                           \
        default:                                                                                   \
          str = "Invalid " BOOST_PP_STRINGIZE(name) " type";                                       \
          break;                                                                                   \
      }                                                                                            \
      for (std::string::iterator itr(str.begin()); itr != str.end(); ++itr)                        \
        ostream << ostream.widen(*itr);                                                            \
      return ostream;                                                                              \
    }                                                                                              \
                                                                                                   \
    inline bool IsValid(name n) {                                                                  \
      return n >= name::GET_OSTREAMABLE_ENUM_VALUE(BOOST_PP_SEQ_HEAD(enumerators)) &&              \
             n <= name::GET_OSTREAMABLE_ENUM_VALUE(BOOST_PP_SEQ_ELEM(                              \
                      BOOST_PP_DEC(BOOST_PP_SEQ_SIZE(enumerators)), enumerators));                 \
    }


#endif  // MAIDSAFE_COMMON_TYPE_MACROS_H_
