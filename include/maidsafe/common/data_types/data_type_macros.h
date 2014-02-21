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

#ifndef MAIDSAFE_DATA_TYPES_DATA_TYPE_MACROS_H_
#define MAIDSAFE_DATA_TYPES_DATA_TYPE_MACROS_H_

#include <string>

#include "boost/preprocessor/arithmetic/dec.hpp"
#include "boost/preprocessor/cat.hpp"
#include "boost/preprocessor/comparison/not_equal.hpp"
#include "boost/preprocessor/punctuation/comma_if.hpp"
#include "boost/preprocessor/punctuation/paren.hpp"
#include "boost/preprocessor/seq/enum.hpp"
#include "boost/preprocessor/seq/for_each.hpp"
#include "boost/preprocessor/seq/for_each_i.hpp"
#include "boost/preprocessor/seq/push_back.hpp"
#include "boost/preprocessor/seq/seq.hpp"
#include "boost/preprocessor/seq/size.hpp"
#include "boost/preprocessor/stringize.hpp"
#include "boost/preprocessor/tuple/elem.hpp"


#define MAIDSAFE_DATA_TYPES_ADD_PAREN_0(A, B) ((A, B)) MAIDSAFE_DATA_TYPES_ADD_PAREN_1
#define MAIDSAFE_DATA_TYPES_ADD_PAREN_1(A, B) ((A, B)) MAIDSAFE_DATA_TYPES_ADD_PAREN_0
#define MAIDSAFE_DATA_TYPES_ADD_PAREN_0_END
#define MAIDSAFE_DATA_TYPES_ADD_PAREN_1_END
#define MAIDSAFE_DATA_TYPES_WRAP_PAIRS_IN_PARENS(input)                                            \
    BOOST_PP_CAT(MAIDSAFE_DATA_TYPES_ADD_PAREN_0 input,_END)  // NOLINT

#define MAIDSAFE_DATA_TYPES_GET_ENUM_VALUE(value) BOOST_PP_CAT(BOOST_PP_CAT(k, value), Value)

#define MAIDSAFE_DATA_TYPES_GET_FIRST_WITH_COMMA(r, data, i, elem)                                 \
    MAIDSAFE_DATA_TYPES_GET_ENUM_VALUE(BOOST_PP_TUPLE_ELEM(2, 0, elem))                            \
    BOOST_PP_COMMA_IF(BOOST_PP_NOT_EQUAL(i, data))

#define MAIDSAFE_DATA_TYPES_X_DEFINE_ENUM_WITH_STRING_CONVERSIONS_TOSTRING_CASE(r, data, elem)     \
    case data::MAIDSAFE_DATA_TYPES_GET_ENUM_VALUE(BOOST_PP_TUPLE_ELEM(2, 0, elem)) :               \
      str = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(2, 0, elem));                                   \
      break;

#define DEFINE_MAIDSAFE_DATA_TYPES_ENUM_VALUES_WITH_PARENS(name, type, enumerators)                \
    enum class name : type {                                                                       \
      BOOST_PP_SEQ_FOR_EACH_I(MAIDSAFE_DATA_TYPES_GET_FIRST_WITH_COMMA,                            \
                              BOOST_PP_DEC(BOOST_PP_SEQ_SIZE(enumerators)), enumerators)           \
    };                                                                                             \
                                                                                                   \
    template<typename Elem, typename Traits>                                                       \
    std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ostream,        \
                                                 const name &n) {                                  \
      std::string str;                                                                             \
      switch (n) {                                                                                 \
        BOOST_PP_SEQ_FOR_EACH(                                                                     \
            MAIDSAFE_DATA_TYPES_X_DEFINE_ENUM_WITH_STRING_CONVERSIONS_TOSTRING_CASE,               \
            name,                                                                                  \
            enumerators)                                                                           \
        default:                                                                                   \
          str = "Invalid " BOOST_PP_STRINGIZE(name) " type";                                       \
          break;                                                                                   \
      }                                                                                            \
      for (std::string::iterator itr(str.begin()); itr != str.end(); ++itr)                        \
        ostream << ostream.widen(*itr);                                                            \
      return ostream;                                                                              \
    }

