/*
 * Copyright (c) 2018 Denis Trofimov (den.a.trofimov@yandex.ru)
 * Distributed under the MIT License.
 * See accompanying file LICENSE.md or copy at http://opensource.org/licenses/MIT
 */

// TODO: example_http_relay
// https://www.boost.org/doc/libs/master/libs/beast/example/doc/http_examples.hpp An HTTP proxy acts
// as a relay between client and server.

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core/multi_buffer.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/version.hpp>
#include <cstdlib>
#include <iostream>

using tcp = boost::asio::ip::tcp;    // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;    // from <boost/asio/ssl.hpp>
namespace http = boost::beast::http; // from <boost/beast/http.hpp>
namespace beast = boost::beast;      // from <boost/beast.hpp>

template <class String> void err(beast::error_code const& ec, String const& what) {
  std::cerr << what << ": " << ec.message() << std::endl;
}

static void load_root_certificates(ssl::context& ctx);

namespace detail {
static void load_root_certificates(ssl::context& ctx, boost::system::error_code& ec);
}

int main() {
  try {
    boost::system::error_code ec;

    // see https://developer.wordpress.com/docs/oauth2/
    // auto const target_host = "https://public-api.wordpress.com:443";
    // auto const host = "www.public-api.wordpress.com";

    // test echo, see https://docs.postman-echo.com/
    // https://postman-echo.com/get?foo1=bar1&foo2=bar2
    auto const target_host = "https://postman-echo.com:443";
    auto const host = "postman-echo.com";

    bool enable_proxy = true;

    int http_version = 11; // HTTP version: 1.0 or 1.1

    // The io_context is required for all I/O
    boost::asio::io_context ioc;
    tcp::socket sock{ioc};

    // These objects perform our I/O

    ssl::context ctx{ssl::context::sslv23_client};

    // load authority cert valid for website
    load_root_certificates(ctx);

    tcp::resolver resolver{ioc};

    {
      if (enable_proxy) {
        // got HTTPs proxy from http://www.proxyserverlist24.top/
        // checked proxy at http://www.checker.freeproxy.ru/checker/
        // 1.10.186.128:56522 HTTP, HTTPs High (Elite), Medium Thailand
        sock.connect({boost::asio::ip::address_v4::from_string("67.207.83.225"), 80}, ec);
      } else {
        // Look up the domain name
        auto const lookup = resolver.resolve({host, "https"}, ec);

        if (ec) {
          std::cerr << "lookup failed"
                    << ": " << ec.message() << std::endl;
        }

        std::cout << "lookup->endpoint " << lookup->endpoint() << std::endl;

        sock.connect({lookup->endpoint().address(), lookup->endpoint().port()}, ec);
      }

      // Make the connection on the IP address we get from a lookup
      if (ec) {
        std::cerr << "connect failed"
                  << ": " << ec.message() << std::endl;
      }
    }

    ssl::stream<tcp::socket&> stream{sock, ctx};

    // To disable peer verification, provide boost::asio::ssl::verify_none to the
    // boost::asio::ssl::stream::set_verify_mode():
    stream.set_verify_mode(ssl::verify_peer | ssl::verify_fail_if_no_peer_cert);

    if (!enable_proxy) {
      // Perform SSL handshaking
      stream.handshake(ssl::stream_base::client, ec);
      if (ec) {
        std::cerr << "handshake failed"
                  << ": " << ec.message() << std::endl;
      }
    }

    // PROXY handshake, uses http::verb::connect
    if (enable_proxy) {
      /** http::verb::connect converts the request connection to a transparent TCP/IP tunnel.

        This is usually to facilitate SSL-encrypted communication (HTTPS)
        through an unencrypted HTTP proxy.
      */
      http::request<http::string_body> req1{http::verb::connect, target_host, http_version};

      req1.set(http::field::host, target_host);

      http::write(sock, req1);
      // http::write(sock, req1);

      boost::beast::flat_buffer buffer;

      // use own response parser
      // NOTE: 200 response to a CONNECT request from a tunneling proxy do not carry a body
      http::response<http::empty_body> res;
      http::parser</* isRequest */ false, http::empty_body> http_parser(res);

      /** Set the skip parse option.
        This option controls whether or not the parser expects to see an HTTP
        body, regardless of the presence or absence of certain fields such as
        Content-Length or a chunked Transfer-Encoding. Depending on the request,
        some responses do not carry a body. For example, a 200 response to a
        CONNECT request from a tunneling proxy, or a response to a HEAD request.
        In these cases, callers may use this function inform the parser that
        no body is expected. The parser will consider the message complete
        after the header has been received.
        @param v `true` to set the skip body option or `false` to disable it.
        @note This function must called before any bytes are processed.
    */
      http_parser.skip(true); // see https://stackoverflow.com/a/49837467/10904212

      /** Read part of a message from a stream using a parser.

    This function is used to read part of a message from a stream into a
    subclass of @ref basic_parser.
    The call will block until one of the following conditions is true
*/
      // because of http_parser.skip above, parser will consider the message complete after the
      // header has been received.
      http::read(sock, buffer, http_parser);
      // http::read(sock, buffer, http_parser);

      // Write the message to standard out
      std::cout << "target_host response: " << res << std::endl;
    }

    // Perform the SSL handshake
    stream.handshake(ssl::stream_base::client);

    // Set up an HTTP GET request message
    {

      // see https://developer.wordpress.com/docs/api/1.1/get/me/
      // http::request<http::string_body> req{http::verb::get, "rest/v1.1/me", http_version};

      // see https://docs.postman-echo.com/
      http::request<http::string_body> req{http::verb::get, "/get?foo1=bar1&foo2=bar2",
                                           http_version};

      // see for understanding http::field
      // https://github.com/boostorg/beast/blob/d4c5dc2747e62c6c1a2c3e319f88940309f0c644/test/beast/http/field.cpp#L39

      // The Host is the domain the request is being sent to. This header was introduced so
      // hosting sites could include multiple domains on a single IP.

      req.set(http::field::host, host);

      // req.set(http::field::proxy_authenticate, fff); // TODO, see
      // https://github.com/equalitie/ceno2/blob/b7dc8cd221f98019fb596fa9f61ee4d78f2303ff/src/authenticate.h#L64

      // req.set(http::field::proxy_connection, fff); // TODO, see
      // https://github.com/equalitie/ceno2/blob/b7dc8cd221f98019fb596fa9f61ee4d78f2303ff/src/cache_control.cpp#L400

      // req.set(http::field::host, host + std::string(":") +
      // std::to_string(lookup->endpoint().port()));

      /*req.set(http::field::user_agent, "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36
      (KHTML, " "like Gecko) Chrome/70.0.3538.77 Safari/537.36");
      // req.set(http::field::authorization, "YOUR_KEY");
      req.set(http::field::upgrade, "1");
      req.set(http::field::pragma, "no-cache");
      req.set(http::field::cache_control, "no-cache");
      req.set(http::field::cookie, "");
      req.set(http::field::accept_language, "en-US,en;q=0.9,ru;q=0.8");
      req.set(http::field::accept_encoding, "gzip, deflate, br");

        // Set the Connection: close field, this way the server will close
        // the connection. This consumes less resources (no TIME_WAIT) because
        // of the graceful close. It also makes things go a little faster.
        //
        req.set(http::field::connection, "close");
*/
      // req.set(
      //    http::field::accept,
      //    "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8");

      // Send the HTTP request to the remote host
      http::write(stream, req);

      boost::beast::flat_buffer buffer;
      http::response<http::dynamic_body> res;
      http::read(stream, buffer, res);

      // Write the message to standard out
      std::cout << "request response: " << res << std::endl;
    }

    // second GET request

    // Set up an HTTP GET request message
    {

      // see https://developer.wordpress.com/docs/api/1.1/get/me/
      // http::request<http::string_body> req{http::verb::get, "rest/v1.1/me", http_version};

      // see https://docs.postman-echo.com/
      http::request<http::string_body> req{http::verb::get, "/get?ds=asd2&sdfdfs=sss2",
                                           http_version};

      // see for understanding http::field
      // https://github.com/boostorg/beast/blob/d4c5dc2747e62c6c1a2c3e319f88940309f0c644/test/beast/http/field.cpp#L39

      // The Host is the domain the request is being sent to. This header was introduced so
      // hosting sites could include multiple domains on a single IP.

      req.set(http::field::host, host);

      // req.set(http::field::proxy_authenticate, fff); // TODO, see
      // https://github.com/equalitie/ceno2/blob/b7dc8cd221f98019fb596fa9f61ee4d78f2303ff/src/authenticate.h#L64

      // req.set(http::field::proxy_connection, fff); // TODO, see
      // https://github.com/equalitie/ceno2/blob/b7dc8cd221f98019fb596fa9f61ee4d78f2303ff/src/cache_control.cpp#L400

      // req.set(http::field::host, host + std::string(":") +
      // std::to_string(lookup->endpoint().port()));

      /*req.set(http::field::user_agent, "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36
      (KHTML, " "like Gecko) Chrome/70.0.3538.77 Safari/537.36");
      // req.set(http::field::authorization, "YOUR_KEY");
      req.set(http::field::upgrade, "1");
      req.set(http::field::pragma, "no-cache");
      req.set(http::field::cache_control, "no-cache");
      req.set(http::field::cookie, "");
      req.set(http::field::accept_language, "en-US,en;q=0.9,ru;q=0.8");
      req.set(http::field::accept_encoding, "gzip, deflate, br");

        // Set the Connection: close field, this way the server will close
        // the connection. This consumes less resources (no TIME_WAIT) because
        // of the graceful close. It also makes things go a little faster.
        //
        req.set(http::field::connection, "close");
*/
      // req.set(
      //    http::field::accept,
      //    "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8");

      // Send the HTTP request to the remote host
      http::write(stream, req);

      boost::beast::flat_buffer buffer;
      http::response<http::dynamic_body> res;
      http::read(stream, buffer, res);

      // Write the message to standard out
      std::cout << "request response: " << res << std::endl;
    }

    getchar();

    // Gracefully close the stream
    // TODO:
    // https://github.com/Webhero9297/Dapps/blob/a3bb302a64aa52061e19f484f150d10b00ff2656/rippled-develop/src/beast/example/http-client-ssl/http_client_ssl.cpp#L98
    // sock.shutdown(boost::asio::ip::tcp::socket::shutdown_send);
    stream.shutdown(ec);
    if (ec == boost::asio::error::eof) {
      // Rationale:
      // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
      ec.assign(0, ec.category());
    }
    if (ec)
      throw boost::system::system_error{ec};
    // If we get here then the connection is closed gracefully
  } catch (std::exception const& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

namespace detail {

static void load_root_certificates(ssl::context& ctx, boost::system::error_code& ec) {

  // https://letsencrypt.org/certs/isrgrootx1.pem.txt
  const std::string cert1 = R"(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)";

  // https://www.identrust.com/certificates/trustid/root-download-x3.html (converted to pem)
  const std::string cert2 = R"(
-----BEGIN CERTIFICATE-----
MIIDSjCCAjKgAwIBAgIQRK+wgNajJ7qJMDmGLvhAazANBgkqhkiG9w0BAQUFADA/
MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT
DkRTVCBSb290IENBIFgzMB4XDTAwMDkzMDIxMTIxOVoXDTIxMDkzMDE0MDExNVow
PzEkMCIGA1UEChMbRGlnaXRhbCBTaWduYXR1cmUgVHJ1c3QgQ28uMRcwFQYDVQQD
Ew5EU1QgUm9vdCBDQSBYMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB
AN+v6ZdQCINXtMxiZfaQguzH0yxrMMpb7NnDfcdAwRgUi+DoM3ZJKuM/IUmTrE4O
rz5Iy2Xu/NMhD2XSKtkyj4zl93ewEnu1lcCJo6m67XMuegwGMoOifooUMM0RoOEq
OLl5CjH9UL2AZd+3UWODyOKIYepLYYHsUmu5ouJLGiifSKOeDNoJjj4XLh7dIN9b
xiqKqy69cK3FCxolkHRyxXtqqzTWMIn/5WgTe1QLyNau7Fqckh49ZLOMxt+/yUFw
7BZy1SbsOFU5Q9D8/RhcQPGX69Wam40dutolucbY38EVAjqr2m7xPi71XAicPNaD
aeQQmxkqtilX4+U9m5/wAl0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNV
HQ8BAf8EBAMCAQYwHQYDVR0OBBYEFMSnsaR7LHH62+FLkHX/xBVghYkQMA0GCSqG
SIb3DQEBBQUAA4IBAQCjGiybFwBcqR7uKGY3Or+Dxz9LwwmglSBd49lZRNI+DT69
ikugdB/OEIKcdBodfpga3csTS7MgROSR6cz8faXbauX+5v3gTt23ADq1cEmv8uXr
AvHRAosZy5Q6XkjEGB5YGV8eAlrwDPGxrancWYaLbumR9YbK+rlmM6pZW87ipxZz
R8srzJmwN0jP41ZL9c8PDHIyh8bwRLtTcm1D9SZImlJnt1ir/md2cXjbDaJWFBM5
JDGFoqgCWjBH4d1QB7wCCZAA62RjYJsWvIjJEubSfZGL+T0yjWW06XyxV3bqxbYo
Ob8VZRzI9neWagqNdwvYkQsEjgfbKbYK7p2CNTUQ
-----END CERTIFICATE-----
)";

  // see https://letsencrypt.org/certificates/
  // https://letsencrypt.org/certs/lets-encrypt-x3-cross-signed.pem.txt
  const std::string cert3 = "-----BEGIN CERTIFICATE-----\n"
                            "MIIEkjCCA3qgAwIBAgIQCgFBQgAAAVOFc2oLheynCDANBgkqhkiG9w0BAQsFADA/\n"
                            "MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n"
                            "DkRTVCBSb290IENBIFgzMB4XDTE2MDMxNzE2NDA0NloXDTIxMDMxNzE2NDA0Nlow\n"
                            "SjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxIzAhBgNVBAMT\n"
                            "GkxldCdzIEVuY3J5cHQgQXV0aG9yaXR5IFgzMIIBIjANBgkqhkiG9w0BAQEFAAOC\n"
                            "AQ8AMIIBCgKCAQEAnNMM8FrlLke3cl03g7NoYzDq1zUmGSXhvb418XCSL7e4S0EF\n"
                            "q6meNQhY7LEqxGiHC6PjdeTm86dicbp5gWAf15Gan/PQeGdxyGkOlZHP/uaZ6WA8\n"
                            "SMx+yk13EiSdRxta67nsHjcAHJyse6cF6s5K671B5TaYucv9bTyWaN8jKkKQDIZ0\n"
                            "Z8h/pZq4UmEUEz9l6YKHy9v6Dlb2honzhT+Xhq+w3Brvaw2VFn3EK6BlspkENnWA\n"
                            "a6xK8xuQSXgvopZPKiAlKQTGdMDQMc2PMTiVFrqoM7hD8bEfwzB/onkxEz0tNvjj\n"
                            "/PIzark5McWvxI0NHWQWM6r6hCm21AvA2H3DkwIDAQABo4IBfTCCAXkwEgYDVR0T\n"
                            "AQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwfwYIKwYBBQUHAQEEczBxMDIG\n"
                            "CCsGAQUFBzABhiZodHRwOi8vaXNyZy50cnVzdGlkLm9jc3AuaWRlbnRydXN0LmNv\n"
                            "bTA7BggrBgEFBQcwAoYvaHR0cDovL2FwcHMuaWRlbnRydXN0LmNvbS9yb290cy9k\n"
                            "c3Ryb290Y2F4My5wN2MwHwYDVR0jBBgwFoAUxKexpHsscfrb4UuQdf/EFWCFiRAw\n"
                            "VAYDVR0gBE0wSzAIBgZngQwBAgEwPwYLKwYBBAGC3xMBAQEwMDAuBggrBgEFBQcC\n"
                            "ARYiaHR0cDovL2Nwcy5yb290LXgxLmxldHNlbmNyeXB0Lm9yZzA8BgNVHR8ENTAz\n"
                            "MDGgL6AthitodHRwOi8vY3JsLmlkZW50cnVzdC5jb20vRFNUUk9PVENBWDNDUkwu\n"
                            "Y3JsMB0GA1UdDgQWBBSoSmpjBH3duubRObemRWXv86jsoTANBgkqhkiG9w0BAQsF\n"
                            "AAOCAQEA3TPXEfNjWDjdGBX7CVW+dla5cEilaUcne8IkCJLxWh9KEik3JHRRHGJo\n"
                            "uM2VcGfl96S8TihRzZvoroed6ti6WqEBmtzw3Wodatg+VyOeph4EYpr/1wXKtx8/\n"
                            "wApIvJSwtmVi4MFU5aMqrSDE6ea73Mj2tcMyo5jMd6jmeWUHK8so/joWUoHOUgwu\n"
                            "X4Po1QYz+3dszkDqMp4fklxBwXRsW10KXzPMTZ+sOPAveyxindmjkW8lGy+QsRlG\n"
                            "PfZ+G6Z6h7mjem0Y+iWlkYcV4PIWL1iwBi8saCbGS5jN2p8M+X+Q7UNKEkROb3N6\n"
                            "KOqkqm57TH2H3eDJAkSnh6/DNFu0Qg==\n"
                            "-----END CERTIFICATE-----\n";

  // DigiCert && GeoTrust certs
  // see https://www.digicert.com/digicert-root-certificates.htm
  const std::string cert =
      /*  This is the DigiCert root certificate.

            CN = DigiCert High Assurance EV Root CA
            OU = www.digicert.com
            O = DigiCert Inc
            C = US

            Valid to: Sunday, ?November ?9, ?2031 5:00:00 PM

            Thumbprint(sha1):
            5f b7 ee 06 33 e2 59 db ad 0c 4c 9a e6 d3 8f 1a 61 c7 dc 25
        */
      "-----BEGIN CERTIFICATE-----\n"
      "MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBs\n"
      "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
      "d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\n"
      "ZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAwMFoXDTMxMTExMDAwMDAwMFowbDEL\n"
      "MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3\n"
      "LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFuY2Ug\n"
      "RVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm\n"
      "+9S75S0tMqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTW\n"
      "PNt0OKRKzE0lgvdKpVMSOO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEM\n"
      "xChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFB\n"
      "Ik5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQNAQTXKFx01p8VdteZOE3\n"
      "hzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUeh10aUAsg\n"
      "EsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQF\n"
      "MAMBAf8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaA\n"
      "FLE+w2kD+L9HAdSYJhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3Nec\n"
      "nzyIZgYIVyHbIUf4KmeqvxgydkAQV8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6z\n"
      "eM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFpmyPInngiK3BD41VHMWEZ71jF\n"
      "hS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkKmNEVX58Svnw2\n"
      "Yzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe\n"
      "vEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep\n"
      "+OkuE6N36B9K\n"
      "-----END CERTIFICATE-----\n"
      /*  This is the GeoTrust root certificate.

            CN = GeoTrust Global CA
            O = GeoTrust Inc.
            C = US
            Valid to: Friday, ‎May ‎20, ‎2022 9:00:00 PM

            Thumbprint(sha1):
            ‎de 28 f4 a4 ff e5 b9 2f a3 c5 03 d1 a3 49 a7 f9 96 2a 82 12
        */
      "-----BEGIN CERTIFICATE-----\n"
      "MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBs\n"
      "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
      "d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\n"
      "ZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAwMFoXDTMxMTExMDAwMDAwMFowbDEL\n"
      "MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3\n"
      "LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFuY2Ug\n"
      "RVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm\n"
      "+9S75S0tMqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTW\n"
      "PNt0OKRKzE0lgvdKpVMSOO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEM\n"
      "xChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFB\n"
      "Ik5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQNAQTXKFx01p8VdteZOE3\n"
      "hzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUeh10aUAsg\n"
      "EsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQF\n"
      "MAMBAf8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaA\n"
      "FLE+w2kD+L9HAdSYJhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3Nec\n"
      "nzyIZgYIVyHbIUf4KmeqvxgydkAQV8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6z\n"
      "eM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFpmyPInngiK3BD41VHMWEZ71jF\n"
      "hS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkKmNEVX58Svnw2\n"
      "Yzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe\n"
      "vEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep\n"
      "+OkuE6N36B9K\n"
      "-----END CERTIFICATE-----\n";

  const std::string cert4 =
      /*  This is the Amazon root certificate.
       *  See https://www.amazontrust.com/repository/
       */
      "-----BEGIN CERTIFICATE-----\n"
      "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"
      "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
      "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"
      "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"
      "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"
      "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"
      "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"
      "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"
      "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"
      "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"
      "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n"
      "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n"
      "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n"
      "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n"
      "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n"
      "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n"
      "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n"
      "rqXRfboQnoZsG4q5WTP468SQvvG5\n"
      "-----END CERTIFICATE-----\n";

  // https://support.comodo.com/index.php?/Knowledgebase/Article/View/970/108/intermediate-2-sha-2-comodo-rsa-domain-validation-secure-server-ca
  const std::string cert5 = "-----BEGIN CERTIFICATE-----\n"
                            "MIIGCDCCA/CgAwIBAgIQKy5u6tl1NmwUim7bo3yMBzANBgkqhkiG9w0BAQwFADCB\n"
                            "hTELMAkGA1UEBhMCR0IxGzAZBgNVBAgTEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4G\n"
                            "A1UEBxMHU2FsZm9yZDEaMBgGA1UEChMRQ09NT0RPIENBIExpbWl0ZWQxKzApBgNV\n"
                            "BAMTIkNPTU9ETyBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTQwMjEy\n"
                            "MDAwMDAwWhcNMjkwMjExMjM1OTU5WjCBkDELMAkGA1UEBhMCR0IxGzAZBgNVBAgT\n"
                            "EkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UEBxMHU2FsZm9yZDEaMBgGA1UEChMR\n"
                            "Q09NT0RPIENBIExpbWl0ZWQxNjA0BgNVBAMTLUNPTU9ETyBSU0EgRG9tYWluIFZh\n"
                            "bGlkYXRpb24gU2VjdXJlIFNlcnZlciBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEP\n"
                            "ADCCAQoCggEBAI7CAhnhoFmk6zg1jSz9AdDTScBkxwtiBUUWOqigwAwCfx3M28Sh\n"
                            "bXcDow+G+eMGnD4LgYqbSRutA776S9uMIO3Vzl5ljj4Nr0zCsLdFXlIvNN5IJGS0\n"
                            "Qa4Al/e+Z96e0HqnU4A7fK31llVvl0cKfIWLIpeNs4TgllfQcBhglo/uLQeTnaG6\n"
                            "ytHNe+nEKpooIZFNb5JPJaXyejXdJtxGpdCsWTWM/06RQ1A/WZMebFEh7lgUq/51\n"
                            "UHg+TLAchhP6a5i84DuUHoVS3AOTJBhuyydRReZw3iVDpA3hSqXttn7IzW3uLh0n\n"
                            "c13cRTCAquOyQQuvvUSH2rnlG51/ruWFgqUCAwEAAaOCAWUwggFhMB8GA1UdIwQY\n"
                            "MBaAFLuvfgI9+qbxPISOre44mOzZMjLUMB0GA1UdDgQWBBSQr2o6lFoL2JDqElZz\n"
                            "30O0Oija5zAOBgNVHQ8BAf8EBAMCAYYwEgYDVR0TAQH/BAgwBgEB/wIBADAdBgNV\n"
                            "HSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwGwYDVR0gBBQwEjAGBgRVHSAAMAgG\n"
                            "BmeBDAECATBMBgNVHR8ERTBDMEGgP6A9hjtodHRwOi8vY3JsLmNvbW9kb2NhLmNv\n"
                            "bS9DT01PRE9SU0FDZXJ0aWZpY2F0aW9uQXV0aG9yaXR5LmNybDBxBggrBgEFBQcB\n"
                            "AQRlMGMwOwYIKwYBBQUHMAKGL2h0dHA6Ly9jcnQuY29tb2RvY2EuY29tL0NPTU9E\n"
                            "T1JTQUFkZFRydXN0Q0EuY3J0MCQGCCsGAQUFBzABhhhodHRwOi8vb2NzcC5jb21v\n"
                            "ZG9jYS5jb20wDQYJKoZIhvcNAQEMBQADggIBAE4rdk+SHGI2ibp3wScF9BzWRJ2p\n"
                            "mj6q1WZmAT7qSeaiNbz69t2Vjpk1mA42GHWx3d1Qcnyu3HeIzg/3kCDKo2cuH1Z/\n"
                            "e+FE6kKVxF0NAVBGFfKBiVlsit2M8RKhjTpCipj4SzR7JzsItG8kO3KdY3RYPBps\n"
                            "P0/HEZrIqPW1N+8QRcZs2eBelSaz662jue5/DJpmNXMyYE7l3YphLG5SEXdoltMY\n"
                            "dVEVABt0iN3hxzgEQyjpFv3ZBdRdRydg1vs4O2xyopT4Qhrf7W8GjEXCBgCq5Ojc\n"
                            "2bXhc3js9iPc0d1sjhqPpepUfJa3w/5Vjo1JXvxku88+vZbrac2/4EjxYoIQ5QxG\n"
                            "V/Iz2tDIY+3GH5QFlkoakdH368+PUq4NCNk+qKBR6cGHdNXJ93SrLlP7u3r7l+L4\n"
                            "HyaPs9Kg4DdbKDsx5Q5XLVq4rXmsXiBmGqW5prU5wfWYQ//u+aen/e7KJD2AFsQX\n"
                            "j4rBYKEMrltDR5FL1ZoXX/nUh8HCjLfn4g8wGTeGrODcQgPmlKidrv0PJFGUzpII\n"
                            "0fxQ8ANAe4hZ7Q7drNJ3gjTcBpUC2JD5Leo31Rpg0Gcg19hCC0Wvgmje3WYkN5Ap\n"
                            "lBlGGSW4gNfL1IYoakRwJiNiqZ+Gb7+6kHDSVneFeO/qJakXzlByjAA6quPbYzSf\n"
                            "+AZxAeKCINT+b72x\n"
                            "-----END CERTIFICATE-----\n";

  ctx.add_certificate_authority(boost::asio::buffer(cert.data(), cert.size()), ec);
  if (ec) {
    std::cerr << "can`t add_certificate_authority" << std::endl;
    return;
  }

  ctx.add_certificate_authority(boost::asio::buffer(cert1.data(), cert1.size()), ec);
  if (ec) {
    std::cerr << "can`t add_certificate_authority" << std::endl;
    return;
  }

  ctx.add_certificate_authority(boost::asio::buffer(cert2.data(), cert2.size()), ec);
  if (ec) {
    std::cerr << "can`t add_certificate_authority" << std::endl;
    return;
  }

  ctx.add_certificate_authority(boost::asio::buffer(cert3.data(), cert3.size()), ec);
  if (ec) {
    std::cerr << "can`t add_certificate_authority" << std::endl;
    return;
  }

  ctx.add_certificate_authority(boost::asio::buffer(cert4.data(), cert4.size()), ec);
  if (ec) {
    std::cerr << "can`t add_certificate_authority" << std::endl;
    return;
  }

  ctx.add_certificate_authority(boost::asio::buffer(cert5.data(), cert5.size()), ec);
  if (ec) {
    std::cerr << "can`t add_certificate_authority" << std::endl;
    return;
  }
}

} // namespace detail

static void load_root_certificates(ssl::context& ctx) {
  boost::system::error_code ec;
  detail::load_root_certificates(ctx, ec);
  // if (ec)
  //   throw boost::system::system_error{ec};
}