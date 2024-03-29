#include "shttpproxies.h"

#include <initializer_list>
#include <map>
#include <string>
#include <utility>

namespace bongo {

Proxies::Proxies(
    const std::initializer_list<std::pair<const std::string, std::string>>&
        hosts)
  : hosts_{hosts} {
}

inline bool Proxies::has(const std::string& protocol) const {
  return hosts_.count(protocol) > 0;
}

inline const std::string&
Proxies::operator[](const std::string& protocol) const {
  return hosts_.at(protocol);
}

}  // namespace bongo
