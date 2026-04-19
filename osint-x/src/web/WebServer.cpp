#include "crow.h"
#include "../core/OSINTModule.hpp"
#include "../core/IPLookup.hpp"
#include "../core/PhoneLookup.hpp"
#include "../core/EmailLookup.hpp"
#include "../core/UsernameLookup.hpp"
#include "../core/WHOISLookup.hpp"
#include <fstream>
#include <sstream>

// Helper: read a file into a string
std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return "";
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// Helper: decode URL-encoded strings (%2B → +, %40 → @, etc.)
std::string urlDecode(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == '%' && i + 2 < str.size()) {
            int val = std::stoi(str.substr(i + 1, 2), nullptr, 16);
            result += static_cast<char>(val);
            i += 2;
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    return result;
}

int main() {
    crow::SimpleApp app;

    // ── Module instances (objects from your classes)
    IPLookup       ipMod;
    PhoneLookup    phoneMod;
    EmailLookup    emailMod;
    UsernameLookup userMod;
    WHOISLookup    whoisMod;

    // ── Serve frontend
    CROW_ROUTE(app, "/")([](){
        std::string html = readFile("frontend/index.html");
        crow::response res(html);
        res.set_header("Content-Type", "text/html");
        return res;
    });

    CROW_ROUTE(app, "/style.css")([](){
        std::string css = readFile("frontend/style.css");
        crow::response res(css);
        res.set_header("Content-Type", "text/css");
        return res;
    });

    CROW_ROUTE(app, "/app.js")([](){
        std::string js = readFile("frontend/app.js");
        crow::response res(js);
        res.set_header("Content-Type", "application/javascript");
        return res;
    });

    // ── API endpoints
    CROW_ROUTE(app, "/api/ip/<string>")([&ipMod](const std::string& query){
        std::string decoded = urlDecode(query);
        crow::response res(ipMod.run(decoded).toJSON());
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        return res;
    });

    CROW_ROUTE(app, "/api/phone/<string>")([&phoneMod](const std::string& query){
        std::string decoded = urlDecode(query);
        crow::response res(phoneMod.run(decoded).toJSON());
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        return res;
    });

    CROW_ROUTE(app, "/api/email/<string>")([&emailMod](const std::string& query){
        std::string decoded = urlDecode(query);
        crow::response res(emailMod.run(decoded).toJSON());
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        return res;
    });

    CROW_ROUTE(app, "/api/username/<string>")([&userMod](const std::string& query){
    auto result = userMod.run(query);
    crow::response res(result.toJSON());
    res.set_header("Content-Type", "application/json");
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Connection", "keep-alive");
    return res;
});

    CROW_ROUTE(app, "/api/whois/<string>")([&whoisMod](const std::string& query){
        std::string decoded = urlDecode(query);
        crow::response res(whoisMod.run(decoded).toJSON());
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        return res;
    });

    std::cout << "\n  osint-x web server running at http://localhost:8080\n\n";
    app.port(8080).multithreaded().run();
    return 0;
}