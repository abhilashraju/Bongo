#pragma once

#include <cstdint>
#include <string>

#include "snetworktypes.h"
#include <utility>
#include <boost/system/error_code.hpp>  // bring in boost::system::error_code et al
#include <iostream>
#include <string>  // for string printing

#ifdef OK
#undef OK
#endif
namespace Ui {

enum class ErrorCode {
    OK = 0,
    CONNECTION_FAILURE,
    EMPTY_RESPONSE,
    HOST_RESOLUTION_FAILURE,
    INTERNAL_ERROR,
    INVALID_URL_FORMAT,
    NETWORK_RECEIVE_ERROR,
    NETWORK_SEND_FAILURE,
    OPERATION_TIMEDOUT,
    PROXY_RESOLUTION_FAILURE,
    SSL_CONNECT_ERROR,
    SSL_LOCAL_CERTIFICATE_ERROR,
    SSL_REMOTE_CERTIFICATE_ERROR,
    SSL_CACERT_ERROR,
    GENERIC_SSL_ERROR,
    UNSUPPORTED_PROTOCOL,
    REQUEST_CANCELLED,
    UNKNOWN_ERROR = 1000,
};

class Error {
  public:
    ErrorCode code = ErrorCode::OK;
    std::string message{};

    Error() = default;

    Error(const std::int32_t& curl_code, std::string&& p_error_message)
            : code{getErrorCodeForCurlError(curl_code)},
              message(std::move(p_error_message)) {}

    explicit operator bool() const {
        return code != ErrorCode::OK;
    }

  private:
    static ErrorCode getErrorCodeForCurlError(std::int32_t curl_code);
};

} // namespace Ui


namespace boost
{
  namespace system
  {
    // Tell the C++ 11 STL metaprogramming that enum ConversionErrc
    // is registered with the standard error code system
    template <> struct is_error_code_enum<Ui::ErrorCode> : std::true_type
    {
    };
  }  // namespace system
}  // namespace boost

namespace detail
{
  // Define a custom error code category derived from boost::system::error_category
  class ConversionErrc_category : public boost::system::error_category
  {
  public:
    // Return a short descriptive name for the category
    virtual const char *name() const noexcept override final { return "Curl Error"; }
    // Return what each enum means in text
    virtual std::string message(int c) const override final
    {
      switch(static_cast<Ui::ErrorCode>(c))
      {
      case Ui::ErrorCode::OK:
        return "success";
      case Ui::ErrorCode::CONNECTION_FAILURE:
            return "case CONNECTION_FAILURE";
      case Ui::ErrorCode::EMPTY_RESPONSE:
            return "case EMPTY_RESPONSE";
      case Ui::ErrorCode::HOST_RESOLUTION_FAILURE:
            return "case HOST_RESOLUTION_FAILURE";
      case Ui::ErrorCode::INTERNAL_ERROR:
            return "case INTERNAL_ERROR";
      case Ui::ErrorCode::INVALID_URL_FORMAT:
            return "case INVALID_URL_FORMAT";
      case Ui::ErrorCode::NETWORK_RECEIVE_ERROR:
            return "case NETWORK_RECEIVE_ERROR";
      case Ui::ErrorCode::NETWORK_SEND_FAILURE:
            return "case NETWORK_SEND_FAILURE";
      case Ui::ErrorCode::OPERATION_TIMEDOUT:
            return "case OPERATION_TIMEDOUT";
      case Ui::ErrorCode::PROXY_RESOLUTION_FAILURE:
            return "case PROXY_RESOLUTION_FAILURE";
      case Ui::ErrorCode::SSL_CONNECT_ERROR:
            return "case SSL_CONNECT_ERROR";
      case Ui::ErrorCode::SSL_LOCAL_CERTIFICATE_ERROR:
            return "case SSL_LOCAL_CERTIFICATE_ERROR";
      case Ui::ErrorCode::SSL_REMOTE_CERTIFICATE_ERROR:
            return "case SSL_REMOTE_CERTIFICATE_ERROR";
      case Ui::ErrorCode::SSL_CACERT_ERROR:
            return "case SSL_CACERT_ERROR";
      case Ui::ErrorCode::GENERIC_SSL_ERROR:
            return "case GENERIC_SSL_ERROR";
      case Ui::ErrorCode::UNSUPPORTED_PROTOCOL:
            return "case UNSUPPORTED_PROTOCOL";
      default:
        return "unknown";
      }
    }
    // OPTIONAL: Allow generic error conditions to be compared to me
    virtual boost::system::error_condition default_error_condition(int c) const noexcept override final
    {
      switch(static_cast<Ui::ErrorCode>(c))
      {
      case Ui::ErrorCode::OK:
        return make_error_condition(boost::system::errc::invalid_argument);
      case Ui::ErrorCode::CONNECTION_FAILURE:
        return make_error_condition(boost::system::errc::invalid_argument);
      case Ui::ErrorCode::INVALID_URL_FORMAT:
        return make_error_condition(boost::system::errc::result_out_of_range);
      default:
        // I have no mapping for this code
        return boost::system::error_condition(c, *this);
      }
    }
  };
}  // namespace detail
// Define the linkage for this function to be used by external code.
// This would be the usual __declspec(dllexport) or __declspec(dllimport)
// if we were in a Windows DLL etc. But for this example use a global
// instance but with inline linkage so multiple definitions do not collide.
#define THIS_MODULE_API_DECL extern inline

// Declare a global function returning a static instance of the custom category
THIS_MODULE_API_DECL const detail::ConversionErrc_category &ConversionErrc_category()
{
static detail::ConversionErrc_category c;
return c;
}