#define DEFINE_MAIDSAFE_DATA_TYPES_ENUM_VALUES(name, type)                                         \
    DEFINE_MAIDSAFE_DATA_TYPES_ENUM_VALUES_WITH_PARENS(name, type,                                 \
        MAIDSAFE_DATA_TYPES_WRAP_PAIRS_IN_PARENS(MAIDSAFE_DATA_TYPES))



#define MAIDSAFE_DATA_TYPES_GET_SECOND_WITH_COMMA_AND_NAME_TYPE(r, data, i, elem)                  \
    BOOST_PP_TUPLE_ELEM(2, 1, elem) ::Name BOOST_PP_COMMA_IF(BOOST_PP_NOT_EQUAL(i, data))

#define MAIDSAFE_DATA_TYPES_MAP_FIRST_TO_SECOND(r, data, elem)                                     \
    case DataTagValue::MAIDSAFE_DATA_TYPES_GET_ENUM_VALUE(BOOST_PP_TUPLE_ELEM(2, 0, elem)) :       \
      return BOOST_PP_TUPLE_ELEM(2, 1, elem) ::Name BOOST_PP_LPAREN() name BOOST_PP_RPAREN();

/*
#define DEFINE_IS_MAIDSAFE_DATA_STRUCT(r, data, elem)                                              \
    template<DataTagValue tag_value>                                                               \
    struct is_maidsafe_data< tag_value,                                                            \
        typename std::enable_if<                                                                   \
            std::is_same<                                                                          \
                std::integral_constant< DataTagValue, tag_value >,                                 \
                std::integral_constant< DataTagValue, DataTagValue::                               \
          MAIDSAFE_DATA_TYPES_GET_ENUM_VALUE(BOOST_PP_TUPLE_ELEM(2, 0, elem))>>::value>::type> {   \
      static const bool value = true;                                                              \
      typedef BOOST_PP_TUPLE_ELEM(2, 1, elem) data_type;                                           \
      typedef data_type::name_type name_type;                                                      \
    };
*/

#define DEFINE_DATA_NAME_VARIANT_WITH_PARENS(enumerators)                                          \
    typedef boost::variant<                                                                        \
        BOOST_PP_SEQ_FOR_EACH_I(                                                                   \
        MAIDSAFE_DATA_TYPES_GET_SECOND_WITH_COMMA_AND_NAME_TYPE,                                   \
        BOOST_PP_DEC(BOOST_PP_SEQ_SIZE(enumerators)), enumerators) > DataNameVariant;              \
                                                                                                   \
    inline DataNameVariant GetDataNameVariant(DataTagValue type, const Identity& name) {           \
      switch (type) {                                                                              \
        BOOST_PP_SEQ_FOR_EACH(MAIDSAFE_DATA_TYPES_MAP_FIRST_TO_SECOND, , enumerators)              \
        default: {                                                                                 \
          LOG(kError) << "Unhandled data type";                                                    \
          BOOST_THROW_EXCEPTION(MakeError(CommonErrors::invalid_parameter));                       \
        }                                                                                          \
      }                                                                                            \
    }                                                                                              \
                                                                                                   \
//    BOOST_PP_SEQ_FOR_EACH(DEFINE_IS_MAIDSAFE_DATA_STRUCT, , enumerators)


#define DEFINE_DATA_NAME_VARIANT DEFINE_DATA_NAME_VARIANT_WITH_PARENS(                             \
    MAIDSAFE_DATA_TYPES_WRAP_PAIRS_IN_PARENS(MAIDSAFE_DATA_TYPES))

#endif  // MAIDSAFE_DATA_TYPES_DATA_TYPE_MACROS_H_
