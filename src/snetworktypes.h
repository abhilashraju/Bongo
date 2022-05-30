#pragma once
#include <string>
#include <algorithm>
#include <map>
#include <numeric>
#include <chrono>
#include "snetworktimeout.h"
namespace bongo {

template <class T>
class StringHolder {
  public:
    StringHolder() {}
    explicit StringHolder(const std::string& str) : str_(str) {}
    explicit StringHolder(std::string&& str) : str_(std::move(str)) {}
    explicit StringHolder(const char* str) : str_(str) {}
    StringHolder(const char* str, size_t len) : str_(str, len) {}
    StringHolder(const std::initializer_list<std::string> args) {
        str_ = std::accumulate(args.begin(), args.end(), str_);
    }
    virtual ~StringHolder() = default;

    operator std::string() const {
        return str_;
    }

    T operator+(const char* rhs) const {
        return T(str_ + rhs);
    }
    T operator+(const std::string& rhs) const {
        return T(str_ + rhs);
    }
    T operator+(const StringHolder<T>& rhs) const {
        return T(str_ + rhs.str_);
    }

    void operator+=(const char* rhs) {
        str_ += rhs;
    }
    void operator+=(const std::string& rhs) {
        str_ += rhs;
    }
    void operator+=(const StringHolder<T>& rhs) {
        str_ += rhs;
    }

    bool operator==(const char* rhs) const {
        return str_.compare(rhs) == 0;
    }
    bool operator==(const std::string& rhs) const {
        return str_ == rhs;
    }
    bool operator==(const StringHolder<T>& rhs) const {
        return str_ == rhs.str_;
    }

    bool operator!=(const char* rhs) const {
        return str_.c_str() != rhs;
    }
    bool operator!=(const std::string& rhs) const {
        return str_ != rhs;
    }
    bool operator!=(const StringHolder<T>& rhs) const {
        return str_ != rhs.str_;
    }

    const std::string& str() {
        return str_;
    }
    const std::string& str() const {
        return str_;
    }
    const char* c_str() const {
        return str_.c_str();
    }
    const char* data() const {
        return str_.data();
    }

