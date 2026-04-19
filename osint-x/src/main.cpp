#include <iostream>
#include <string>
#include <map>
#include "core/OSINTModule.hpp"
#include "core/IPLookup.hpp"
#include "core/PhoneLookup.hpp"
#include "core/EmailLookup.hpp"
#include "core/UsernameLookup.hpp"
#include "core/WHOISLookup.hpp"

void printBanner() {
    std::cout << R"(
  ╔═══════════════════════════════════════╗
  ║          osint-x  v1.0                ║
  ║   Open Source Intelligence Toolkit    ║
  ║                   By Team Crawlers~   ║
  ╚═══════════════════════════════════════╝
)" << "\n";
}

void printHelp() {
    std::cout << "Commands:\n"
              << "  ip       <address>   IP geolocation\n"
              << "  phone    <number>    Phone intelligence\n"
              << "  email    <address>   Email validation + breach check\n"
              << "  username <handle>    Username lookup across 30 platforms\n"
              << "  whois    <domain>    WHOIS + DNS records\n"
              << "  help                 Show this menu\n"
              << "  exit                 Quit\n\n";
}

int main(int argc, char* argv[]) {
    printBanner();

    // Module map — demonstrates polymorphism
    // All accessed through the OSINTModule* base pointer
    std::map<std::string, OSINTModule*> modules;
    IPLookup       ipMod;
    PhoneLookup    phoneMod;   
    EmailLookup    emailMod;   // paste HIBP key here
    UsernameLookup userMod;
    WHOISLookup    whoisMod;

    modules["ip"]       = &ipMod;
    modules["phone"]    = &phoneMod;
    modules["email"]    = &emailMod;
    modules["username"] = &userMod;
    modules["whois"]    = &whoisMod;

    // One-shot mode: ./osint-x ip 8.8.8.8
    if (argc == 3) {
        std::string cmd   = argv[1];
        std::string query = argv[2];
        if (modules.count(cmd)) {
            auto result = modules[cmd]->run(query);
            modules[cmd]->printResult(result);
        } else {
            std::cerr << "Unknown module: " << cmd << "\n";
        }
        return 0;
    }

    // Interactive REPL mode
    printHelp();
    std::string line;
    std::cout << "osint-x> ";

    while (std::getline(std::cin, line)) {
        if (line.empty()) { std::cout << "osint-x> "; continue; }

        size_t space = line.find(' ');
        std::string cmd   = (space == std::string::npos) ? line : line.substr(0, space);
        std::string query = (space == std::string::npos) ? "" : line.substr(space + 1);

        if (cmd == "exit" || cmd == "quit") break;
        if (cmd == "help") { printHelp(); }
        else if (modules.count(cmd)) {
            if (query.empty()) {
                std::cout << modules[cmd]->helpText() << "\n";
            } else {
                auto result = modules[cmd]->run(query);
                modules[cmd]->printResult(result);
            }
        } else {
            std::cout << "Unknown command. Type 'help' for options.\n";
        }
        std::cout << "\nosint-x> ";
    }
    return 0;
}