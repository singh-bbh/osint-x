#pragma once
#include "OSINTModule.hpp"
#include "../utils/HTTPClient.hpp"
#include "../utils/SimpleJSON.hpp"
#include "../utils/KeyRotator.hpp"
#include "../include/config.hpp"
#include <regex>
#include <set>

class EmailLookup : public OSINTModule {
private:
    HTTPClient http;
    KeyRotator keyPool;

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

    bool checkMXRecord(const std::string& domain) {
        std::string url  = "https://dns.google/resolve?name=" + domain + "&type=MX";
        std::string resp = http.get(url);
        return !resp.empty() && resp.find("\"Answer\"") != std::string::npos;
    }

    std::string friendlyFieldName(const std::string& field) const {
        std::string f = field;
        for (char& c : f) c = tolower(c);
        if (f == "email")       return "Email address";
        if (f == "password")    return "Password (plaintext)";
        if (f == "hash")        return "Password hash";
        if (f == "salt")        return "Password salt";
        if (f == "username")    return "Username";
        if (f == "name")        return "Full name";
        if (f == "phone")       return "Phone number";
        if (f == "address")     return "Physical address";
        if (f == "ip")          return "IP address";
        if (f == "dob")         return "Date of birth";
        if (f == "gender")      return "Gender";
        if (f == "country")     return "Country";
        if (f == "city")        return "City";
        if (f == "zip")         return "ZIP / Postal code";
        if (f == "credit_card") return "Credit card info";
        if (f == "ssn")         return "Social Security Number";
        if (f == "national_id") return "National ID";
        if (f == "employer")    return "Employer";
        if (f == "job_title")   return "Job title";
        if (f == "token")       return "Auth token";
        if (f == "2fa_secret")  return "2FA secret key";
        return field;
    }

    std::string fieldSeverity(const std::string& field) const {
        std::string f = field;
        for (char& c : f) c = tolower(c);
        if (f == "password" || f == "credit_card" || f == "ssn" ||
            f == "passport"  || f == "national_id" || f == "2fa_secret")
            return "CRITICAL";
        if (f == "hash" || f == "phone" || f == "address" ||
            f == "dob"  || f == "ip"    || f == "token")
            return "HIGH";
        if (f == "name" || f == "username" || f == "employer" || f == "city")
            return "MEDIUM";
        return "LOW";
    }

    // Fallback date if API doesn't return one
    std::string knownBreachDate(const std::string& source) const {
        std::string s = source;
        for (char& c : s) c = tolower(c);
        if (s.find("linkedin")   != std::string::npos) return "2012";
        if (s.find("adobe")      != std::string::npos) return "2013";
        if (s.find("canva")      != std::string::npos) return "2019";
        if (s.find("dropbox")    != std::string::npos) return "2012";
        if (s.find("twitter")    != std::string::npos) return "2022";
        if (s.find("facebook")   != std::string::npos) return "2021";
        if (s.find("instagram")  != std::string::npos) return "2019";
        if (s.find("zomato")     != std::string::npos) return "2017";
        if (s.find("bigbasket")  != std::string::npos) return "2020";
        if (s.find("mobikwik")   != std::string::npos) return "2021";
        if (s.find("byju")       != std::string::npos) return "2022";
        if (s.find("swiggy")     != std::string::npos) return "2020";
        if (s.find("paytm")      != std::string::npos) return "2018";
        if (s.find("hotstar")    != std::string::npos) return "2020";
        if (s.find("dominos")    != std::string::npos) return "2021";
        if (s.find("airtel")     != std::string::npos) return "2021";
        if (s.find("juspay")     != std::string::npos) return "2020";
        if (s.find("unacademy")  != std::string::npos) return "2020";
        if (s.find("dunzo")      != std::string::npos) return "2020";
        return "";
    }

