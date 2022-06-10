#pragma once
#include "scurlclient.hpp"
#include "sender_reciever.hpp"
#include "urilite.h"
namespace bongo{
class CurlSession
{
    
    SCurlSession curl_session_;
    Port port_{"80"};
    std::string vers_{"1.0"};
    Target target_{"/"};
    ContentType cont_type_{"text/plain"};
    public:
    CurlSession(){}
    
    void SetOption(const Url& url)
    {
       curl_session_.SetUrl(url);
    }
    void SetOption(const Parameters& parameters)
    {
       curl_session_.SetParameters(parameters);
    }
    void SetOption(Parameters&& parameters)
    {
        curl_session_.SetParameters(std::move(parameters));
    }
    void SetOption(const HttpHeader& header)
    {
        curl_session_.SetHeader(header);
    }
    void SetOption(const Timeout& timeout){curl_session_.SetTimeout(timeout);}
    void SetOption(const ConnectTimeout& timeout){curl_session_.SetConnectTimeout(timeout);}
    void SetOption(const Authentication& auth)
    {
        curl_session_.SetAuth(auth);
    }
    void SetOption(const Digest& auth){curl_session_.SetDigest(auth);}
    void SetOption(const UserAgent& ua){curl_session_.SetUserAgent(ua);}
    void SetOption(Payload&& payload)
    {
        curl_session_.SetPayload(std::move(payload));
        
    }
    void SetOption(const Payload& payload)
    {
        curl_session_.SetPayload(payload);
    }
    void SetOption(Body&& b)
    {
        curl_session_.SetBody(std::move(b));
    }
    void SetOption(const Body& b)
    {
        curl_session_.SetBody(b);
    }
    void SetOption(const ContentType& t){
        cont_type_=t;
    }
    void SetOption(ContentType&& t){
        cont_type_=std::move(t);
    }

    void SetOption(bongo::Port port)
    {
        port_=std::move(port);
    }
    void SetOption(bongo::Target t)
    {
        target_=std::move(t);
    }
   
    void SetOption(const VerifySsl& verify)
    {
        curl_session_.SetVerifySsl(verify);
    }
    // void SetOption(const LimitRate& limit_rate);
    void SetOption(Proxies&& proxies){ curl_session_.SetProxies(std::move(proxies));}
    void SetOption(const Proxies& proxies){curl_session_.SetProxies(proxies); }
    // void SetOption(Multipart&& multipart);
    // void SetOption(const Multipart& multipart);
    // void SetOption(const NTLM& auth);
    // void SetOption(const bool& redirect);
    // void SetOption(const MaxRedirects& max_redirects);
    // void SetOption(const Cookies& cookies);
    template<typename... ARGS>
    Response get(ARGS&&... args)
    { 
        using expandtype=int[];
        expandtype{(SetOption(args),0)...};
        return curl_session_.Get();
    }
    Response post()
    {
        return curl_session_.Post();
       
    }
    
    
    
    
};
}