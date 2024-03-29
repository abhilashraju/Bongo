#pragma once
#include <mutex>
#include <string>
#include <array>
#include <curl/curl.h>

namespace bongo {

struct CurlHolder {
private:
  /**
   * Mutex for curl_easy_init().
   * curl_easy_init() is not thread save.
   * References:
   * https://curl.haxx.se/libcurl/c/curl_easy_init.html
   * https://curl.haxx.se/libcurl/c/threadsafe.html
   **/
  // It does not make sense to make a std::mutex const.
  // NOLINTNEXTLINE (cppcoreguidelines-avoid-non-const-global-variables)

public:
  CURL* handle{nullptr};
  struct curl_slist* chunk{nullptr};
  struct curl_httppost* formpost{nullptr};
  std::array<char, CURL_ERROR_SIZE> error{};

  CurlHolder();
  CurlHolder(const CurlHolder& other) = default;
  CurlHolder(CurlHolder&& old) noexcept = default;
  ~CurlHolder();

  CurlHolder& operator=(CurlHolder&& old) noexcept = default;
  CurlHolder& operator=(const CurlHolder& other) = default;

  /**
   * Uses curl_easy_escape(...) for escaping the given string.
   **/
  std::string urlEncode(const std::string& s) const;

  /**
   * Uses curl_easy_unescape(...) for unescaping the given string.
   **/
  std::string urlDecode(const std::string& s) const;
};
}  // namespace bongo
#include "scurlholder.cpp"
