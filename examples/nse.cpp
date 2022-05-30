#include "http_client.hpp"
int main()
{
    using namespace std::literals::chrono_literals;
    std::string url("http://121.241.2.63/bondsnew/rest/login");
   http::verb verb_ = http::verb::get;
   do {
        urilite::uri remotepath =urilite::uri::parse(url);
        Ui::Get()(remotepath,
                                Ui::HttpHeader{{"Accept", "*/*"},
                                                {"Accept-Language", "en-US,en;q=0.5"},
                                                {"Host" ,"www1.nseindia.com"},
                                                {"User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:28.0) Gecko/20100101 Firefox/28.0"},
                                                {"X-Requested-With", "XMLHttpRequest'"}},
                                Ui::ContentType{"application/json"},[=](beast::error_code ec,std::string data){
                                std::cout << "call_app_handler: ec=" << ec.message() << ", msg=" << data<< std::endl;
                        
                                });
            std::cout << "Enter url\n";
            std::cin >> url;
        
   }
   while(url != "end");//wait indefenitely
  
    return 0;
}