#pragma once
#include "OSINTModule.hpp"
#include "../utils/HTTPClient.hpp"
#include "../utils/SimpleJSON.hpp"
#include "../utils/KeyRotator.hpp"
#include "../../include/config.hpp"
#include <regex>
#include <vector>

class WHOISLookup : public OSINTModule {
private:
    HTTPClient http;
    KeyRotator keyPool;

    bool isValidDomain(const std::string& d) const {
        std::regex domainRe(R"(^([a-zA-Z0-9]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?\.)+[a-zA-Z]{2,}$)");
        return std::regex_match(d, domainRe);
    }

    // Google DNS over HTTPS — confirmed working in Codespaces
    std::string queryDNS(const std::string& domain, const std::string& type) {
        std::string url = "https://dns.google/resolve?name=" + domain + "&type=" + type;
        return http.get(url);
    }

    // Extract all "data" fields from Google DNS response
    std::vector<std::string> extractDNSData(const std::string& resp) {
        std::vector<std::string> vals;
        size_t pos = 0;
        while ((pos = resp.find("\"data\":", pos)) != std::string::npos) {
            pos += 7;
            while (pos < resp.size() && resp[pos] == ' ') pos++;
            if (resp[pos] == '"') {
                pos++;
                std::string v;
                while (pos < resp.size() && resp[pos] != '"') v += resp[pos++];
                vals.push_back(v);
                pos++;
            } else {
                // numeric data
                std::string v;
                while (pos < resp.size() && resp[pos] != ',' && resp[pos] != '}') v += resp[pos++];
                if (!v.empty()) vals.push_back(v);
            }
        }
        return vals;
    }

    // Parse WhoisFreaks response
    void parseWhoisFreaks(const std::string& resp, ReconResult& result) {
        // registrar
        std::string registrar = SimpleJSON::getString(resp, "registrar_name");
        if (registrar.empty()) registrar = SimpleJSON::getString(resp, "registrar");
        result.addField("Registrar", registrar.empty() ? "N/A" : registrar);

        // dates
        std::string created = SimpleJSON::getString(resp, "domain_registered");
        if (created.empty()) created = SimpleJSON::getString(resp, "created_date");
        if (created.empty()) created = SimpleJSON::getString(resp, "creation_date");
        result.addField("Registered", created.empty() ? "N/A" : created);

        std::string expires = SimpleJSON::getString(resp, "domain_updated");
        if (expires.empty()) expires = SimpleJSON::getString(resp, "expiry_date");
        if (expires.empty()) expires = SimpleJSON::getString(resp, "expiration_date");
        result.addField("Expires", expires.empty() ? "N/A" : expires);

        // status
        std::string status = SimpleJSON::getString(resp, "domain_status");
        if (status.empty()) status = SimpleJSON::getString(resp, "status");
        result.addField("Status", status.empty() ? "N/A" : status);

        // country
        std::string country = SimpleJSON::getString(resp, "registrant_country");
        if (country.empty()) country = SimpleJSON::getString(resp, "country");
        result.addField("Registrant Country", country.empty() ? "N/A" : country);

        // nameservers — look for name_servers array
        size_t nsPos = resp.find("\"name_servers\"");
        if (nsPos == std::string::npos) nsPos = resp.find("\"nameservers\"");
        if (nsPos != std::string::npos) {
            size_t arrStart = resp.find('[', nsPos);
            if (arrStart != std::string::npos) {
                arrStart++;
                while (arrStart < resp.size() && resp[arrStart] != ']') {
                    while (arrStart < resp.size() &&
                           (resp[arrStart]==' '||resp[arrStart]=='\n'||resp[arrStart]==',')) arrStart++;
                    if (resp[arrStart] == '"') {
                        arrStart++;
                        std::string ns;
                        while (arrStart < resp.size() && resp[arrStart] != '"') ns += resp[arrStart++];
                        if (!ns.empty()) result.list.push_back("NS: " + ns);
                        arrStart++;
                    } else arrStart++;
                }
            }
        }
    }

public:
    // Pass whoisfreaks API key
    WHOISLookup() : OSINTModule("WHOISLookup", ""),
                    keyPool(APIConfig::whoisfreaksKeys()) {}

    bool validate(const std::string& query) const override {
        return isValidDomain(query);
    }

    std::string helpText() const override {
        return "Usage: whois <domain>\nExample: whois google.com";
    }

    ReconResult run(const std::string& query) override {
        ReconResult result;
        result.module = moduleName;
        result.query  = query;

        if (!validate(query)) {
            result.success = false;
            result.error   = "Invalid domain name format";
            return result;
        }

        result.addField("Domain", query);

        // WHOIS via WhoisFreaks with key rotation
        if (!keyPool.empty()) {
            log("Querying WhoisFreaks for " + query);
            std::string key  = keyPool.next();
            std::string url  = "https://api.whoisfreaks.com/v1.0/whois?whois=live&domainName=" +
                                query + "&apiKey=" + key;
            std::string resp = http.get(url);

            // Rotate if rate limited
            if (resp.empty() || resp.find("\"error\"") != std::string::npos) {
                key  = keyPool.next();
                url  = "https://api.whoisfreaks.com/v1.0/whois?whois=live&domainName=" +
                        query + "&apiKey=" + key;
                resp = http.get(url);
            }

            if (!resp.empty() && resp.find("registrar") != std::string::npos)
                parseWhoisFreaks(resp, result);
            else
                result.addField("WHOIS", "Unavailable — check WhoisFreaks keys");

            result.addField("WHOIS Source", "WhoisFreaks API");
        } else {
            result.addField("WHOIS", "No WhoisFreaks keys in config.hpp");
        }

        // ── DNS Records via Google DNS (confirmed working) ──
        log("Fetching A records");
        std::string aResp = queryDNS(query, "A");
        auto aRecords = extractDNSData(aResp);
        for (auto& r : aRecords)
            result.list.push_back("A:   " + r);

        log("Fetching MX records");
        std::string mxResp = queryDNS(query, "MX");
        auto mxRecords = extractDNSData(mxResp);
        for (auto& r : mxRecords)
            result.list.push_back("MX:  " + r);

        log("Fetching TXT records");
        std::string txtResp = queryDNS(query, "TXT");
        auto txtRecords = extractDNSData(txtResp);
        for (auto& r : txtRecords)
            result.list.push_back("TXT: " + r);

        log("Fetching NS records");
        std::string nsResp = queryDNS(query, "NS");
        auto nsRecords = extractDNSData(nsResp);
        for (auto& r : nsRecords)
            result.list.push_back("NS:  " + r);

        result.addField("DNS Source", "Google DNS over HTTPS (dns.google)");
        result.success = true;
        return result;
    }
};