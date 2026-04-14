#pragma once
#include "OSINTModule.hpp"
#include "../utils/HTTPClient.hpp"
#include <vector>
#include <thread>
#include <mutex>

class UsernameLookup : public OSINTModule {
private:
    HTTPClient http;
    std::mutex mtx;

    struct Platform {
        std::string name;
        std::string urlTemplate;  // {} replaced with username
        int         expectedCode; // 200 = found, some sites use 404 for not found
    };

    std::vector<Platform> platforms = {
        {"GitHub",       "https://github.com/{}",                        200},
        {"GitLab",       "https://gitlab.com/{}",                        200},
        {"Twitter/X",    "https://twitter.com/{}",                       200},
        {"Instagram",    "https://www.instagram.com/{}/",                200},
        {"Reddit",       "https://www.reddit.com/user/{}",               200},
        {"LinkedIn",     "https://www.linkedin.com/in/{}",               200},
        {"TikTok",       "https://www.tiktok.com/@{}",                   200},
        {"YouTube",      "https://www.youtube.com/@{}",                  200},
        {"Pinterest",    "https://www.pinterest.com/{}/",                200},
        {"Twitch",       "https://www.twitch.tv/{}",                     200},
        {"Steam",        "https://steamcommunity.com/id/{}",             200},
        {"Medium",       "https://medium.com/@{}",                       200},
        {"Dev.to",       "https://dev.to/{}",                            200},
        {"Keybase",      "https://keybase.io/{}",                        200},
        {"HackerNews",   "https://news.ycombinator.com/user?id={}",      200},
        {"Pastebin",     "https://pastebin.com/u/{}",                    200},
        {"Replit",       "https://replit.com/@{}",                       200},
        {"Codecademy",   "https://www.codecademy.com/profiles/{}",       200},
        {"Fiverr",       "https://www.fiverr.com/{}",                    200},
        {"Kaggle",       "https://www.kaggle.com/{}",                    200},
        {"HackerRank",   "https://www.hackerrank.com/{}",                200},
        {"LeetCode",     "https://leetcode.com/{}",                      200},
        {"Linktree",     "https://linktr.ee/{}",                         200},
        {"About.me",     "https://about.me/{}",                          200},
        {"Gravatar",     "https://gravatar.com/{}",                      200},
        {"Blogger",      "https://{}.blogspot.com",                      200},
        {"WordPress",    "https://{}.wordpress.com",                     200},
        {"Mastodon",     "https://mastodon.social/@{}",                  200},
        {"Snapchat",     "https://www.snapchat.com/add/{}",              200},
        {"Spotify",      "https://open.spotify.com/user/{}",             200},
    };

    std::string buildURL(const std::string& tmpl, const std::string& username) const {
        std::string url = tmpl;
        size_t pos = url.find("{}");
        if (pos != std::string::npos) url.replace(pos, 2, username);
        return url;
    }

public:
    UsernameLookup() : OSINTModule("UsernameLookup") {}

    bool validate(const std::string& query) const override {
        if (query.size() < 1 || query.size() > 40) return false;
        for (char c : query)
            if (!std::isalnum(c) && c != '_' && c != '-' && c != '.') return false;
        return true;
    }

    std::string helpText() const override {
        return "Usage: username <handle>\nExample: username johndoe\nSearches " +
               std::to_string(platforms.size()) + " platforms";
    }

    ReconResult run(const std::string& query) override {
        ReconResult result;
        result.module  = moduleName;
        result.query   = query;
        result.success = true;

        if (!validate(query)) {
            result.success = false;
            result.error   = "Invalid username — use only letters, numbers, _ - .";
            return result;
        }

        int found = 0, notFound = 0;
        std::vector<std::string> foundList, notFoundList;

        std::cout << "[*] Scanning " << platforms.size()
                  << " platforms for username: " << query << "\n";

        for (auto& p : platforms) {
            std::string url  = buildURL(p.urlTemplate, query);
            long        code = http.headStatus(url);
            bool        hit  = (code == 200);

            if (hit) {
                foundList.push_back("[FOUND] " + p.name + " → " + url);
                found++;
                std::cout << "  [+] FOUND: " << p.name << "\n";
            } else {
                notFoundList.push_back("[NOT FOUND] " + p.name);
                notFound++;
            }
        }

        result.addField("Platforms Checked", std::to_string(platforms.size()));
        result.addField("Found",             std::to_string(found));
        result.addField("Not Found",         std::to_string(notFound));

        for (auto& s : foundList)    result.list.push_back(s);
        for (auto& s : notFoundList) result.list.push_back(s);

        return result;
    }
};