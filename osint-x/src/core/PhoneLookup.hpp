#pragma once
#include "OSINTModule.hpp"
#include "../utils/HTTPClient.hpp"
#include "../utils/SimpleJSON.hpp"
#include "../utils/KeyRotator.hpp"
#include "../include/config.hpp"
#include <regex>

class PhoneLookup : public OSINTModule {
private:
    HTTPClient http;
    KeyRotator keyPool;

    std::string cleanNumber(const std::string& raw) const {
        std::string clean;
        for (char c : raw)
            if (std::isdigit(c) || c == '+') clean += c;
        return clean;
    }

public:
    PhoneLookup() : OSINTModule("PhoneLookup", ""),
                keyPool(APIConfig::numverifyKeys()) {}

    bool validate(const std::string& query) const override {
        std::string clean;
        for (char c : query)
            if (std::isdigit(c) || c == '+') clean += c;
        return clean.size() >= 7 && clean.size() <= 15;
    }

    std::string helpText() const override {
        return "Usage: phone <number>\nExample: phone +919876543210\nInclude country code for best results";
    }

    ReconResult run(const std::string& query) override {
        ReconResult result;
        result.module = moduleName;
        result.query  = query;

        if (!validate(query)) {
            result.success = false;
            result.error   = "Invalid phone number format";
            return result;
        }

        if (keyPool.empty()) {
            result.success = false;
            result.error   = "No NumVerify keys — add them in include/config.hpp";
            return result;
        }

        std::string clean = cleanNumber(query);
        log("Querying NumVerify for " + clean);

        // Use key from pool
        std::string key  = keyPool.next();
        std::string url  = "http://apilayer.net/api/validate?access_key=" +
                            key + "&number=" + http.urlEncode(clean) + "&format=1";
        std::string resp = http.get(url);

        // Auto rotate if rate limited or error
        if (resp.empty() || resp.find("\"success\":false") != std::string::npos) {
            log("Key rate limited — rotating to next key");
            key  = keyPool.next();
            url  = "http://apilayer.net/api/validate?access_key=" +
                    key + "&number=" + http.urlEncode(clean) + "&format=1";
            resp = http.get(url);
        }

        if (resp.empty()) {
            result.success = false;
            result.error   = "Network error — could not reach NumVerify";
            return result;
        }

        // Check for API error
        if (resp.find("\"success\":false") != std::string::npos) {
            result.success = false;
            result.error   = "API error: " + SimpleJSON::getString(resp, "info");
            return result;
        }

        bool valid = SimpleJSON::getBool(resp, "valid");
        if (!valid) {
            result.success = false;
            result.error   = "Number is invalid according to NumVerify";
            return result;
        }

        // Parse all fields
        std::string number        = SimpleJSON::getString(resp, "number");
        std::string localFormat   = SimpleJSON::getString(resp, "local_format");
        std::string intlFormat    = SimpleJSON::getString(resp, "international_format");
        std::string countryPrefix = SimpleJSON::getString(resp, "country_prefix");
        std::string countryCode   = SimpleJSON::getString(resp, "country_code");
        std::string countryName   = SimpleJSON::getString(resp, "country_name");
        std::string location      = SimpleJSON::getString(resp, "location");
        std::string carrier       = SimpleJSON::getString(resp, "carrier");
        std::string lineType      = SimpleJSON::getString(resp, "line_type");

        result.addField("Number",               number);
        result.addField("Local Format",         localFormat);
        result.addField("International Format", intlFormat);
        result.addField("Country",              countryName);
        result.addField("Country Code",         countryCode);
        result.addField("Dial Code",            countryPrefix);
        result.addField("Location/Region",      location.empty() ? "N/A" : location);
        result.addField("Carrier",              carrier.empty() ? "N/A" : carrier);
        result.addField("Line Type",            lineType.empty() ? "N/A" : lineType);
        result.addField("Valid",                "Yes");
        result.addField("Source",               "NumVerify API (apilayer.net)");

        result.success = true;
        return result;
    }
};