#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/mod/mod_base.hpp"

mod_base::mod_base() {
    if (version.empty()) {
        version = "1.0.0";
    }
}

mod_base::~mod_base() {
}
