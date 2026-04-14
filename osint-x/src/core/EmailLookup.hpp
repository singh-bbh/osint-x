#pragma once
#include "OSINTModule.hpp"
#include "../utils/HTTPClient.hpp"
#include "../utils/SimpleJSON.hpp"
#include <regex>
#include <set>

class EmailLookup : public OSINTModule {
private:
    HTTPClient http;
    std::string rapidApiKey;
    std::string intelxKey;

    std::set<std::string> disposableDomains = {
        "mailinator.com","guerrillamail.com","tempmail.com","throwaway.email",
        "yopmail.com","10minutemail.com","trashmail.com","fakeinbox.com",
        "sharklasers.com","dispostable.com","maildrop.cc","discard.email"
    };

    std::string extractDomain(const std::string& email) const {
        size_t pos = email.find('@');
        if (pos == std::string::npos) return "";
        return email.substr(pos + 1);
    }

    // Fixed — uses Google DNS instead of dns-api.org
    bool checkMXRecord(const std::string& domain) {
        std::string url  = "https://dns.google/resolve?name=" + domain + "&type=MX";
        std::string resp = http.get(url);
        return !resp.empty() && resp.find("\"Answer\"") != std::string::npos;
    }

    void checkBreachDirectory(const std::string& email, ReconResult& result) {
        if (rapidApiKey.empty()) {
            result.addField("BreachDirectory", "No RapidAPI key provided");
            return;
        }

        log("Querying BreachDirectory for " + email);

        std::string url = "https://breachdirectory.p.rapidapi.com/?func=auto&term="
                          + http.urlEncode(email);

        std::map<std::string, std::string> headers = {
            {"X-RapidAPI-Key",  rapidApiKey},
            {"X-RapidAPI-Host", "breachdirectory.p.rapidapi.com"}
        };

        std::string resp = http.get(url, headers);

        if (resp.empty()) {
            result.addField("BreachDirectory", "Network error");
            return;
        }

        if (resp.find("\"found\":false") != std::string::npos ||
            resp.find("\"result\":[]")   != std::string::npos) {
            result.addField("BreachDirectory", "No breaches found");
            return;
        }

        size_t pos = resp.find("\"result\"");
        if (pos == std::string::npos) {
            result.addField("BreachDirectory", "Unexpected response");
            return;
        }

        result.list.push_back("=== BreachDirectory Results ===");

        int breachCount = 0;
        size_t objStart = resp.find('{', pos + 1);
        while (objStart != std::string::npos) {
            size_t objEnd = resp.find('}', objStart);
            if (objEnd == std::string::npos) break;
            std::string obj = resp.substr(objStart, objEnd - objStart + 1);

            std::string source  = SimpleJSON::getString(obj, "sources");
            std::string hasPass = SimpleJSON::getString(obj, "password");

            if (!source.empty()) {
                std::string line = "• " + source;
                if (!hasPass.empty() && hasPass != "false")
                    line += " [password exposed]";
                result.list.push_back(line);
                breachCount++;
            }
            objStart = resp.find('{', objEnd + 1);
        }

        if (breachCount > 0)
            result.addField("BreachDirectory Status",
                "COMPROMISED — found in " + std::to_string(breachCount) + " breach(es)");
        else
            result.addField("BreachDirectory", "No breach details parsed");
    }

