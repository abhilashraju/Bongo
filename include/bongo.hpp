#pragma once
#include "scurlclient.hpp"
#include "urilite.h"
#include <unifex/then.hpp>
#include <unifex/just.hpp>
#include <unifex/let_error.hpp>
#include <unifex/when_all.hpp>

namespace bongo {
class CurlSession {
  SCurlSession curl_session_;
  Port port_{"80"};
  std::string vers_{"1.0"};
  Target target_{"/"};
  ContentType cont_type_{"text/plain"};

public:
  CurlSession() {}

  void SetOption(const Url& url) { curl_session_.SetUrl(url); }
  void SetOption(const Parameters& parameters) {
    curl_session_.SetParameters(parameters);
  }
  void SetOption(Parameters&& parameters) {
    curl_session_.SetParameters(std::move(parameters));
  }
  void SetOption(const HttpHeader& header) { curl_session_.SetHeader(header); }
  void SetOption(const Timeout& timeout) { curl_session_.SetTimeout(timeout); }
  void SetOption(const ConnectTimeout& timeout) {
    curl_session_.SetConnectTimeout(timeout);
  }
  void SetOption(const Authentication& auth) { curl_session_.SetAuth(auth); }
  void SetOption(const Digest& auth) { curl_session_.SetDigest(auth); }
  void SetOption(const UserAgent& ua) { curl_session_.SetUserAgent(ua); }
  void SetOption(Payload&& payload) {
    curl_session_.SetPayload(std::move(payload));
  }
  void SetOption(const Payload& payload) { curl_session_.SetPayload(payload); }
  void SetOption(Body&& b) { curl_session_.SetBody(std::move(b)); }
  void SetOption(const Body& b) { curl_session_.SetBody(b); }
  void SetOption(const ContentType& t) { cont_type_ = t; }
  void SetOption(ContentType&& t) { cont_type_ = std::move(t); }

  void SetOption(bongo::Port port) { port_ = std::move(port); }
  void SetOption(bongo::Target t) { target_ = std::move(t); }

  void SetOption(const VerifySsl& verify) {
    curl_session_.SetVerifySsl(verify);
  }
  // void SetOption(const LimitRate& limit_rate);
  void SetOption(Proxies&& proxies) {
    curl_session_.SetProxies(std::move(proxies));
  }
  void SetOption(const Proxies& proxies) { curl_session_.SetProxies(proxies); }
  // void SetOption(Multipart&& multipart);
  // void SetOption(const Multipart& multipart);
  // void SetOption(const NTLM& auth);
  // void SetOption(const bool& redirect);
  // void SetOption(const MaxRedirects& max_redirects);
  // void SetOption(const Cookies& cookies);

  template <typename HttpFunc, typename... Args>
  Response call_generic_http(HttpFunc func, Args... args) {
    using expandtype = int[];
    expandtype{(SetOption(args), 0)...};
    return func();
  }
  template <typename... ARGS>
  Response get(ARGS&&... args) {
    // using expandtype=int[];
    // expandtype{(SetOption(args),0)...};
    // return curl_session_.Get();
    return call_generic_http(
        [&]() { return curl_session_.Get(); }, ((ARGS &&)(args))...);
  }
  template <typename... ARGS>
  Response post(ARGS&&... args) {
    return call_generic_http(
        [&]() { return curl_session_.Post(); }, ((ARGS &&)(args))...);
  }

  template <typename... ARGS>
  Response put(ARGS&&... args) {
    return call_generic_http(
        [&]() { return curl_session_.Put(); }, ((ARGS &&)(args))...);
  }
};

struct HttpException : std::exception {
  std::string message;
  HttpException(std::string err) : message(std::move(err)) {}
  const char* what() const noexcept override { return message.data(); }
};
enum class verb { get, post, put };
template <typename... Args>
struct process {
  using Callback = std::function<Response()>;
  Callback callback;
  process(verb v, std::string url, Args... args) {
    auto getcall = [](verb v, std::string url, auto... args) {
      urilite::uri remotepath = urilite::uri::parse(url);
      bongo::CurlSession session;
      auto resp = [&]() {
        switch (v) {
          case verb::get:
            return session.get(
                bongo::Url(urilite::update_query(remotepath)), args...);
            break;
          case verb::post:
            return session.post(
                bongo::Url(urilite::update_query(remotepath)), args...);
            break;
          case verb::put:
            return session.put(
                bongo::Url(urilite::update_query(remotepath)), args...);
            break;
        }
        throw std::runtime_error("Invallid method");
      }();

      if (resp.error.code != ErrorCode::OK) {
        throw HttpException{Error::getErrorMessage(resp.error.code)};
      }
      return resp;
    };
    callback =
        std::bind(getcall, v, std::move(url), std::forward<Args>(args)...);
  }
  template <typename Sender>
  friend auto operator|(Sender sender, process ex) {
    return unifex::then(
        sender, [ex = std::move(ex)]() { return ex.callback(); });
  }
};
inline auto error_to_response(std::exception_ptr err) {
  try {
    std::rethrow_exception(err);
  } catch (const std::exception& e) {
    return unifex::just(std::string(e.what()));
  }
}
template <typename Func>
struct upon_error {
  Func callback;
  upon_error(Func func) : callback(std::move(func)) {}
  template <typename Sender>
  friend auto operator|(Sender sender, upon_error onerror) {
    return sender | unifex::let_error(error_to_response) |
        unifex::then([onerror = std::move(onerror)](auto v) {
             return onerror.callback(v);
           });
  }
};

template <typename... T>
inline auto get_all(T&&... ts) {
  return std::make_tuple(std::get<0>(std::get<0>(ts))...);
}

template <typename Func>
struct then_all {
  Func callback;
  then_all(Func func) : callback(std::move(func)) {}
  template <std::size_t... Indices, typename... Args>
  auto
  deliver_value(std::index_sequence<Indices...>, std::tuple<Args...> t) const {
    return callback(std::get<Indices>(t)...);
  }
  template <typename Sender>
  friend auto operator|(Sender send, then_all t) {
    return send | unifex::then([=](auto... args) {
             return t.deliver_value(
                 std::make_index_sequence<sizeof...(args)>{},
                 get_all((decltype(args)&&)args...));
           });
  }
};

}  // namespace bongo