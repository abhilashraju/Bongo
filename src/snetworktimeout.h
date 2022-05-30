#pragma once
#include <chrono>
#include <cstdint>
namespace bongo{
class Timeout {
  public:
    Timeout(const std::chrono::milliseconds& duration) : ms{duration} {}
    Timeout(const std::int32_t& milliseconds) : Timeout{std::chrono::milliseconds(milliseconds)} {}

    long Milliseconds() const{return ms.count();}

    std::chrono::milliseconds ms;
};

class ConnectTimeout : public Timeout {
  public:
    ConnectTimeout(const std::chrono::milliseconds& duration) : Timeout{duration} {}
    ConnectTimeout(const std::int32_t& milliseconds) : Timeout{milliseconds} {}
};
}