    void checkIntelX(const std::string& email, ReconResult& result) {
        if (intelxKey.empty()) {
            result.addField("IntelX", "No key provided");
            return;
        }

        log("Querying IntelX for " + email);

        std::string searchUrl = "https://2.intelx.io/intelligent/search";
        std::string body = "{\"term\":\"" + email + "\","
                           "\"maxresults\":10,"
                           "\"media\":0,"
                           "\"target\":0,"
                           "\"timeout\":20}";

        std::map<std::string, std::string> headers = {
            {"x-key",        intelxKey},
            {"Content-Type", "application/json"}
        };

        std::string searchResp = http.post(searchUrl, body, headers);

        if (searchResp.empty()) {
            result.addField("IntelX", "Blocked by network — run osint-x outside Codespaces for this feature");
            return;
        }

        std::string searchId = SimpleJSON::getString(searchResp, "id");
        if (searchId.empty()) {
            result.addField("IntelX", "No results or invalid key");
            return;
        }

        std::string resultUrl = "https://2.intelx.io/intelligent/search/result?id="
                                + searchId + "&limit=10";

        std::string resultResp = http.get(resultUrl, headers);

        if (resultResp.empty()) {
            result.addField("IntelX", "Network error fetching results");
            return;
        }

        result.list.push_back("=== IntelX Results (Paste/Dark Web) ===");

        int found = 0;
        size_t pos = resultResp.find("\"records\"");
        if (pos == std::string::npos) {
            result.addField("IntelX", "No records found");
            return;
        }

        size_t objStart = resultResp.find('{', pos + 1);
        while (objStart != std::string::npos && found < 10) {
            size_t objEnd = resultResp.find('}', objStart);
            if (objEnd == std::string::npos) break;
            std::string obj = resultResp.substr(objStart, objEnd - objStart + 1);

            std::string name   = SimpleJSON::getString(obj, "name");
            std::string bucket = SimpleJSON::getString(obj, "bucket");
            std::string date   = SimpleJSON::getString(obj, "date");

            if (!name.empty() || !bucket.empty()) {
                std::string line = "• ";
                if (!name.empty())   line += name;
                if (!bucket.empty()) line += " [" + bucket + "]";
                if (!date.empty())   line += " — " + date.substr(0, 10);
                result.list.push_back(line);
                found++;
            }
            objStart = resultResp.find('{', objEnd + 1);
        }

        if (found > 0)
            result.addField("IntelX Status",
                "Found in " + std::to_string(found) + " source(s)");
        else
            result.addField("IntelX", "No pastes or leaks found");
    }

public:
    EmailLookup(const std::string& rapidApiKey = "",
                const std::string& intelxKey   = "")
        : OSINTModule("EmailLookup", ""),
          rapidApiKey(rapidApiKey),
          intelxKey(intelxKey) {}

    bool validate(const std::string& query) const override {
        std::regex emailRe(R"(^[a-zA-Z0-9._%+\-]+@[a-zA-Z0-9.\-]+\.[a-zA-Z]{2,}$)");
        return std::regex_match(query, emailRe);
    }

    std::string helpText() const override {
        return "Usage: email <address>\nExample: email user@gmail.com";
    }

    ReconResult run(const std::string& query) override {
        ReconResult result;
        result.module = moduleName;
        result.query  = query;

        if (!validate(query)) {
            result.success = false;
            result.error   = "Invalid email format";
            return result;
        }

        std::string domain = extractDomain(query);
        result.addField("Email",  query);
        result.addField("Domain", domain);

        // 1. Disposable check
        bool isDisposable = disposableDomains.count(domain) > 0;
        result.addField("Disposable", isDisposable ? "YES — likely temporary" : "No");

        // 2. MX Record check via Google DNS
        log("Checking MX records for " + domain);
        bool hasMX = checkMXRecord(domain);
        result.addField("MX Record",   hasMX ? "Valid — domain accepts email" : "None found");
        result.addField("Email Valid", hasMX ? "Likely valid" : "Possibly invalid domain");

        // 3. BreachDirectory
        checkBreachDirectory(query, result);

        // 4. IntelX
        checkIntelX(query, result);

        // 5. Security guidelines
        result.list.push_back("=== Security Guidelines ===");
        result.list.push_back("Use a unique password for every service");
        result.list.push_back("Enable 2FA — use Google Authenticator or Authy");
        result.list.push_back("Use a password manager — Bitwarden is free and open source");
        result.list.push_back("Never reuse passwords across platforms");
        if (isDisposable)
            result.list.push_back("WARNING: Disposable email — avoid for important accounts");

        result.success = true;
        return result;
    }
};