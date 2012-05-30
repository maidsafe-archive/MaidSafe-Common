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
#include "boost/filesystem.hpp"
#include "maidsafe/log/logworker.h"

namespace maidsafe {

namespace log {

LogMessage::LogMessage(const std::string &file,
                       const int line,
                       const std::string& function,
                       const int level)
  : file_(file),
    line_(line),
    function_(function),
    level_(level) {}

LogMessage::~LogMessage() {
  std::ostringstream oss;
    boost::char_separator<char> seperator(", ");
  boost::tokenizer< boost::char_separator<char>> tokens(Logging::instance().Filter(), seperator);

  for(auto &t : tokens) {
      if ((function_.find(t) == std::string::npos) || (Logging::instance().LogLevel() < level_))
        return;
   }

   std::string log_level;
   switch (level_) {
     case 0:
       log_level = "INFO";
       break;
     case 1:
       log_level = "WARNING";
       break;
     case 2:
       log_level = "ERROR";
       break;
     case 3:
       log_level = "FATAL";
       break;
     default:
       log_level = "Unknown";
   }

   oss << log_level << " [" << boost::filesystem3::path(file_).filename().string();
   oss << " :" << line_ ;
   oss << " Thread : " <<  std::this_thread::get_id();
   oss <<  " Function: " << function_ << "] ";
   const std::string str(stream_.str());
   if(!str.empty())
        oss << '"' << str << '"';
  std::string log_entry(oss.str());
  Logging::instance().Send([=] () { std::cout << log_entry << std::endl;} ); // message saved
}

Logging::Logging() :
    background_(maidsafe::Active::createActive()),
    log_levels_(FATAL),  // Default on (fatal
    filter_() {}

void Logging::Send(functor voidfunction) {
  background_->Send(voidfunction);
}


}  // namespace log
}  // namespace maidsafe
