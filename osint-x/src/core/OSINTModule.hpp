#pragma once
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>

struct ReconResult {
    bool        success;
    std::string module;
    std::string query;
    std::string error;
    std::map<std::string, std::string> fields;
    std::vector<std::string>           list;
    double latitude  = 0.0;
    double longitude = 0.0;
    bool   hasCoords = false;

    void addField(const std::string& key, const std::string& val) {
        fields[key] = val;
    }

    std::string toJSON() const {
        std::ostringstream j;
        j << "{";
        j << "\"success\":" << (success ? "true" : "false") << ",";
        j << "\"module\":\"" << module << "\",";
        j << "\"query\":\"" << query << "\",";
        if (!error.empty()) j << "\"error\":\"" << error << "\",";
        if (hasCoords) {
            j << "\"latitude\":"  << latitude  << ",";
            j << "\"longitude\":" << longitude << ",";
        }
        j << "\"fields\":{";
        bool first = true;
        for (auto& [k, v] : fields) {
            if (!first) j << ",";
            j << "\"" << k << "\":\"" << v << "\"";
            first = false;
        }
        j << "},\"list\":[";
        for (size_t i = 0; i < list.size(); i++) {
            if (i) j << ",";
            j << "\"" << list[i] << "\"";
        }
        j << "]}";
        return j.str();
    }
};

// Abstract Base Class — Abstraction + Encapsulation
class OSINTModule {
protected:
    std::string moduleName;
    std::string apiKey;
    bool        verbose;

    void log(const std::string& msg) const {
        if (verbose) std::cerr << "[" << moduleName << "] " << msg << "\n";
    }

    std::string jsonEscape(const std::string& s) const {
        std::string out;
        for (char c : s) {
            if      (c == '"')  out += "\\\"";
            else if (c == '\\') out += "\\\\";
            else if (c == '\n') out += "\\n";
            else                out += c;
        }
        return out;
    }

public:
    OSINTModule(const std::string& name, const std::string& key = "", bool v = false)
        : moduleName(name), apiKey(key), verbose(v) {}

    virtual ~OSINTModule() = default;

    // Pure virtual — forces all child classes to implement these
    virtual ReconResult run(const std::string& query) = 0;
    virtual bool        validate(const std::string& query) const = 0;
    virtual std::string helpText() const = 0;

    // Virtual — can be overridden (Polymorphism)
    virtual void printResult(const ReconResult& r) const {
        std::cout << "\n[" << r.module << "] Results for: " << r.query << "\n";
        std::cout << std::string(50, '-') << "\n";
        if (!r.success) { std::cout << "ERROR: " << r.error << "\n"; return; }
        for (auto& [k, v] : r.fields)
            std::cout << "  " << k << ": " << v << "\n";
        for (auto& item : r.list)
            std::cout << "  • " << item << "\n";
    }

    std::string getName() const { return moduleName; }
    void setApiKey(const std::string& key) { apiKey = key; }
    void setVerbose(bool v) { verbose = v; }
};