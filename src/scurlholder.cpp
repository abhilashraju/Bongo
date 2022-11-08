#include "scurlholder.h"
#include <cassert>

namespace bongo {
// It does not make sense to make a std::mutex const.
// NOLINTNEXTLINE (cppcoreguidelines-avoid-non-const-global-variables)

inline std::mutex& getStaticMutex() {
  static std::mutex curl_easy_init_mutex_{};
  return curl_easy_init_mutex_;
}

CurlHolder::CurlHolder() {
  /**
   * Allow multithreaded access to CPR by locking curl_easy_init().
   * curl_easy_init() is not thread safe.
   * References:
   * https://curl.haxx.se/libcurl/c/curl_easy_init.html
   * https://curl.haxx.se/libcurl/c/threadsafe.html
   **/
  getStaticMutex().lock();
  handle = curl_easy_init();
  getStaticMutex().unlock();

  assert(handle);
}  // namespace cpr

CurlHolder::~CurlHolder() {
  curl_easy_cleanup(handle);
  curl_slist_free_all(chunk);
  curl_formfree(formpost);
}

inline std::string CurlHolder::urlEncode(const std::string& s) const {
  assert(handle);
  char* output = curl_easy_escape(handle, s.c_str(), s.length());
  if (output) {
    std::string result = output;
    curl_free(output);
    return result;
  }
  return "";
}

inline std::string CurlHolder::urlDecode(const std::string& s) const {
  assert(handle);
  char* output = curl_easy_unescape(handle, s.c_str(), s.length(), nullptr);
  if (output) {
    std::string result = output;
    curl_free(output);
    return result;
  }
  return "";
}
}  // namespace bongo
