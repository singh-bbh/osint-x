#pragma once
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include "../utils/SimpleJSON.hpp"  

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

        j << "\"module\":\"" << SimpleJSON::escape(module) << "\",";
        j << "\"query\":\"" << SimpleJSON::escape(query) << "\",";

        if (!error.empty())
            j << "\"error\":\"" << SimpleJSON::escape(error) << "\",";

        if (hasCoords) {
            j << "\"latitude\":"  << latitude  << ",";
            j << "\"longitude\":" << longitude << ",";
        }

        // Fields
        j << "\"fields\":{";
        bool first = true;
        for (const auto& [k, v] : fields) {
            if (!first) j << ",";
            j << "\"" << SimpleJSON::escape(k) << "\":\""
              << SimpleJSON::escape(v) << "\"";
            first = false;
        }
        j << "},";

        // List
        j << "\"list\":[";
        for (size_t i = 0; i < list.size(); i++) {
            if (i) j << ",";
            j << "\"" << SimpleJSON::escape(list[i]) << "\"";
        }
        j << "]";

        j << "}";

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

public:
    OSINTModule(const std::string& name, const std::string& key = "", bool v = false)
        : moduleName(name), apiKey(key), verbose(v) {}

    virtual ~OSINTModule() = default;

    virtual ReconResult run(const std::string& query) = 0;
    virtual bool        validate(const std::string& query) const = 0;
    virtual std::string helpText() const = 0;

    virtual void printResult(const ReconResult& r) const {
        std::cout << "\n[" << r.module << "] Results for: " << r.query << "\n";
        std::cout << std::string(50, '-') << "\n";
        if (!r.success) {
            std::cout << "ERROR: " << r.error << "\n";
            return;
        }
        for (const auto& [k, v] : r.fields)
            std::cout << "  " << k << ": " << v << "\n";
        for (const auto& item : r.list)
            std::cout << "  • " << item << "\n";
    }

    std::string getName() const { return moduleName; }
    void setApiKey(const std::string& key) { apiKey = key; }
    void setVerbose(bool v) { verbose = v; }
};