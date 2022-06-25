#include "assert.h"
#include "bongo.hpp"
#include "urilite.h"
#include <unifex/scheduler_concepts.hpp>
#include <unifex/sync_wait.hpp>
#include <unifex/single_thread_context.hpp>
#include <unifex/then.hpp>
#include <unifex/when_all.hpp>
#include <unifex/sync_wait.hpp>
#include <unifex/just.hpp>
#include <unifex/let_error.hpp>

#include <iostream>

using namespace bongo;
using namespace unifex;

int main(){

    
    single_thread_context context;
    auto sh = context.get_scheduler();
    
    auto graph1= schedule(sh) 
                  |get(std::string("https://www.google.com/"),bongo::HttpHeader{{"Accept", "*/*"},
                                                {"Accept-Language", "en-US,en;q=0.5"}},
                                                bongo::ContentType{"application/json"})
                  |then([](auto v){return v.text;});
    auto graph2= schedule(sh)
                  |get(std::string("https://www.yahoo.com/"))
                  |then([](auto v){return v.text;});
    auto w = when_all(graph1,graph2);
    auto i =w|then([](auto&& a,auto&& b){ return std::get<0>(std::get<0>(a)) + std::get<0>(std::get<0>(b));})
                |upon_error([](auto v){return v;});
    auto t= sync_wait(std::move(i)).value();
    std::printf("%s", t.data());
   
    
    // ctx.finish();
    // ctx.join();



}