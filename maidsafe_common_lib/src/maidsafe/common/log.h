/* Copyright (c) 2010 maidsafe.net limited
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


#ifndef MAIDSAFE_COMMON_LOG_H_
#define MAIDSAFE_COMMON_LOG_H_

#include <string>
// For MSVC, we need to include windows.h which in turn includes WinGDI.h
// which defines ERROR (which conflicts with Glog's ERROR definition)
#ifdef __MSVC__
#  include <windows.h>  // NOLINT
#  undef ERROR
#  pragma warning(push, 1)
#  include "glog/logging.h"
#  pragma warning(pop)
#else
#  include "glog/logging.h"  // NOLINT (Fraser)
#endif
#include "maidsafe/common/version.h"

#if MAIDSAFE_COMMON_VERSION != 1004
#  error This API is not compatible with the installed library.\
    Please update the MaidSafe-Common library.
#endif


namespace google {

#undef COMPACT_GOOGLE_LOG_INFO
#undef LOG_TO_STRING_INFO
#undef COMPACT_GOOGLE_LOG_WARNING
#undef LOG_TO_STRING_WARNING
#undef COMPACT_GOOGLE_LOG_ERROR
#undef LOG_TO_STRING_ERROR
#undef COMPACT_GOOGLE_LOG_FATAL
#undef LOG_TO_STRING_FATAL
#undef COMPACT_GOOGLE_LOG_DFATAL
#undef LOG
#undef DLOG

#if GOOGLE_STRIP_LOG == 0
#  define COMPACT_GOOGLE_LOG_INFO(project, separator) \
          (FLAGS_ms_logging_ ## project > 0) ? (void) 0 : \
          google::LogMessageVoidify() & google::LogMessage( \
          std::string((std::string(__FILE__) + #separator + \
          #project)).c_str(), __LINE__)
#  define LOG_TO_STRING_INFO(project, separator, message) \
          (FLAGS_ms_logging_ ## project > 0) ? (void) 0 : \
          google::LogMessageVoidify() & google::LogMessage( \
          std::string((std::string(__FILE__) + #separator + \
          #project)).c_str(), __LINE__, google::INFO, message)
#else
#  define COMPACT_GOOGLE_LOG_INFO(project, separator) google::NullStream()
#  define LOG_TO_STRING_INFO(project, separator, message) google::NullStream()
#endif

#if GOOGLE_STRIP_LOG <= 1
#  define COMPACT_GOOGLE_LOG_WARNING(project, separator) \
          (FLAGS_ms_logging_ ## project > 1) ? (void) 0 : \
          google::LogMessageVoidify() & google::LogMessage( \
          std::string((std::string(__FILE__) + #separator + \
          #project)).c_str(), __LINE__, google::WARNING)
#  define LOG_TO_STRING_WARNING(project, separator, message) \
          (FLAGS_ms_logging_ ## project > 1) ? (void) 0 : \
          google::LogMessageVoidify() & google::LogMessage( \
          std::string((std::string(__FILE__) + #separator + \
          #project)).c_str(), __LINE__, google::WARNING, message)
#else
#  define COMPACT_GOOGLE_LOG_WARNING(project, separator) google::NullStream()
#  define LOG_TO_STRING_WARNING(project, separator, message) \
          google::NullStream()
#endif

#if GOOGLE_STRIP_LOG <= 2
#  define COMPACT_GOOGLE_LOG_ERROR(project, separator) \
          (FLAGS_ms_logging_ ## project > 2) ? (void) 0 : \
          google::LogMessageVoidify() & google::LogMessage( \
          std::string((std::string(__FILE__) + #separator + \
          #project)).c_str(), __LINE__, google::ERROR)
#  define LOG_TO_STRING_ERROR(project, separator, message) \
          (FLAGS_ms_logging_ ## project > 2) ? (void) 0 : \
          google::LogMessageVoidify() & google::LogMessage( \
          std::string((std::string(__FILE__) + #separator + \
          #project)).c_str(), __LINE__, google::ERROR, message)
#else
#  define COMPACT_GOOGLE_LOG_ERROR(project, separator) google::NullStream()
#  define LOG_TO_STRING_ERROR(project, separator, message) google::NullStream()
#endif

#if GOOGLE_STRIP_LOG <= 3
#  define COMPACT_GOOGLE_LOG_FATAL(project, separator) \
          (FLAGS_ms_logging_ ## project > 3) ? (void) 0 : \
          google::LogMessageVoidify() & google::LogMessageFatal( \
          std::string((std::string(__FILE__) + #separator + \
          #project)).c_str(), __LINE__)
#  define LOG_TO_STRING_FATAL(project, separator, message) \
          (FLAGS_ms_logging_ ## project > 3) ? (void) 0 : \
          google::LogMessageVoidify() & google::LogMessage( \
          std::string((std::string(__FILE__) + #separator + \
          #project)).c_str(), __LINE__, google::FATAL, message)
#else
#  define COMPACT_GOOGLE_LOG_FATAL(project, separator) google::NullStreamFatal()
#  define LOG_TO_STRING_FATAL(project, separator, message) \
          google::NullStreamFatal()
#endif

#ifdef NDEBUG
#  define COMPACT_GOOGLE_LOG_DFATAL COMPACT_GOOGLE_LOG_ERROR
#elif GOOGLE_STRIP_LOG <= 3
#  define COMPACT_GOOGLE_LOG_DFATAL(project, separator) \
          !(FLAGS_ms_logging_ ## project) ? (void) 0 : \
          google::LogMessageVoidify() & google::LogMessage( \
          std::string((std::string(__FILE__) + #separator + \
          #project)).c_str(), __LINE__, google::FATAL)
#else
#  define COMPACT_GOOGLE_LOG_DFATAL(project, separator) \
          google::NullStreamFatal()
#endif


#define ULOG(severity) COMPACT_GOOGLE_LOG_ ## severity(user, :).stream()
#define BLOG(severity) COMPACT_GOOGLE_LOG_ ## severity(benchmark, :).stream()

#define LOG(severity) COMPACT_GOOGLE_LOG_ ## severity(common, :).stream()
#ifndef NDEBUG
#  define DLOG(severity) LOG(severity)
#else
#  define DLOG(severity) google::NullStream()
#endif
}  // google

extern int FLAGS_ms_logging_user;
extern int FLAGS_ms_logging_benchmark;

extern int FLAGS_ms_logging_common;
extern int FLAGS_ms_logging_transport;
extern int FLAGS_ms_logging_encrypt;
extern int FLAGS_ms_logging_dht;
extern int FLAGS_ms_logging_pki;
extern int FLAGS_ms_logging_passport;
extern int FLAGS_ms_logging_pd;
extern int FLAGS_ms_logging_lifestuff;
extern int FLAGS_ms_logging_lifestuff_gui;
extern int FLAGS_ms_logging_file_browser;
extern int FLAGS_ms_logging_drive;

extern int FLAGS_ms_logging_sigmoid_storage_director;
extern int FLAGS_ms_logging_sigmoid_core;
extern int FLAGS_ms_logging_sigmoid_pro;


#endif  // MAIDSAFE_COMMON_LOG_H_
