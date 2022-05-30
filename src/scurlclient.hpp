#pragma once
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <string>

#include <curl/curl.h>

#include "snetworktypes.h"
#include "sauthstring.h"
#include "shttpparameters.h"
#include "spayload.h"
#include "shttpparameters.h"
#include "ssloptions.h"
#include "smultipart.h"
#include "shttpproxies.h"
#include "ssloptions.h"
#include "httpstatuscode.h"
#include "scookies.h"
#include "scurlholder.h"
#include "shttperror.h"
#include "shttputil.h"
namespace Ui {
class Response {
  private:
    std::shared_ptr<CurlHolder> curl_{nullptr};
  public:
        long status_code{};
        std::string text{};
        HttpHeader header{};
        Url url{};
        double elapsed{};
        Cookies cookies{};
        Error error{};
        std::string raw_header{};
        std::string status_line{};
        std::string reason{};
        curl_off_t uploaded_bytes{};
        curl_off_t downloaded_bytes{};
        // Ignored here since libcu
        // NOLINTNEXTLINE(google-ru
        long redirect_count{};
        Response() = default;
        Response(std::shared_ptr<CurlHolder> curl, std::string&& p_text, std::string&& p_header_string,
                Cookies&& p_cookies, Error&& p_error)
                : curl_(std::move(curl)), text(std::move(p_text)), cookies(std::move(p_cookies)),
                    error(std::move(p_error)) {
                header = Ui::util::parseHeader(p_header_string, &status_line, &reason);
                assert(curl_);
                assert(curl_->handle);
                curl_easy_getinfo(curl_->handle, CURLINFO_RESPONSE_CODE, &status_code);
                curl_easy_getinfo(curl_->handle, CURLINFO_TOTAL_TIME, &elapsed);
                char* url_string{nullptr};
                curl_easy_getinfo(curl_->handle, CURLINFO_EFFECTIVE_URL, &url_string);
                url = Url(url_string);
            #if LIBCURL_VERSION_NUM >= 0x073700
                curl_easy_getinfo(curl_->handle, CURLINFO_SIZE_DOWNLOAD_T, &downloaded_bytes);
                curl_easy_getinfo(curl_->handle, CURLINFO_SIZE_UPLOAD_T, &uploaded_bytes);
            #else
                double downloaded_bytes_double, uploaded_bytes_double;
                curl_easy_getinfo(curl_->handle, CURLINFO_SIZE_DOWNLOAD, &downloaded_bytes_double);
                curl_easy_getinfo(curl_->handle, CURLINFO_SIZE_UPLOAD, &uploaded_bytes_double);
                downloaded_bytes = downloaded_bytes_double;
                uploaded_bytes = uploaded_bytes_double;
            #endif
                curl_easy_getinfo(curl_->handle, CURLINFO_REDIRECT_COUNT, &redirect_count);
            }

        std::vector<std::string> GetCertInfo(){
            assert(curl_);
            assert(curl_->handle);
            curl_certinfo* ci{nullptr};
            curl_easy_getinfo(curl_->handle, CURLINFO_CERTINFO, &ci);

            std::vector<std::string> info;
            info.resize(ci->num_of_certs);
            for (int i = 0; i < ci->num_of_certs; i++) {
                // No way around here.
                // NOLINTNEXTLINE (cppcoreguidelines-pro-bounds-pointer-arithmetic)
                info[i] = std::string{ci->certinfo[i]->data};
            }

            return info;
        }
        Response(const Response& other) = default;
        Response(Response&& old) noexcept = default;
        ~Response() noexcept = default;

        Response& operator=(Response&& old) noexcept = default;
        Response& operator=(const Response& other) = default;
};
// Ignored here since libcurl reqires a long:
// NOLINTNEXTLINE(google-runtime-int)
constexpr long ON = 1L;
// Ignored here since libcurl reqires a long:
// NOLINTNEXTLINE(google-runtime-int)
constexpr long OFF = 0L;

class SCurlSession {
  public:
    SCurlSession();