    std::string adviceForBreach(const std::string& source) const {
        std::string s = source;
        for (char& c : s) c = tolower(c);
        if (s.find("linkedin")   != std::string::npos) return "Change LinkedIn password + any reused passwords";
        if (s.find("adobe")      != std::string::npos) return "Adobe breach exposed hints — rotate passwords";
        if (s.find("facebook")   != std::string::npos) return "Review Facebook privacy settings";
        if (s.find("canva")      != std::string::npos) return "Change Canva password immediately";
        if (s.find("dropbox")    != std::string::npos) return "Enable 2FA on Dropbox immediately";
        if (s.find("twitter")    != std::string::npos) return "Change Twitter/X password + enable 2FA";
        if (s.find("instagram")  != std::string::npos) return "Enable 2FA on Instagram immediately";
        if (s.find("zomato")     != std::string::npos) return "Change Zomato password";
        if (s.find("bigbasket")  != std::string::npos) return "Change BigBasket password immediately";
        if (s.find("mobikwik")   != std::string::npos) return "Change MobiKwik password + check wallet";
        if (s.find("byju")       != std::string::npos) return "Change Byjus credentials immediately";
        if (s.find("swiggy")     != std::string::npos) return "Change Swiggy password immediately";
        if (s.find("paytm")      != std::string::npos) return "Change Paytm password + check transactions";
        if (s.find("hotstar")    != std::string::npos) return "Change Hotstar/Disney+ password";
        if (s.find("dominos")    != std::string::npos) return "Change Dominos account password";
        if (s.find("airtel")     != std::string::npos) return "Change Airtel account password";
        if (s.find("juspay")     != std::string::npos) return "Monitor payment cards linked to Juspay";
        if (s.find("unacademy")  != std::string::npos) return "Change Unacademy password immediately";
        return "Change password for this service + enable 2FA";
    }

