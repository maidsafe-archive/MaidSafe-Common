/** ==========================================================================
 * 2011 by KjellKod.cc. This is PUBLIC DOMAIN to use at your own risk and comes
 * with no warranties. This code is yours to share, use and modify with no
 * strings attached and no restrictions or obligations.
 * ============================================================================*/

#include "maidsafe/common/log.h"

#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <cstdio>
#include <cassert>
#include <mutex>
#include <thread>
#include <signal.h>
#include <algorithm>
#include "boost/tokenizer.hpp"
#include "maidsafe/log/logworker.h"

namespace maidsafe {

namespace log {

std::string splitFileName(const std::string& str) {
  size_t found;
  found = str.find_last_of("(/\\");
  return str.substr(found+1);
}


LogMessage::LogMessage(const std::string &file,
                       const int line,
                       const std::string& function,
                       const std::string &level)
  : file_(file),
    line_(line),
    function_(function),
    level_(level) {}

LogMessage::~LogMessage() {
  std::ostringstream oss;
//  boost::char_separator<char> seperator(", ");
//  boost::tokenizer< boost::char_separator<char>> tokens(Logging::Filter(), seperator);
//  bool test(false);
//  for(auto &t : tokens) {
//      if (function_.find(t) == std::string::npos)
//        test = true;
//   }
//   if (LogLevel().find(level_) == std::string::npos)
//     return;
//   if (test)
//     return;
   oss << level_ << " [ file " << splitFileName(file_);
   oss << " line:" << line_ ;
   oss << " thread id: " <<  std::this_thread::get_id();
   oss <<  " in function: " << function_ << "] ";
   const std::string str(stream_.str());
   if(!str.empty())
        oss << '"' << str << '"';
  log_entry_ += oss.str();
  auto f = [=] () { std::cout << log_entry_;} ;
  Logging::Send(f); // message saved
}

Logging::Logging() :
    background_(maidsafe::Active::createActive()),
    log_levels_(),
    filter_() {}

void Logging::Send(functor voidfunction) {
  background_->Send(voidfunction);
}


}  // namespace log
}  // namespace maidsafe