    void SetUrl(const Url& url);
    void SetParameters(const Parameters& parameters);
    void SetParameters(Parameters&& parameters);
    void SetHeader(const HttpHeader& header);
    void UpdateHeader(const HttpHeader& header);
    void SetTimeout(const Timeout& timeout);
    void SetConnectTimeout(const ConnectTimeout& timeout);
    void SetAuth(const Authentication& auth);
// Only supported with libcurl >= 7.61.0.
// As an alternative use SetHeader and add the token manually.
#if LIBCURL_VERSION_NUM >= 0x073D00
    void SetBearer(const Bearer& token);
#endif
    void SetDigest(const Digest& auth);
    void SetUserAgent(const UserAgent& ua);
    void SetPayload(Payload&& payload);
    void SetPayload(const Payload& payload);
    void SetProxies(Proxies&& proxies);
    void SetProxies(const Proxies& proxies);
    void SetMultipart(Multipart&& multipart);
    void SetMultipart(const Multipart& multipart);
    void SetNTLM(const NTLM& auth);
    void SetRedirect(const bool& redirect);
    void SetMaxRedirects(const MaxRedirects& max_redirects);
    void SetCookies(const Cookies& cookies);
    void SetBody(Body&& body);
    void SetBody(const Body& body);
    void SetReadCallback(const ReadCallback& read);
    void SetHeaderCallback(const HeaderCallback& header);
    void SetWriteCallback(const WriteCallback& write);
    void SetProgressCallback(const ProgressCallback& progress);
    void SetDebugCallback(const DebugCallback& debug);
    void SetLowSpeed(const LowSpeed& low_speed);
    void SetVerifySsl(const VerifySsl& verify);
    void SetLimitRate(const LimitRate& limit_rate);
    void SetUnixSocket(const UnixSocket& unix_socket);
    void SetVerbose(const Verbose& verbose);
    void SetSslOptions(const SslOptions& options);

    Response Delete();
    Response Download(const WriteCallback& write);
    Response Download(std::ofstream& file);
    Response Get();
    Response Head();
    Response Options();
    Response Patch();
    Response Post();
    Response Put();

    std::shared_ptr<CurlHolder> GetCurlHolder();

  private:
    void SetHeaderInternal();
    bool hasBodyOrPayload_{false};

    std::shared_ptr<CurlHolder> curl_;
    Url url_;
    Parameters parameters_;
    Proxies proxies_;
    HttpHeader header_;
    /**
     * Will be set by the read callback.
     * Ensures that the "Transfer-Encoding" is set to "chunked", if not overriden in header_.
     **/
    bool chunkedTransferEncoding{false};

    ReadCallback readcb_;
    HeaderCallback headercb_;
    WriteCallback writecb_;
    ProgressCallback progresscb_;
    DebugCallback debugcb_;