  protected:
    std::string str_{};
};

template <class T>
std::ostream& operator<<(std::ostream& os, const StringHolder<T>& s) {
    os << s.str();
    return os;
}

class Url : public StringHolder<Url> {
  public:
    Url() : StringHolder<Url>() {}
    Url(const std::string& url) : StringHolder<Url>(url) {}
    Url(std::string&& url) : StringHolder<Url>(std::move(url)) {}
    Url(const char* url) : StringHolder<Url>(url) {}
    Url(const char* str, size_t len) : StringHolder<Url>(std::string(str, len)) {}
    Url(const std::initializer_list<std::string> args) : StringHolder<Url>(args) {}
    ~Url() = default;
};
struct Port : public StringHolder<Port> {
  public:
    Port(const std::string& p) : StringHolder<Port>(p) {}
    Port(std::string&& p) : StringHolder<Port>(std::move(p)) {}
    Port(const char* p) : StringHolder<Port>(p) {}
    Port(const char* str, size_t len) : StringHolder<Port>(std::string(str, len)) {}
    ~Port()  = default;
};
struct Version : public StringHolder<Version> {
  public:
    Version(const std::string& p) : StringHolder<Version>(p) {}
    Version(std::string&& p) : StringHolder<Version>(std::move(p)) {}
    Version(const char* p) : StringHolder<Version>(p) {}
    Version(const char* str, size_t len) : StringHolder<Version>(std::string(str, len)) {}
    ~Version()  = default;
};
struct Target : public StringHolder<Target> {
  public:
    Target(const std::string& p) : StringHolder<Target>(p) {}
    Target(std::string&& p) : StringHolder<Target>(std::move(p)) {}
    Target(const char* p) : StringHolder<Target>(p) {}
    Target(const char* str, size_t len) : StringHolder<Target>(std::string(str, len)) {}
    ~Target()  = default;
};
struct CaseInsensitiveCompare {
    bool operator()(const std::string& a, const std::string& b) const noexcept
    {
        return a.size() == b.size() && std::equal(begin(a),end(a),begin(b),[](auto c1,auto c2){
            return std::toupper(c1)==std::toupper(c2);
        });
    }
};
struct IdentityValidator{
    std::string operator()(const std::string& v)const{return v;}
};
using HttpHeader = std::map<std::string, std::string, CaseInsensitiveCompare>;

class Body : public StringHolder<Body> {
  public:
    Body() : StringHolder<Body>() {}
    Body(const std::string& body) : StringHolder<Body>(body) {}
    Body(std::string&& body) : StringHolder<Body>(std::move(body)) {}
    Body(const char* body) : StringHolder<Body>(body) {}
    Body(const char* str, size_t len) : StringHolder<Body>(str, len) {}
    Body(const std::initializer_list<std::string> args) : StringHolder<Body>(args) {}
    ~Body() override = default;
};
class ContentType : public StringHolder<ContentType> {
  public:
    ContentType() : StringHolder<ContentType>() {}
    ContentType(const std::string& t) : StringHolder<ContentType>(t) {}
    ContentType(std::string&& t) : StringHolder<ContentType>(std::move(t)) {}
    ContentType(const char* t) : StringHolder<ContentType>(t) {}
    ContentType(const char* str, size_t len) : StringHolder<ContentType>(str, len) {}
    ContentType(const std::initializer_list<std::string> args) : StringHolder<ContentType>(args) {}
    ~ContentType() override = default;
};
class UserAgent : public StringHolder<UserAgent> {
  public:
    UserAgent() : StringHolder<UserAgent>() {}
    UserAgent(const std::string& useragent) : StringHolder<UserAgent>(useragent) {}
    UserAgent(std::string&& useragent) : StringHolder<UserAgent>(std::move(useragent)) {}
    UserAgent(const char* useragent) : StringHolder<UserAgent>(useragent) {}
    UserAgent(const char* str, size_t len) : StringHolder<UserAgent>(str, len) {}
    UserAgent(const std::initializer_list<std::string> args) : StringHolder<UserAgent>(args) {}
    ~UserAgent() override = default;
};
class MaxRedirects {
  public:
    explicit MaxRedirects(const std::int32_t number_of_redirects)
            : number_of_redirects(number_of_redirects) {}

    std::int32_t number_of_redirects;
};

class LowSpeed {
  public:
    LowSpeed(const std::int32_t limit, const std::int32_t time) : limit(limit), time(time) {}

    std::int32_t limit;
    std::int32_t time;
};
class LimitRate {
  public:
    LimitRate(const std::int64_t downrate, const std::int64_t uprate)
            : downrate(downrate), uprate(uprate) {}

    std::int64_t downrate = 0;
    std::int64_t uprate = 0;
};

class Verbose {
  public:
    Verbose() = default;
    // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
    Verbose(const bool verbose) : verbose{verbose} {}

    bool verbose = true;
};
class UnixSocket {
  public:
    // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
    UnixSocket(std::string&& unix_socket) : unix_socket_(std::move(unix_socket)) {}

    const char* GetUnixSocketString() const noexcept;

  private:
    const std::string unix_socket_;
};
// #if LIBCURL_VERSION_NUM >= 0x073D00
class Bearer {
  public:
    // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
    Bearer(const std::string& token) : token_string_{token} {}
    // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
    Bearer(std::string&& token) : token_string_{std::move(token)} {}
    Bearer(const Bearer& other) = default;
    Bearer(Bearer&& old) noexcept = default;
    virtual ~Bearer() noexcept = default;

    Bearer& operator=(Bearer&& old) noexcept = default;
    Bearer& operator=(const Bearer& other) = default;

    virtual const char* GetToken() const noexcept;

  protected:
    std::string token_string_;
};
// #endif
} // namespace Ui
