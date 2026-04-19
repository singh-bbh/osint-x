#pragma once
#include <string>
#include <vector>

// ── API Key pools ──
// Tool cycles through keys automatically if one hits rate limit
// Add your own keys to expand the pool

struct APIConfig {

    // NumVerify — numverify.com (100 req/month each)
    static std::vector<std::string> numverifyKeys() {
        return {
            "d8142dfbaebbdfb1310283e4abcd332e",
            "df9045267a9c0127425b5e2747167ae2",
            "057d1ed3090f93bd84cf0d0130a3c38f"
        };
    }

    // LeakCheck — leakcheck.io
    static std::vector<std::string> leakcheckKeys() {
        return {
            "205ff6600aa99ee523da362582a0a01ede568f9c",
            "00d4ae43ca041d28e21786c132fe2a83bc3617ca",
            "be673fec2fc12ce0530251f863b295b7e79f20d9"
        };
    }

    // WhoisFreaks — whoisfreaks.com (500 req/month each)
    static std::vector<std::string> whoisfreaksKeys() {
        return {
            "95720f3168c3427f8a171dfdda8e33bc",
            "34e60334008444b49f173e281a478c38",
            "6df0c0339bdf4c53ad9913977a3e42c2"
        };
    }
};