#include "assert.h"
#include "bongo.hpp"
#include "urilite.h"

int main(){

    using namespace bongo;
    thread_context ctx;
    std::printf("main thread: %s\n", get_thread_id().c_str());

    //just_sender<int> first{42};
    auto sh= schedule(ctx.get_scheduler());
     auto request = [](auto url){
        urilite::uri remotepath =urilite::uri::parse(url);
        bongo::CurlSession session;
        auto resp=session.get(bongo::Url(urilite::update_query(remotepath)),
                                bongo::HttpHeader{{"Accept", "*/*"},
                                                {"Accept-Language", "en-US,en;q=0.5"},
                                                {"Host" ,"www1.nseindia.com"},
                                                {"User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:28.0) Gecko/20100101 Firefox/28.0"},
                                                {"X-Requested-With", "XMLHttpRequest'"}},
                                                bongo::ContentType{"application/json"});
        
        return resp;
        
    };
    auto first = then(sh, [](auto ){ return std::string("https://www.google.com/"); });
    auto next = then(first, request);
    auto recur= [=](auto t) {
        spawn(sh,reciever_of([=](auto i){
              auto first = then(sh, [](auto ){ return std::string("https://www.google.com/"); });
              auto next = then(first, request);
              auto last = connect(next, reciever_of([=](auto i){
                    std::printf("%s", i.text.data());
                }));
              start(last);
        }));
        return t;
    } ;
    auto last = then(next,recur);

    auto i = sync_wait(last).value();
    // std::printf("%s", i.text.data());
    ctx.finish();
    ctx.join();

}