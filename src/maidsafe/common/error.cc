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

namespace error {
  std::error_code make_error_code(maidsafe_error_code e)  {
    return std::error_code(static_cast<int>(e), error_category());
    }

  std::error_condition make_error_condition(maidsafe_error_condition e) {
    return std::error_condition(static_cast<int>(e), error_category());
    }
 }
#ifdef MIDSAFE_WIN32
const char* error_category_impl::name() const
#else
const char* error_category_impl::name() const noexcept(true)
#endif
{
    return "maidsafe";
}

std::string error_category_impl::message(int ev) const
{
    switch (ev)
    {
        case error::bad_string_length:
            return "String must be length 64";
        case error::no_private_key:
            return "Invalid or missing private key";
        case  error::no_public_key:
            return "Invalid or missing public key";
        case  error::decrypt_failed:
            return "Decrypt failed";
        case  error::encrypt_failed:
            return "Encrypt failed";
        case  error::no_bootstrap_nodes_found:
            return "Cannot connect to any supplied bootsrtap nodes";
        case  error::node_not_found:
            return "Cannot find node";
        case  error::listen_failed:
            return "Listen incoming connections failed";
        case  error::accept_failed:
            return "Accept connection failed";
        case  error::no_transports:
            return "No transports available";
        case  error::transports_full:
            return "Tranports full";
        case  error::no_available_ports:
              return "No available ports";
        case  error::address_in_use:
            return "Address already in use";
        case  error::invalid_message:
            return "invalid message";
        case  error::drive_not_mounted:
            return "Drive not mounted";
        case  error::cannot_mount_drive:
            return "Cannot mount drive";
        case  error::no_filesystem_driver:
            return "Filesystem driver not installed";
        case  error::save_failed:
            return "Save data failed";
        case  error::get_failed:
            return "Get data failed";
        case  error::modify_failed:
            return "Modify data failed";
        case  error::delete_failed:
            return "Delete data failed";
        default:
            return "Unknown error";
    }
}

std::error_condition
    error_category_impl::default_error_condition(int ev) const noexcept(true)
{
    switch (ev)
    {
        case  error::no_bootstrap_nodes_found:
        case  error::node_not_found:
          return error::routing;
        case  error::listen_failed:
        case  error::accept_failed:
        case  error::no_transports:
        case  error::transports_full:
        case  error::no_available_ports:
        case  error::address_in_use:
        case  error::invalid_message:
            return error::rudp;
        case  error::drive_not_mounted:
        case  error::cannot_mount_drive:
        case  error::no_filesystem_driver:
          return error::drive;
        default:
            return std::error_condition(ev, *this);
    }
}

const std::error_category& error_category()
{
    static error_category_impl instance;
    return instance;
}

} // maidsafe

