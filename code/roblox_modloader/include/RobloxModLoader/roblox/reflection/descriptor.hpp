#pragma once

#include "RobloxModLoader/roblox/memory/noncopyable.hpp"

class Descriptor : public rml::memory::Noncopyable {
public:
    struct Attributes {
        bool is_deprecated;
        const Descriptor *preferred; // used if isDeprecated
        Attributes()
            : is_deprecated(false)
              , preferred(nullptr) {
        }

        static Attributes deprecated(const Descriptor &preferred) {
            Attributes result;
            result.is_deprecated = true;
            result.preferred = &preferred;
            return result;
        }

        static Attributes deprecated() {
            Attributes result;
            result.is_deprecated = true;
            return result;
        }
    };

    static bool locked_down;
    std::string_view name;
    std::unique_ptr<bool> is_replicable;
    std::unique_ptr<bool> is_outdated;
    const Attributes attributes;

    virtual ~Descriptor() {
    }
};
