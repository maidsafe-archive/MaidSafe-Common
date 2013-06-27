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

#ifndef MAIDSAFE_COMMON_CONFIG_H_
#define MAIDSAFE_COMMON_CONFIG_H_

#include <string>
#include "boost/preprocessor/stringize.hpp"

#ifdef MAIDSAFE_WIN32
#  define MAIDSAFE_NOEXCEPT
#else
#  define MAIDSAFE_NOEXCEPT noexcept(true)
#endif

namespace maidsafe {

#ifdef COMPANY_NAME
const std::string kCompanyName(BOOST_PP_STRINGIZE(COMPANY_NAME));
#else
#  error COMPANY_NAME must be defined.
#endif

#ifdef APPLICATION_NAME
const std::string kApplicationName(BOOST_PP_STRINGIZE(APPLICATION_NAME));
#else
#  error APPLICATION_NAME must be defined.
#endif

#if defined APPLICATION_VERSION_MAJOR && \
    defined APPLICATION_VERSION_MINOR && \
    defined APPLICATION_VERSION_PATCH
const std::string kApplicationVersion(BOOST_PP_STRINGIZE(APPLICATION_VERSION_MAJOR) +
                                      std::string(".") +
                                      BOOST_PP_STRINGIZE(APPLICATION_VERSION_MINOR) +
                                      std::string(".") +
                                      BOOST_PP_STRINGIZE(APPLICATION_VERSION_PATCH));
#else
#  error APPLICATION_VERSION_MAJOR, APPLICATION_VERSION_MINOR and APPLICATION_VERSION_PATCH \
         must be defined.
#endif

#ifdef TARGET_PLATFORM
const std::string kTargetPlatform(BOOST_PP_STRINGIZE(TARGET_PLATFORM));
#else
#  error TARGET_PLATFORM must be defined.
#endif

#ifdef TARGET_ARCHITECTURE
const std::string kTargetArchitecture(BOOST_PP_STRINGIZE(TARGET_ARCHITECTURE));
#else
#  error TARGET_ARCHITECTURE must be defined.
#endif

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_CONFIG_H_
