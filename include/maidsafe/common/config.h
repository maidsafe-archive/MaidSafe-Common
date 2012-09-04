/* Copyright (c) 2012 maidsafe.net limited
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    * Neither the name of the maidsafe.net limited nor the names of its
    contributors may be used to endorse or promote products derived from this
    software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef MAIDSAFE_COMMON_CONFIG_H_
#define MAIDSAFE_COMMON_CONFIG_H_

#include <string>
#include "boost/preprocessor/stringize.hpp"

namespace maidsafe {

#if !defined(COMPANY_NAME)
const std::string kCompanyName("maidsafe");
#else
const std::string kCompanyName(BOOST_PP_STRINGIZE(COMPANY_NAME));
#endif

#if !defined(APPLICATION_NAME)
const std::string kApplicationName("lifestuff");
#else
const std::string kApplicationName(BOOST_PP_STRINGIZE(APPLICATION_NAME));
#endif

#if !defined(APPLICATION_VERSION_MAJOR) || \
    !defined(APPLICATION_VERSION_MINOR) || \
    !defined(APPLICATION_VERSION_PATCH)
#  error APPLICATION_VERSION_MAJOR, APPLICATION_VERSION_MINOR and APPLICATION_VERSION_PATCH \
         must be defined
#else
const std::string kApplicationVersion(BOOST_PP_STRINGIZE(APPLICATION_VERSION_MAJOR) +
                                      std::string(".") +
                                      BOOST_PP_STRINGIZE(APPLICATION_VERSION_MINOR) +
                                      std::string(".") +
                                      BOOST_PP_STRINGIZE(APPLICATION_VERSION_PATCH));
#endif

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_CONFIG_H_
