#include "http_client.hpp"
#include <regex>
namespace {
    template <typename Func>
    std::string replace(std::string s,std::regex  reg,Func func){
        std::smatch match;
        bool stop=false;
        while (!stop && std::regex_search(s, match, reg)) {
            std::string param=match.str(); 
            std::string formatter="$`";
            formatter += func(boost::string_view{param.c_str()+1,param.length()-2},stop);
            formatter += "$'";
            s = match.format(
                formatter); 
        }
        return s;
    }
    std::string getLanguage(){
        constexpr std::array <const char*,38> lang{
            "en",
        };
        return lang[0];
    }
    std::string getCountry(){
        constexpr std::array <const char*,55> country{
            "BR",
        };
        return country[0];
    }
    std::string getModel(){
        return "HP%20OfficeJet%20Pro%208020%20series";
    }
    std::string getStatus(){
        return "success";
    }
    std::string getError(){
        return "failed";
    }
    std::string getAuthToken(std::string name)
        {
            return std::string{"Basic QVFBQUFBRnpWaHBmaVFBQUFBRTRXOW5ROmtZRXpWYVBHY0VvalpLeXZOT1ZuK0tEYzE0UUVwc09PWE1VZGd4SlYvL1U9"};
            
        }
    std::string substituteParams(std::string urlp){
        
        std::regex pattern(R"((\{([^\}]|\{\})*\}))");
        auto repalced = replace(urlp,pattern,[=](auto truncparam,bool& stop){
                if(truncparam=="$lang"){
                    return getLanguage();
                }
                if(truncparam=="$region"){
                    return getCountry();
                }
                if(truncparam=="$model"){
                    return getModel();
                }
                if(truncparam=="$status"){
                    return getStatus();
                }
                 if(truncparam=="$error"){
                    stop=true;
                    return getError();
                }
                // if(truncparam[0]=='#'){
                //     return get_param(truncparam.substr(1));
                // }

                // return get_param(truncparam);
               
                return std::string{truncparam.data(),truncparam.length()};
            });
        // SDEBUG()<<"URL: "<< repalced;
        return repalced;
    };
    std::string create_body(){
        return "session_id=03bf261c-ed73-4b76-b9c7-dc32dfd152e3&session_name=test.younus@gmail.com&shortcut_id=601d359fe16cd70f8f7e8daf";
    }
}
int main()
{
    using namespace std::literals::chrono_literals;
//    std::string url("http://sipserver.psr.rd.hpicorp.net/main/xmltestCases_v3.1/conformance_v3.1.xml");
//    std::string url("https://pie-crs-pod1-appgate-gen2-podlb-386153685.us-east-1.elb.amazonaws.com:443/shortcut/v1/cui?current=shortcut&lang={$lang}&amp;region={$region}&amp;model={$model}");
   std::string url("https://pie-crs-pod1-appgate-gen2-podlb-386153685.us-east-1.elb.amazonaws.com:443/shortcut/v1/cui?current=shortcuts_list&lang=en&region=CR&model=HP%20OfficeJet%20Pro%208020%20series");
   http::verb verb_ = http::verb::post;
   do {
        urilite::uri remotepath =urilite::uri::parse(substituteParams(url));
        Ui::Get()(remotepath,verb_,Ui::Body(create_body()),
                                Ui::HttpHeader{{std::string{"Authorization"},getAuthToken("svc_sbs")}},
                                Ui::ContentType{"text/plain"},[=](beast::error_code ec,std::string data){
                                std::cout << "call_app_handler: ec=" << ec.message() << ", msg=" << data<< std::endl;
                        
                                });
            std::cout << "Enter url\n";
            std::cin >> url;
            int v=3;
            std::cout << "Enter method\n";
            std::cin >> v;
            verb_= http::verb(v);
   }
   while(url != "end");//wait indefenitely
  
    return 0;
}