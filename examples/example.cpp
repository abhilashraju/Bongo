#include "config.h"
#include "assert.h"
#include "bongo.hpp"
#include "urilite.h"
#include <unifex/scheduler_concepts.hpp>
#include <unifex/sync_wait.hpp>
#include <unifex/static_thread_pool.hpp>
#include <unifex/then.hpp>
#include <unifex/when_all.hpp>
#include <unifex/sync_wait.hpp>
#include <unifex/just.hpp>
#include <unifex/let_error.hpp>

#include <iostream>
#include <nlohmann/json.hpp>
using namespace bongo;
using namespace unifex;
using namespace nlohmann;

int main() {
  static_thread_pool context;
  auto sh = context.get_scheduler();

  auto graph1 =
      schedule(sh) |
      bongo::process(
          verb::get,
          std::string("https://google.com"),
          bongo::HttpHeader{
              {"Accept", "*/*"}, {"Accept-Language", "en-US,en;q=0.5"}},
          bongo::ContentType{"application/json"}) |
      then([](auto v) { return json::parse(v.text).dump(4); });
//   auto graph2 = schedule(sh) |
//       bongo::process(
//                     verb::get,
//                     std::string("https://gorest.co.in/public/v2/posts")) |
//       then([](auto v) { return json::parse(v.text); });
//   auto w = when_all(graph1, graph2);
//   auto i = w | then_all([](auto&& a, auto&& b) { return a.dump(4); }) |
//       upon_error([](auto v) { return v; });
  auto t = sync_wait(graph1).value();
  std::printf("%s", t.data());
}