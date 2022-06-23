# http client for modern c++ 

Experimental HTTP client for modern c++ users. 
  It wraps libcurl apis using modern c++ style and sender reciever patterns.
  
  Example
  ```
    single_thread_context context;
    auto sh = context.get_scheduler();
    
    auto graph1= schedule(sh) 
                  |get(std::string("https://www.google.com/"),bongo::HttpHeader{{"Accept", "*/*"},
                                                {"Accept-Language", "en-US,en;q=0.5"}},
                                                bongo::ContentType{"application/json"})
                  |then([](auto v){return v.text;});
    auto graph2= schedule(sh)
                  |get(std::string("https://www.yaheoo.com/"))
                  |then([](auto v){return v.text;});
    auto w = when_all(graph1,graph2);
    auto i =w|then([](auto&& a,auto&& b){ return std::get<0>(std::get<0>(a)) + std::get<0>(std::get<0>(b));});
    try{
        auto t= sync_wait(std::move(i)).value();
        std::printf("%s", t.data());
    }catch(HttpException ex){
         std::printf("%s", ex.what());
    }
  ```
 


