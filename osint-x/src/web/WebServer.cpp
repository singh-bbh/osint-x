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

int main() {
    crow::SimpleApp app;

    // ── Module instances (objects from your classes)
    IPLookup       ipMod;
    PhoneLookup    phoneMod("d8142dfbaebbdfb1310283e4abcd332e");         
    EmailLookup    emailMod("629f58f594mshbcc57d1906dd330p1ab9dajsn78ca0cc8e740","a2f5f105-b35d-45a0-9faf-b47235f4d190");         
    UsernameLookup userMod;
    WHOISLookup    whoisMod("95720f3168c3427f8a171dfdda8e33bc");

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
        crow::response res(ipMod.run(query).toJSON());
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        return res;
    });

    CROW_ROUTE(app, "/api/phone/<string>")([&phoneMod](const std::string& query){
        crow::response res(phoneMod.run(query).toJSON());
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        return res;
    });

    CROW_ROUTE(app, "/api/email/<string>")([&emailMod](const std::string& query){
        crow::response res(emailMod.run(query).toJSON());
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        return res;
    });

    CROW_ROUTE(app, "/api/username/<string>")([&userMod](const std::string& query){
        crow::response res(userMod.run(query).toJSON());
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        return res;
    });

    CROW_ROUTE(app, "/api/whois/<string>")([&whoisMod](const std::string& query){
        crow::response res(whoisMod.run(query).toJSON());
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        return res;
    });

    std::cout << "\n  osint-x web server running at http://localhost:8080\n\n";
    app.port(8080).multithreaded().run();
    return 0;
}