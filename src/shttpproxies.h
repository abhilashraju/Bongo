#pragma once

#include <initializer_list>
#include <map>
#include <string>
#include <utility>

namespace bongo {

class Proxies {
  public:
    Proxies() {}
    Proxies(const std::initializer_list<std::pair<const std::string, std::string>>& hosts);

    bool has(const std::string& protocol) const;
    const std::string& operator[](const std::string& protocol)const;

  private:
    std::map<std::string, std::string> hosts_;
};

} // namespace cpr