    void checkLeakCheck(const std::string& email, ReconResult& result) {
        if (keyPool.empty()) {
            result.addField("Breach Check", "No LeakCheck keys in config.hpp");
            return;
        }

        log("Querying LeakCheck for " + email);

        std::string key  = keyPool.next();
        std::string url  = "https://leakcheck.io/api/public?key=" +
                            key + "&check=" + http.urlEncode(email);
        std::string resp = http.get(url);

        // Rotate key if rate limited
        if (resp.empty() || resp.find("\"success\":false") != std::string::npos) {
            key  = keyPool.next();
            url  = "https://leakcheck.io/api/public?key=" +
                    key + "&check=" + http.urlEncode(email);
            resp = http.get(url);
        }

        if (resp.empty()) {
            result.addField("Breach Check", "Network error — could not reach LeakCheck");
            return;
        }

        if (resp.find("\"success\":false") != std::string::npos ||
            resp.find("\"found\":0")        != std::string::npos ||
            resp.find("not found")          != std::string::npos) {
            result.addField("Breach Status", "No breaches found — email looks clean");
            return;
        }

        std::string foundCount = SimpleJSON::getString(resp, "found");
        if (!foundCount.empty() && foundCount != "0")
            result.addField("Breach Status",
                "COMPROMISED — found in " + foundCount + " breach source(s)");

        result.list.push_back("=== Breach Details ===");

        size_t pos = resp.find("\"sources\"");
        if (pos == std::string::npos) return;

        pos = resp.find('[', pos);
        if (pos == std::string::npos) return;

        int breachIdx = 0;

        while (pos < resp.size()) {
            size_t objStart = resp.find('{', pos);
            if (objStart == std::string::npos) break;

            // Find matching closing brace — handles nested objects
            int depth = 1;
            size_t objEnd = objStart + 1;
            while (objEnd < resp.size() && depth > 0) {
                if      (resp[objEnd] == '{') depth++;
                else if (resp[objEnd] == '}') depth--;
                objEnd++;
            }
            if (depth != 0) break;

            std::string obj = resp.substr(objStart, objEnd - objStart);

            std::string sourceName = SimpleJSON::getString(obj, "name");
            if (sourceName.empty()) { pos = objEnd; continue; }

            breachIdx++;

            // Try to get date from API response first
            std::string breachDate = SimpleJSON::getString(obj, "date");
            if (breachDate.empty())
                breachDate = SimpleJSON::getString(obj, "breach_date");
            if (breachDate.empty())
                breachDate = SimpleJSON::getString(obj, "year");

            // Trim date to just year if it's a full timestamp
            if (breachDate.size() > 4) {
                // e.g. "2021-03-15" → "2021-03-15"  keep it, looks good
                // e.g. "2021-03-15T00:00:00Z" → trim at T
                size_t tPos = breachDate.find('T');
                if (tPos != std::string::npos)
                    breachDate = breachDate.substr(0, tPos);
            }

            // Fallback to hardcoded if API gave nothing
            if (breachDate.empty())
                breachDate = knownBreachDate(sourceName);

            // Build breach header line
            std::string header = "Breach #" + std::to_string(breachIdx) +
                                 ": " + sourceName;
            if (!breachDate.empty())
                header += " (breached: " + breachDate + ")";

            result.list.push_back(header);

            // Parse fields array
            size_t fieldsPos = obj.find("\"fields\"");
            if (fieldsPos != std::string::npos) {
                size_t arrStart = obj.find('[', fieldsPos);
                size_t arrEnd   = obj.find(']', arrStart);

                if (arrStart != std::string::npos && arrEnd != std::string::npos) {
                    std::string arrContent = obj.substr(arrStart + 1,
                                                        arrEnd - arrStart - 1);
                    std::vector<std::string> leakedFields;
                    size_t fpos = 0;
                    while (fpos < arrContent.size()) {
                        size_t qStart = arrContent.find('"', fpos);
                        if (qStart == std::string::npos) break;
                        qStart++;
                        size_t qEnd = arrContent.find('"', qStart);
                        if (qEnd == std::string::npos) break;
                        std::string fieldName = arrContent.substr(qStart,
                                                                   qEnd - qStart);
                        if (!fieldName.empty())
                            leakedFields.push_back(fieldName);
                        fpos = qEnd + 1;
                    }

                    if (!leakedFields.empty()) {
                        result.list.push_back("  Exposed data types:");
                        for (auto& f : leakedFields) {
                            result.list.push_back("  [" + fieldSeverity(f) + "] " +
                                                   friendlyFieldName(f));
                        }
                    }
                }
            }

            result.list.push_back("  Action: " + adviceForBreach(sourceName));
            result.list.push_back("");

            pos = objEnd;
            if (pos < resp.size() && resp[pos] == ']') break;
        }

        result.addField("Data Source", "LeakCheck.io");
    }

public:
    EmailLookup() : OSINTModule("EmailLookup", ""),
                keyPool(APIConfig::leakcheckKeys()) {}

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

        bool isDisposable = disposableDomains.count(domain) > 0;
        result.addField("Disposable", isDisposable ? "YES — likely temporary" : "No");

        log("Checking MX records for " + domain);
        bool hasMX = checkMXRecord(domain);
        result.addField("MX Record",   hasMX ? "Valid — domain accepts email" : "None found");
        result.addField("Email Valid", hasMX ? "Likely valid" : "Possibly invalid domain");

        checkLeakCheck(query, result);

        result.list.push_back("=== Security Guidelines ===");
        result.list.push_back("Use a unique password for every service");
        result.list.push_back("Enable 2FA — use Google Authenticator or Authy");
        result.list.push_back("Use a password manager — Bitwarden is free and open source");
        result.list.push_back("Never reuse passwords across platforms");
        result.list.push_back("Monitor your email regularly at leakcheck.io");
        if (isDisposable)
            result.list.push_back("WARNING: Disposable email — avoid for important accounts");

        result.success = true;
        return result;
    }
};