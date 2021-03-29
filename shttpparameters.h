#pragma once
#include <string>
#include "snetworktypes.h"
namespace Ui{
    struct Parameter {
        Parameter(const std::string& key, const std::string& value,const std::string& t=std::string(),bool ih=false)
                : key{key}, value{value} ,type{t},inherit{ih}{}
        Parameter(std::string&& key, std::string&& value,std::string&& t=std::string(),bool ih=false)
                : key{std::move(key)}, value{std::move(value)},type{std::move(t)},inherit{ih} {}

        std::string key;
        std::string value;
        std::string type;
        bool inherit{false};
    };

    template<typename Validator>
    class Parametersimpl {
    public:
        Parametersimpl() = default;
        Parametersimpl(const std::initializer_list<Parameter>& parameters)
        {
            Validator holder;
            for (const auto& parameter : parameters) {
                AddParameter(parameter, holder);
            }
        }
        Parametersimpl(const std::vector<Parameter>& parameters)
        {
            Validator holder;
            for (const auto& parameter : parameters) {
                AddParameter(parameter, holder);
            }
        }

        void AddParameter(const Parameter& parameter, const Validator& holder=Validator())
        {
            if (!content.empty()) {
                content += "&";
            }

            std::string escapedKey = holder(parameter.key);
            if (parameter.value.empty()) {
                content += escapedKey;
            } else {
                std::string escapedValue = holder(parameter.value);
                content += escapedKey + "=" + escapedValue;
            }
        }
        template<typename CutomValidator>
        std::string GetContent(const CutomValidator& holder) const{
            return content;
        }
        std::string content;
    };
    using Parameters=Parametersimpl<IdentityValidator>;
}