    Response makeDownloadRequest();
    Response makeRequest();
    static void freeHolder(CurlHolder* holder);
};

inline SCurlSession::SCurlSession() : curl_(new CurlHolder()) {
    // Set up some sensible defaults
    curl_version_info_data* version_info = curl_version_info(CURLVERSION_NOW);
    std::string version = "curl/" + std::string{version_info->version};
    curl_easy_setopt(curl_->handle, CURLOPT_USERAGENT, version.c_str());
    curl_easy_setopt(curl_->handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_->handle, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl_->handle, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(curl_->handle, CURLOPT_ERRORBUFFER, curl_->error.data());
    curl_easy_setopt(curl_->handle, CURLOPT_COOKIEFILE, "");
#ifdef CPR_CURL_NOSIGNAL
    curl_easy_setopt(curl_->handle, CURLOPT_NOSIGNAL, 1L);
#endif

#if LIBCURL_VERSION_MAJOR >= 7
#if LIBCURL_VERSION_MINOR >= 25
#if LIBCURL_VERSION_PATCH >= 0
    curl_easy_setopt(curl_->handle, CURLOPT_TCP_KEEPALIVE, 1L);
#endif
#endif
#endif
}

inline void SCurlSession::SetUrl(const Url& url) {
    url_ = url;
}

inline void SCurlSession::SetParameters(const Parameters& parameters) {
    parameters_ = parameters;
}

inline void SCurlSession::SetParameters(Parameters&& parameters) {
    parameters_ = std::move(parameters);
}

inline void SCurlSession::SetHeaderInternal() {
    curl_slist* chunk = nullptr;
    for (const std::pair<const std::string, std::string>& item : header_) {
        std::string header_string = item.first;
        if (item.second.empty()) {
            header_string += ";";
        } else {
            header_string += ": " + item.second;
        }

        curl_slist* temp = curl_slist_append(chunk, header_string.c_str());
        if (temp) {
            chunk = temp;
        }
    }

    // Set the chunked transfer encoding in case it does not already exist:
    if (chunkedTransferEncoding && header_.find("Transfer-Encoding") == header_.end()) {
        curl_slist* temp = curl_slist_append(chunk, "Transfer-Encoding:chunked");
        if (temp) {
            chunk = temp;
        }
    }

    curl_easy_setopt(curl_->handle, CURLOPT_HTTPHEADER, chunk);

    curl_slist_free_all(curl_->chunk);
    curl_->chunk = chunk;
}

inline void SCurlSession::SetHeader(const HttpHeader& header) {
    header_ = header;
}

inline void SCurlSession::UpdateHeader(const HttpHeader& header) {
    for (const std::pair<const std::string, std::string>& item : header) {
        header_[item.first] = item.second;
    }
}

inline void SCurlSession::SetTimeout(const Timeout& timeout) {
    curl_easy_setopt(curl_->handle, CURLOPT_TIMEOUT_MS, timeout.Milliseconds());
}

inline void SCurlSession::SetConnectTimeout(const ConnectTimeout& timeout) {
    curl_easy_setopt(curl_->handle, CURLOPT_CONNECTTIMEOUT_MS, timeout.Milliseconds());
}

inline void SCurlSession::SetVerbose(const Verbose& verbose) {
    curl_easy_setopt(curl_->handle, CURLOPT_VERBOSE, verbose.verbose ? ON : OFF);
}

inline void SCurlSession::SetAuth(const Authentication& auth) {
    // Ignore here since this has been defined by libcurl.
    curl_easy_setopt(curl_->handle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    curl_easy_setopt(curl_->handle, CURLOPT_USERPWD, auth.GetAuthString());
}

// Only supported with libcurl >= 7.61.0.
// As an alternative use SetHeader and add the token manually.
#if LIBCURL_VERSION_NUM >= 0x073D00
inline void SCurlSession::SetBearer(const Bearer& token) {
    // Ignore here since this has been defined by libcurl.
    curl_easy_setopt(curl_->handle, CURLOPT_HTTPAUTH, CURLAUTH_BEARER);
    curl_easy_setopt(curl_->handle, CURLOPT_XOAUTH2_BEARER, token.GetToken());
}
#endif

inline void SCurlSession::SetDigest(const Digest& auth) {
    // Ignore here since this has been defined by libcurl.
    curl_easy_setopt(curl_->handle, CURLOPT_HTTPAUTH, CURLAUTH_DIGEST);
    curl_easy_setopt(curl_->handle, CURLOPT_USERPWD, auth.GetAuthString());
}

inline void SCurlSession::SetUserAgent(const UserAgent& ua) {
    curl_easy_setopt(curl_->handle, CURLOPT_USERAGENT, ua.c_str());
}

inline void SCurlSession::SetPayload(Payload&& payload) {
    hasBodyOrPayload_ = true;
    const std::string content = payload.GetContent(*curl_);
    curl_easy_setopt(curl_->handle, CURLOPT_POSTFIELDSIZE_LARGE,
                     static_cast<curl_off_t>(content.length()));
    curl_easy_setopt(curl_->handle, CURLOPT_COPYPOSTFIELDS, content.c_str());
}

inline void SCurlSession::SetPayload(const Payload& payload) {
    hasBodyOrPayload_ = true;
    const std::string content = payload.GetContent(*curl_);
    curl_easy_setopt(curl_->handle, CURLOPT_POSTFIELDSIZE_LARGE,
                     static_cast<curl_off_t>(content.length()));
    curl_easy_setopt(curl_->handle, CURLOPT_COPYPOSTFIELDS, content.c_str());
}

inline void SCurlSession::SetProxies(const Proxies& proxies) {
    proxies_ = proxies;
}

inline void SCurlSession::SetProxies(Proxies&& proxies) {
    proxies_ = std::move(proxies);
}

inline void SCurlSession::SetMultipart(Multipart&& multipart) {
    curl_httppost* formpost = nullptr;
    curl_httppost* lastptr = nullptr;

    for (const Part& part : multipart.parts) {
        std::vector<curl_forms> formdata;
        if (part.is_buffer) {
            // Do not use formdata, to prevent having to use reinterpreter_cast:
            curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, part.name.c_str(), CURLFORM_BUFFER,
                         part.value.c_str(), CURLFORM_BUFFERPTR, part.data, CURLFORM_BUFFERLENGTH,
                         part.datalen, CURLFORM_END);
        } else {
            formdata.push_back({CURLFORM_COPYNAME, part.name.c_str()});
            if (part.is_file) {
                formdata.push_back({CURLFORM_FILE, part.value.c_str()});
            } else {
                formdata.push_back({CURLFORM_COPYCONTENTS, part.value.c_str()});
            }
        }
        if (!part.content_type.empty()) {
            formdata.push_back({CURLFORM_CONTENTTYPE, part.content_type.c_str()});
        }

        formdata.push_back({CURLFORM_END, nullptr});
        curl_formadd(&formpost, &lastptr, CURLFORM_ARRAY, formdata.data(), CURLFORM_END);
    }
    curl_easy_setopt(curl_->handle, CURLOPT_HTTPPOST, formpost);
    hasBodyOrPayload_ = true;

    curl_formfree(curl_->formpost);
    curl_->formpost = formpost;
}

inline void SCurlSession::SetMultipart(const Multipart& multipart) {
    curl_httppost* formpost = nullptr;
    curl_httppost* lastptr = nullptr;

    for (const Part& part : multipart.parts) {
        std::vector<curl_forms> formdata;
        if (part.is_buffer) {
            // Do not use formdata, to prevent having to use reinterpreter_cast:
            curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, part.name.c_str(), CURLFORM_BUFFER,
                         part.value.c_str(), CURLFORM_BUFFERPTR, part.data, CURLFORM_BUFFERLENGTH,
                         part.datalen, CURLFORM_END);
        } else {
            formdata.push_back({CURLFORM_COPYNAME, part.name.c_str()});
            if (part.is_file) {
                formdata.push_back({CURLFORM_FILE, part.value.c_str()});
            } else {
                formdata.push_back({CURLFORM_COPYCONTENTS, part.value.c_str()});
            }
        }
        if (!part.content_type.empty()) {
            formdata.push_back({CURLFORM_CONTENTTYPE, part.content_type.c_str()});
        }

        formdata.push_back({CURLFORM_END, nullptr});
        curl_formadd(&formpost, &lastptr, CURLFORM_ARRAY, formdata.data(), CURLFORM_END);
    }
    curl_easy_setopt(curl_->handle, CURLOPT_HTTPPOST, formpost);
    hasBodyOrPayload_ = true;

    curl_formfree(curl_->formpost);
    curl_->formpost = formpost;
}

inline void SCurlSession::SetLimitRate(const LimitRate& limit_rate) {
    curl_easy_setopt(curl_->handle, CURLOPT_MAX_RECV_SPEED_LARGE, limit_rate.downrate);
    curl_easy_setopt(curl_->handle, CURLOPT_MAX_SEND_SPEED_LARGE, limit_rate.uprate);
}

inline void SCurlSession::SetNTLM(const NTLM& auth) {
    // Ignore here since this has been defined by libcurl.
    curl_easy_setopt(curl_->handle, CURLOPT_HTTPAUTH, CURLAUTH_NTLM);
    curl_easy_setopt(curl_->handle, CURLOPT_USERPWD, auth.GetAuthString());
}

inline void SCurlSession::SetRedirect(const bool& redirect) {
    curl_easy_setopt(curl_->handle, CURLOPT_FOLLOWLOCATION, std::int32_t(redirect));
}

inline void SCurlSession::SetMaxRedirects(const MaxRedirects& max_redirects) {
    curl_easy_setopt(curl_->handle, CURLOPT_MAXREDIRS, max_redirects.number_of_redirects);
}

inline void SCurlSession::SetCookies(const Cookies& cookies) {
    curl_easy_setopt(curl_->handle, CURLOPT_COOKIELIST, "ALL");
    curl_easy_setopt(curl_->handle, CURLOPT_COOKIE, cookies.GetEncoded(*curl_).c_str());
}

inline void SCurlSession::SetBody(Body&& body) {
    hasBodyOrPayload_ = true;
    curl_easy_setopt(curl_->handle, CURLOPT_POSTFIELDSIZE_LARGE,
                     static_cast<curl_off_t>(body.str().length()));
    curl_easy_setopt(curl_->handle, CURLOPT_COPYPOSTFIELDS, body.c_str());
}

inline void SCurlSession::SetBody(const Body& body) {
    hasBodyOrPayload_ = true;
    curl_easy_setopt(curl_->handle, CURLOPT_POSTFIELDSIZE_LARGE,
                     static_cast<curl_off_t>(body.str().length()));
    curl_easy_setopt(curl_->handle, CURLOPT_POSTFIELDS, body.c_str());
}

inline void SCurlSession::SetReadCallback(const ReadCallback& read) {
    readcb_ = read;
    curl_easy_setopt(curl_->handle, CURLOPT_INFILESIZE_LARGE, read.size);
    curl_easy_setopt(curl_->handle, CURLOPT_POSTFIELDSIZE_LARGE, read.size);
    curl_easy_setopt(curl_->handle, CURLOPT_READFUNCTION, Ui::util::readUserFunction);
    curl_easy_setopt(curl_->handle, CURLOPT_READDATA, &readcb_);
    chunkedTransferEncoding = read.size == -1;
}

inline void SCurlSession::SetHeaderCallback(const HeaderCallback& header) {
    curl_easy_setopt(curl_->handle, CURLOPT_HEADERFUNCTION, Ui::util::headerUserFunction);
    headercb_ = header;
    curl_easy_setopt(curl_->handle, CURLOPT_HEADERDATA, &headercb_);
}

inline void SCurlSession::SetWriteCallback(const WriteCallback& write) {
    curl_easy_setopt(curl_->handle, CURLOPT_WRITEFUNCTION, Ui::util::writeUserFunction);
    writecb_ = write;
    curl_easy_setopt(curl_->handle, CURLOPT_WRITEDATA, &writecb_);
}

inline void SCurlSession::SetProgressCallback(const ProgressCallback& progress) {
    progresscb_ = progress;
#if LIBCURL_VERSION_NUM < 0x072000
    curl_easy_setopt(curl_->handle, CURLOPT_PROGRESSFUNCTION, Ui::util::progressUserFunction);
    curl_easy_setopt(curl_->handle, CURLOPT_PROGRESSDATA, &progresscb_);
#else
    curl_easy_setopt(curl_->handle, CURLOPT_XFERINFOFUNCTION, Ui::util::progressUserFunction);
    curl_easy_setopt(curl_->handle, CURLOPT_XFERINFODATA, &progresscb_);
#endif
    curl_easy_setopt(curl_->handle, CURLOPT_NOPROGRESS, 0L);
}

inline void SCurlSession::SetDebugCallback(const DebugCallback& debug) {
    curl_easy_setopt(curl_->handle, CURLOPT_DEBUGFUNCTION, Ui::util::debugUserFunction);
    debugcb_ = debug;
    curl_easy_setopt(curl_->handle, CURLOPT_DEBUGDATA, &debugcb_);
    curl_easy_setopt(curl_->handle, CURLOPT_VERBOSE, 1L);
}

inline void SCurlSession::SetLowSpeed(const LowSpeed& low_speed) {
    curl_easy_setopt(curl_->handle, CURLOPT_LOW_SPEED_LIMIT, low_speed.limit);
    curl_easy_setopt(curl_->handle, CURLOPT_LOW_SPEED_TIME, low_speed.time);
}

inline void SCurlSession::SetVerifySsl(const VerifySsl& verify) {
    curl_easy_setopt(curl_->handle, CURLOPT_SSL_VERIFYPEER, verify ? ON : OFF);
    curl_easy_setopt(curl_->handle, CURLOPT_SSL_VERIFYHOST, verify ? 2L : 0L);
}

inline void SCurlSession::SetUnixSocket(const UnixSocket& unix_socket) {
    curl_easy_setopt(curl_->handle, CURLOPT_UNIX_SOCKET_PATH, unix_socket.GetUnixSocketString());
}

inline void SCurlSession::SetSslOptions(const SslOptions& options) {
    if (!options.cert_file.empty()) {
        curl_easy_setopt(curl_->handle, CURLOPT_SSLCERT, options.cert_file.c_str());
        if (!options.cert_type.empty()) {
            curl_easy_setopt(curl_->handle, CURLOPT_SSLCERTTYPE, options.cert_type.c_str());
        }
    }
    if (!options.key_file.empty()) {
        curl_easy_setopt(curl_->handle, CURLOPT_SSLKEY, options.key_file.c_str());
        if (!options.key_type.empty()) {
            curl_easy_setopt(curl_->handle, CURLOPT_SSLKEYTYPE, options.key_type.c_str());
        }
        if (!options.key_pass.empty()) {
            curl_easy_setopt(curl_->handle, CURLOPT_KEYPASSWD, options.key_pass.c_str());
        }
    }
#if SUPPORT_ALPN
    curl_easy_setopt(curl_->handle, CURLOPT_SSL_ENABLE_ALPN, options.enable_alpn ? ON : OFF);
#endif
#if SUPPORT_NPN
    curl_easy_setopt(curl_->handle, CURLOPT_SSL_ENABLE_NPN, options.enable_npn ? ON : OFF);
#endif
    curl_easy_setopt(curl_->handle, CURLOPT_SSL_VERIFYPEER, options.verify_peer ? ON : OFF);
    curl_easy_setopt(curl_->handle, CURLOPT_SSL_VERIFYHOST, options.verify_host ? 2L : 0L);
#if LIBCURL_VERSION_NUM >= 0x072900
    curl_easy_setopt(curl_->handle, CURLOPT_SSL_VERIFYSTATUS, options.verify_status ? ON : OFF);
#endif
    curl_easy_setopt(curl_->handle, CURLOPT_SSLVERSION,
                     // Ignore here since this has been defined by libcurl.
                     options.ssl_version
#if SUPPORT_MAX_TLS_VERSION
                             | options.max_version
#endif
    );
#if SUPPORT_SSL_NO_REVOKE
    if (options.ssl_no_revoke) {
        curl_easy_setopt(curl_->handle, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NO_REVOKE);
    }
#endif
    if (!options.ca_info.empty()) {
        curl_easy_setopt(curl_->handle, CURLOPT_CAINFO, options.ca_info.c_str());
    }
    if (!options.ca_path.empty()) {
        curl_easy_setopt(curl_->handle, CURLOPT_CAPATH, options.ca_path.c_str());
    }
    if (!options.crl_file.empty()) {
        curl_easy_setopt(curl_->handle, CURLOPT_CRLFILE, options.crl_file.c_str());
    }
    if (!options.ciphers.empty()) {
        curl_easy_setopt(curl_->handle, CURLOPT_SSL_CIPHER_LIST, options.ciphers.c_str());
    }
#if SUPPORT_TLSv13_CIPHERS
    if (!options.tls13_ciphers.empty()) {
        curl_easy_setopt(curl_->handle, CURLOPT_TLS13_CIPHERS, options.ciphers.c_str());
    }
#endif
#if SUPPORT_SESSIONID_CACHE
    curl_easy_setopt(curl_->handle, CURLOPT_SSL_SESSIONID_CACHE,
                     options.session_id_cache ? ON : OFF);
#endif
}

inline Response SCurlSession::Delete() {
    curl_easy_setopt(curl_->handle, CURLOPT_HTTPGET, 0L);
    curl_easy_setopt(curl_->handle, CURLOPT_NOBODY, 0L);
    curl_easy_setopt(curl_->handle, CURLOPT_CUSTOMREQUEST, "DELETE");

    return makeRequest();
}

inline Response SCurlSession::Download(const WriteCallback& write) {
    curl_easy_setopt(curl_->handle, CURLOPT_NOBODY, 0L);
    curl_easy_setopt(curl_->handle, CURLOPT_CUSTOMREQUEST, "GET");

    SetWriteCallback(write);

    return makeDownloadRequest();
}

inline Response SCurlSession::Download(std::ofstream& file) {
    curl_easy_setopt(curl_->handle, CURLOPT_NOBODY, 0L);
    curl_easy_setopt(curl_->handle, CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(curl_->handle, CURLOPT_WRITEFUNCTION, Ui::util::writeFileFunction);
    curl_easy_setopt(curl_->handle, CURLOPT_WRITEDATA, &file);

    return makeDownloadRequest();
}

inline Response SCurlSession::Get() {
    // In case there is a body or payload for this request, we create a custom GET-Request since a
    // GET-Request with body is based on the HTTP RFC **not** a leagal request.
    if (hasBodyOrPayload_) {
        curl_easy_setopt(curl_->handle, CURLOPT_NOBODY, 0L);
        curl_easy_setopt(curl_->handle, CURLOPT_CUSTOMREQUEST, "GET");
    } else {
        curl_easy_setopt(curl_->handle, CURLOPT_NOBODY, 0L);
        curl_easy_setopt(curl_->handle, CURLOPT_CUSTOMREQUEST, nullptr);
        curl_easy_setopt(curl_->handle, CURLOPT_HTTPGET, 1L);
    }

    return makeRequest();
}

inline Response SCurlSession::Head() {
    curl_easy_setopt(curl_->handle, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl_->handle, CURLOPT_CUSTOMREQUEST, nullptr);

    return makeRequest();
}

inline Response SCurlSession::Options() {
    curl_easy_setopt(curl_->handle, CURLOPT_NOBODY, 0L);
    curl_easy_setopt(curl_->handle, CURLOPT_CUSTOMREQUEST, "OPTIONS");

    return makeRequest();
}

inline Response SCurlSession::Patch() {
    curl_easy_setopt(curl_->handle, CURLOPT_NOBODY, 0L);
    curl_easy_setopt(curl_->handle, CURLOPT_CUSTOMREQUEST, "PATCH");

    return makeRequest();
}

inline Response SCurlSession::Post() {
    curl_easy_setopt(curl_->handle, CURLOPT_NOBODY, 0L);

    // In case there is no body or payload set it to an empty post:
    if (hasBodyOrPayload_) {
        curl_easy_setopt(curl_->handle, CURLOPT_CUSTOMREQUEST, nullptr);
    } else {
        curl_easy_setopt(curl_->handle, CURLOPT_POSTFIELDS, readcb_.callback ? nullptr : "");
        curl_easy_setopt(curl_->handle, CURLOPT_CUSTOMREQUEST, "POST");
    }

    return makeRequest();
}

inline Response SCurlSession::Put() {
    curl_easy_setopt(curl_->handle, CURLOPT_NOBODY, 0L);
    curl_easy_setopt(curl_->handle, CURLOPT_CUSTOMREQUEST, "PUT");

    return makeRequest();
}

inline std::shared_ptr<CurlHolder> SCurlSession::GetCurlHolder() {
    return curl_;
}

inline Response SCurlSession::makeDownloadRequest() {
    assert(curl_->handle);
    const std::string parametersContent = parameters_.GetContent(*curl_);
    if (!parametersContent.empty()) {
        Url new_url{url_ + "?" + parametersContent};
        curl_easy_setopt(curl_->handle, CURLOPT_URL, new_url.c_str());
    } else {
        curl_easy_setopt(curl_->handle, CURLOPT_URL, url_.c_str());
    }

    std::string protocol = url_.str().substr(0, url_.str().find(':'));
    if (proxies_.has(protocol)) {
        curl_easy_setopt(curl_->handle, CURLOPT_PROXY, proxies_[protocol].c_str());
    } else {
        curl_easy_setopt(curl_->handle, CURLOPT_PROXY, "");
    }

    curl_->error[0] = '\0';

    std::string header_string;
    if (headercb_.callback) {
        curl_easy_setopt(curl_->handle, CURLOPT_HEADERFUNCTION, Ui::util::headerUserFunction);
        curl_easy_setopt(curl_->handle, CURLOPT_HEADERDATA, &headercb_);
    } else {
        curl_easy_setopt(curl_->handle, CURLOPT_HEADERFUNCTION, Ui::util::writeFunction);
        curl_easy_setopt(curl_->handle, CURLOPT_HEADERDATA, &header_string);
    }

    CURLcode curl_error = curl_easy_perform(curl_->handle);

    curl_slist* raw_cookies{nullptr};
    curl_easy_getinfo(curl_->handle, CURLINFO_COOKIELIST, &raw_cookies);
    Cookies cookies = util::parseCookies(raw_cookies);
    curl_slist_free_all(raw_cookies);
    std::string errorMsg = curl_->error.data();

    return Response(curl_, "", std::move(header_string), std::move(cookies),
                    Error(curl_error, std::move(errorMsg)));
}

inline Response SCurlSession::makeRequest() {
    assert(curl_->handle);

    // Set Header:
    SetHeaderInternal();

    const std::string parametersContent = parameters_.GetContent(*curl_);
    if (!parametersContent.empty()) {
        Url new_url{url_ + "?" + parametersContent};
        curl_easy_setopt(curl_->handle, CURLOPT_URL, new_url.c_str());
    } else {
        curl_easy_setopt(curl_->handle, CURLOPT_URL, url_.c_str());
    }

    // Proxy:
    std::string protocol = url_.str().substr(0, url_.str().find(':'));
    if (proxies_.has(protocol)) {
        curl_easy_setopt(curl_->handle, CURLOPT_PROXY, proxies_[protocol].c_str());
    } else {
        curl_easy_setopt(curl_->handle, CURLOPT_PROXY, nullptr);
    }

#if LIBCURL_VERSION_MAJOR >= 7
#if LIBCURL_VERSION_MINOR >= 21
    /* enable all supported built-in compressions */
    curl_easy_setopt(curl_->handle, CURLOPT_ACCEPT_ENCODING, "");
#endif
#endif

    curl_->error[0] = '\0';

    std::string response_string;
    std::string header_string;
    if (!this->writecb_.callback) {
        curl_easy_setopt(curl_->handle, CURLOPT_WRITEFUNCTION, Ui::util::writeFunction);
        curl_easy_setopt(curl_->handle, CURLOPT_WRITEDATA, &response_string);
    }
    if (!this->headercb_.callback) {
        curl_easy_setopt(curl_->handle, CURLOPT_HEADERFUNCTION, Ui::util::writeFunction);
        curl_easy_setopt(curl_->handle, CURLOPT_HEADERDATA, &header_string);
    }

    // Enable so we are able to retrive certificate information:
    curl_easy_setopt(curl_->handle, CURLOPT_CERTINFO, 1L);

    CURLcode curl_error = curl_easy_perform(curl_->handle);

    curl_slist* raw_cookies{nullptr};
    curl_easy_getinfo(curl_->handle, CURLINFO_COOKIELIST, &raw_cookies);
    Cookies cookies = util::parseCookies(raw_cookies);
    curl_slist_free_all(raw_cookies);

    // Reset the has no body property:
    hasBodyOrPayload_ = false;

    std::string errorMsg = curl_->error.data();
    return Response(curl_, std::move(response_string), std::move(header_string), std::move(cookies),
                    Error(curl_error, std::move(errorMsg)));
}

} // namespace cpr
