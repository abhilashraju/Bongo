#pragma once
#include <utility>
#include "snetworktypes.h"

namespace Ui{
    using Pair = std::pair<std::string, std::string>;

    template<typename Validator>
    class Payloadimpl {
    public:
        Payloadimpl()=default;
        template <class It>
        Payloadimpl(const It begin, const It end) {
            // Create a temporary CurlHolder for URL encoding:
            Validator holder;
            for (It pair = begin; pair != end; ++pair) {
                AddPair(*pair, holder);
            }
        }
        Payloadimpl(const std::initializer_list<Pair>& pairs)
        {
             Validator holder;
             for (auto& p:pairs) {
                AddPair(p, holder);
            }

        }

        void AddPair(const Pair& p, const Validator& holder)
        {
            if (!content.empty()) {
                content += "&";
            }
            std::string escaped = holder(p.second);
            content += p.first + "=" + escaped;
        }
        template<typename CutomValidator>
        std::string GetContent(const CutomValidator& holder) const{
            return content;
        }
        std::string content;
    };
    using Payload=Payloadimpl<IdentityValidator>;
}
