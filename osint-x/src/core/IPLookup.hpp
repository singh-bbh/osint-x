#pragma once
#include "OSINTModule.hpp"
#include "../utils/HTTPClient.hpp"
#include "../utils/SimpleJSON.hpp"
#include <regex>

// Inherits from OSINTModule — Inheritance + Method Overriding
class IPLookup : public OSINTModule {
private:
    HTTPClient http;

    bool isValidIP(const std::string& ip) const {
        std::regex ipv4(R"(^(\d{1,3}\.){3}\d{1,3}$)");
        std::regex ipv6(R"(^([0-9a-fA-F]{0,4}:){1,7}[0-9a-fA-F]{0,4}$)");
        return std::regex_match(ip, ipv4) || std::regex_match(ip, ipv6);
    }

public:
    IPLookup() : OSINTModule("IPLookup") {}

    // Method Override
    bool validate(const std::string& query) const override {
        return isValidIP(query);
    }

    std::string helpText() const override {
        return "Usage: ip <address>\nExample: ip 8.8.8.8";
    }

    ReconResult run(const std::string& query) override {
        ReconResult result;
        result.module = moduleName;
        result.query  = query;

        if (!validate(query)) {
            result.success = false;
            result.error   = "Invalid IP address format";
            return result;
        }

        log("Querying ip-api.com for " + query);

        // ip-api.com — free, no key, 1000 req/min
        std::string url = "http://ip-api.com/json/" + query +
                          "?fields=status,message,country,countryCode,region,"
                          "regionName,city,zip,lat,lon,timezone,isp,org,as,query";

        std::string resp = http.get(url);
        if (resp.empty()) {
            result.success = false;
            result.error   = "Network error — could not reach ip-api.com";
            return result;
        }

        std::string status = SimpleJSON::getString(resp, "status");
        if (status != "success") {
            result.success = false;
            result.error   = SimpleJSON::getString(resp, "message");
            return result;
        }

        result.success   = true;
        result.latitude  = SimpleJSON::getDouble(resp, "lat");
        result.longitude = SimpleJSON::getDouble(resp, "lon");
        result.hasCoords = true;

        result.addField("IP",          SimpleJSON::getString(resp, "query"));
        result.addField("City",        SimpleJSON::getString(resp, "city"));
        result.addField("Region",      SimpleJSON::getString(resp, "regionName"));
        result.addField("Country",     SimpleJSON::getString(resp, "country"));
        result.addField("Country Code",SimpleJSON::getString(resp, "countryCode"));
        result.addField("ZIP",         SimpleJSON::getString(resp, "zip"));
        result.addField("Timezone",    SimpleJSON::getString(resp, "timezone"));
        result.addField("ISP",         SimpleJSON::getString(resp, "isp"));
        result.addField("Org",         SimpleJSON::getString(resp, "org"));
        result.addField("ASN",         SimpleJSON::getString(resp, "as"));
        result.addField("Latitude",    std::to_string(result.latitude));
        result.addField("Longitude",   std::to_string(result.longitude));

        return result;
    }

    // Overriding printResult — Polymorphism
    void printResult(const ReconResult& r) const override {
        OSINTModule::printResult(r);  // call base
        if (r.hasCoords)
            std::cout << "  Map: https://www.openstreetmap.org/?mlat="
                      << r.latitude << "&mlon=" << r.longitude << "\n";
    }
};