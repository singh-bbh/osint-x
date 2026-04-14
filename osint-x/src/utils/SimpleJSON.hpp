#pragma once
#include <string>
#include <vector>

// Lightweight JSON field extractor — no external dependency
class SimpleJSON {
public:
    static std::string getString(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\"";
        size_t pos = json.find(search);
        if (pos == std::string::npos) return "";
        pos = json.find(':', pos + search.size());
        if (pos == std::string::npos) return "";
        pos++;
        while (pos < json.size() && json[pos] == ' ') pos++;
        if (pos >= json.size()) return "";

        if (json[pos] == '"') {
            pos++;
            std::string result;
            while (pos < json.size() && json[pos] != '"') {
                if (json[pos] == '\\' && pos + 1 < json.size()) {
                    pos++;
                    if      (json[pos] == 'n') result += '\n';
                    else if (json[pos] == 't') result += '\t';
                    else result += json[pos];
                } else result += json[pos];
                pos++;
            }
            return result;
        } else {
            size_t end = json.find_first_of(",}\n", pos);
            if (end == std::string::npos) end = json.size();
            std::string val = json.substr(pos, end - pos);
            while (!val.empty() && val.back() == ' ') val.pop_back();
            return val;
        }
    }

    static bool getBool(const std::string& json, const std::string& key) {
        return getString(json, key) == "true";
    }

    static double getDouble(const std::string& json, const std::string& key) {
        try { return std::stod(getString(json, key)); } catch (...) { return 0.0; }
    }

    static std::vector<std::string> getArray(const std::string& json, const std::string& key) {
        std::vector<std::string> results;
        size_t pos = json.find("\"" + key + "\"");
        if (pos == std::string::npos) return results;
        pos = json.find('[', pos);
        if (pos == std::string::npos) return results;
        pos++;
        while (pos < json.size() && json[pos] != ']') {
            while (pos < json.size() &&
                   (json[pos]==' '||json[pos]=='\n'||json[pos]==',')) pos++;
            if (json[pos] == '"') {
                pos++;
                std::string item;
                while (pos < json.size() && json[pos] != '"') item += json[pos++];
                results.push_back(item);
                pos++;
            } else if (json[pos] == '{') {
                int d = 1; pos++;
                while (pos < json.size() && d > 0) {
                    if      (json[pos] == '{') d++;
                    else if (json[pos] == '}') d--;
                    pos++;
                }
            } else pos++;
        }
        return results;
    }
};