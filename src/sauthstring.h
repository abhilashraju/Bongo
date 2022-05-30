#pragma once
#include <string>
namespace Ui {

class Authentication {
  public:
    Authentication()=default;
    Authentication(const std::string& username, const std::string& password)
            : auth_string_{username + ":" + password} {}
    Authentication(std::string&& username, std::string&& password)
            : auth_string_{std::move(username) + ":" + std::move(password)} {}
    ~Authentication() = default;

    const char* GetAuthString() const noexcept{return auth_string_.c_str();}

  protected:
    std::string auth_string_;
};
class Digest : public Authentication {
  public:
    Digest(const std::string& username, const std::string& password)
            : Authentication{username, password} {}
};
class NTLM : public Authentication {
  public:
    NTLM(const std::string& username, const std::string& password)
            : Authentication{username, password} {}
};
} // namespace cpr
