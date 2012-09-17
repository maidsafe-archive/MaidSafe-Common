/*******************************************************************************
 *  Copyright 2012 maidsafe.net limited                                        *
 *                                                                             *
 *  The following source code is property of maidsafe.net limited and is not   *
 *  meant for external use.  The use of this code is governed by the licence   *
 *  file licence.txt found in the root of this directory and also on           *
 *  www.maidsafe.net.                                                          *
 *                                                                             *
 *  You are not free to copy, amend or otherwise use this source code without  *
 *  the explicit written permission of the board of directors of maidsafe.net. *
 ******************************************************************************/

#include "maidsafe/common/error.h"
namespace maidsafe {

std::error_code make_error_code(maidsafe_error_code e)  {
  return std::error_code(static_cast<int>(e), error_category());
}

std::error_condition make_error_condition(maidsafe_error_condition e) {
  return std::error_condition(static_cast<int>(e), error_category());
}

const char* error_category_impl::name() const MAIDSAFE_NOEXCEPT {
    return "maidsafe";
}

std::string error_category_impl::message(int ev) const
{
    switch (static_cast<maidsafe_error_code>(ev))
    {
        case maidsafe_error_code::bad_string_length:
            return "String must be length 64";
        case maidsafe_error_code::no_private_key:
            return "Invalid or missing private key";
        case  maidsafe_error_code::no_public_key:
            return "Invalid or missing public key";
        case  maidsafe_error_code::decrypt_failed:
            return "Decrypt failed";
        case  maidsafe_error_code::encrypt_failed:
            return "Encrypt failed";
        case  maidsafe_error_code::no_bootstrap_nodes_found:
            return "Cannot connect to any supplied bootsrtap nodes";
        case  maidsafe_error_code::node_not_found:
            return "Cannot find node";
        case  maidsafe_error_code::listen_failed:
            return "Listen incoming connections failed";
        case  maidsafe_error_code::accept_failed:
            return "Accept connection failed";
        case  maidsafe_error_code::no_transports:
            return "No transports available";
        case  maidsafe_error_code::transports_full:
            return "Tranports full";
        case  maidsafe_error_code::no_available_ports:
              return "No available ports";
        case  maidsafe_error_code::address_in_use:
            return "Address already in use";
        case  maidsafe_error_code::invalid_message:
            return "invalid message";
        case  maidsafe_error_code::drive_not_mounted:
            return "Drive not mounted";
        case  maidsafe_error_code::cannot_mount_drive:
            return "Cannot mount drive";
        case  maidsafe_error_code::no_filesystem_driver:
            return "Filesystem driver not installed";
        case  maidsafe_error_code::save_failed:
            return "Save data failed";
        case  maidsafe_error_code::get_failed:
            return "Get data failed";
        case  maidsafe_error_code::modify_failed:
            return "Modify data failed";
        case  maidsafe_error_code::delete_failed:
            return "Delete data failed";
        default:
            return "Unknown error";
    }
}

std::error_condition
     error_category_impl::default_error_condition(int ev) const MAIDSAFE_NOEXCEPT {
    switch (static_cast<maidsafe_error_code>(ev))
    {
        case  maidsafe_error_code::no_bootstrap_nodes_found:
        case  maidsafe_error_code::node_not_found:
          return maidsafe_error_condition::routing;
        case  maidsafe_error_code::listen_failed:
        case  maidsafe_error_code::accept_failed:
        case  maidsafe_error_code::no_transports:
        case  maidsafe_error_code::transports_full:
        case  maidsafe_error_code::no_available_ports:
        case  maidsafe_error_code::address_in_use:
        case  maidsafe_error_code::invalid_message:
            return maidsafe_error_condition::rudp;
        case  maidsafe_error_code::drive_not_mounted:
        case  maidsafe_error_code::cannot_mount_drive:
        case  maidsafe_error_code::no_filesystem_driver:
          return maidsafe_error_condition::drive;
        default:
            return maidsafe_error_condition::unknown_error;
    }
}

const std::error_category& error_category() {
  static error_category_impl instance;
  return instance;
}
}  // maidsafe
