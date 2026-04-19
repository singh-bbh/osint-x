#pragma once
#include "OSINTModule.hpp"
#include "../utils/HTTPClient.hpp"
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

class UsernameLookup : public OSINTModule {
private:
    struct Platform {
        std::string name;
        std::string urlTemplate;
        int         expectedCode;
    };

    struct ScanResult {
        std::string name;
        std::string url;
        bool        found;
    };

    std::vector<Platform> platforms = {
        // ── Dev / Tech ──
        {"GitHub",         "https://github.com/{}",                          200},
        {"GitLab",         "https://gitlab.com/{}",                          200},
        {"Bitbucket",      "https://bitbucket.org/{}",                       200},
        {"npm",            "https://www.npmjs.com/~{}",                      200},
        {"PyPI",           "https://pypi.org/user/{}",                       200},
        {"Dockerhub",      "https://hub.docker.com/u/{}",                    200},
        {"Codepen",        "https://codepen.io/{}",                          200},
        {"Replit",         "https://replit.com/@{}",                         200},
        {"Codesandbox",    "https://codesandbox.io/u/{}",                    200},
        {"HackerRank",     "https://www.hackerrank.com/{}",                  200},
        {"LeetCode",       "https://leetcode.com/{}",                        200},
        {"Kaggle",         "https://www.kaggle.com/{}",                      200},
        {"Codecademy",     "https://www.codecademy.com/profiles/{}",         200},
        {"HackerNews",     "https://news.ycombinator.com/user?id={}",        200},
        {"Dev.to",         "https://dev.to/{}",                              200},
        {"Hashnode",       "https://hashnode.com/@{}",                       200},
        {"SourceForge",    "https://sourceforge.net/u/{}/profile",           200},
        {"Exercism",       "https://exercism.org/profiles/{}",               200},
        {"Codeforces",     "https://codeforces.com/profile/{}",              200},
        {"Topcoder",       "https://www.topcoder.com/members/{}",            200},
        {"CodeChef",       "https://www.codechef.com/users/{}",              200},
        {"AtCoder",        "https://atcoder.jp/users/{}",                    200},
        {"Sourcehut",      "https://sr.ht/~{}",                              200},
        // ── Social Media ──
        {"Twitter/X",      "https://twitter.com/{}",                         200},
        {"Instagram",      "https://www.instagram.com/{}/",                  200},
        {"Facebook",       "https://www.facebook.com/{}",                    200},
        {"TikTok",         "https://www.tiktok.com/@{}",                     200},
        {"Snapchat",       "https://www.snapchat.com/add/{}",                200},
        {"Pinterest",      "https://www.pinterest.com/{}/",                  200},
        {"Tumblr",         "https://{}.tumblr.com",                          200},
        {"Flickr",         "https://www.flickr.com/people/{}",               200},
        {"VK",             "https://vk.com/{}",                              200},
        {"Telegram",       "https://t.me/{}",                                200},
        {"Clubhouse",      "https://www.joinclubhouse.com/@{}",              200},
        {"Mastodon",       "https://mastodon.social/@{}",                    200},
        {"Minds",          "https://www.minds.com/{}",                       200},
        {"Ello",           "https://ello.co/{}",                             200},
        {"Reddit",         "https://www.reddit.com/user/{}",                 200},
        {"Quora",          "https://www.quora.com/profile/{}",               200},
        // ── Video / Streaming ──
        {"YouTube",        "https://www.youtube.com/@{}",                    200},
        {"Twitch",         "https://www.twitch.tv/{}",                       200},
        {"Vimeo",          "https://vimeo.com/{}",                           200},
        {"Dailymotion",    "https://www.dailymotion.com/{}",                 200},
        {"Rumble",         "https://rumble.com/c/{}",                        200},
        {"Odysee",         "https://odysee.com/@{}",                         200},
        {"Kick",           "https://kick.com/{}",                            200},
        // ── Music ──
        {"Spotify",        "https://open.spotify.com/user/{}",               200},
        {"Soundcloud",     "https://soundcloud.com/{}",                      200},
        {"Bandcamp",       "https://{}.bandcamp.com",                        200},
        {"Mixcloud",       "https://www.mixcloud.com/{}/",                   200},
        {"Last.fm",        "https://www.last.fm/user/{}",                    200},
        {"Audiomack",      "https://audiomack.com/{}",                       200},
        // ── Professional ──
        {"LinkedIn",       "https://www.linkedin.com/in/{}",                 200},
        {"AngelList",      "https://angel.co/u/{}",                          200},
        {"ProductHunt",    "https://www.producthunt.com/@{}",                200},
        {"Indie Hackers",  "https://www.indiehackers.com/{}",                200},
        {"Fiverr",         "https://www.fiverr.com/{}",                      200},
        {"Upwork",         "https://www.upwork.com/freelancers/~{}",         200},
        {"Freelancer",     "https://www.freelancer.com/u/{}",                200},
        // ── Blogging / Writing ──
        {"Medium",         "https://medium.com/@{}",                         200},
        {"Substack",       "https://substack.com/@{}",                       200},
        {"Blogger",        "https://{}.blogspot.com",                        200},
        {"WordPress",      "https://{}.wordpress.com",                       200},
        {"Ghost",          "https://{}.ghost.io",                            200},
        {"WriteAs",        "https://write.as/{}",                            200},
        // ── Creative ──
        {"Behance",        "https://www.behance.net/{}",                     200},
        {"Dribbble",       "https://dribbble.com/{}",                        200},
        {"ArtStation",     "https://www.artstation.com/{}",                  200},
        {"DeviantArt",     "https://www.deviantart.com/{}",                  200},
        {"Wattpad",        "https://www.wattpad.com/user/{}",                200},
        {"500px",          "https://500px.com/p/{}",                         200},
        {"Unsplash",       "https://unsplash.com/@{}",                       200},
        {"Imgur",          "https://imgur.com/user/{}",                      200},
        {"Giphy",          "https://giphy.com/{}",                           200},
        // ── Gaming ──
        {"Steam",          "https://steamcommunity.com/id/{}",               200},
        {"Xbox",           "https://xboxgamertag.com/search/{}",             200},
        {"PSN",            "https://psnprofiles.com/{}",                     200},
        {"Roblox",         "https://www.roblox.com/user.aspx?username={}",   200},
        {"Minecraft",      "https://namemc.com/profile/{}",                  200},
        {"Faceit",         "https://www.faceit.com/en/players/{}",           200},
        {"Chess.com",      "https://www.chess.com/member/{}",                200},
        {"Lichess",        "https://lichess.org/@/{}",                       200},
        {"itch.io",        "https://{}.itch.io",                             200},
        {"GameJolt",       "https://gamejolt.com/@{}",                       200},
        {"Speedrun",       "https://www.speedrun.com/user/{}",               200},
        {"Newgrounds",     "https://{}.newgrounds.com",                      200},
        // ── Cybersecurity ──
        {"TryHackMe",      "https://tryhackme.com/p/{}",                     200},
        {"HackTheBox",     "https://app.hackthebox.com/profile/{}",          200},
        {"BugCrowd",       "https://bugcrowd.com/{}",                        200},
        {"HackerOne",      "https://hackerone.com/{}",                       200},
        {"Root-me",        "https://www.root-me.org/{}",                     200},
        // ── Forums ──
        {"Stack Overflow", "https://stackoverflow.com/users/{}",             200},
        {"Disqus",         "https://disqus.com/by/{}/",                      200},
        {"Instructables",  "https://www.instructables.com/member/{}",        200},
        {"Duolingo",       "https://www.duolingo.com/profile/{}",            200},
        {"Pastebin",       "https://pastebin.com/u/{}",                      200},
        {"Keybase",        "https://keybase.io/{}",                          200},
        {"Gravatar",       "https://gravatar.com/{}",                        200},
        {"About.me",       "https://about.me/{}",                            200},
        {"Linktree",       "https://linktr.ee/{}",                           200},
        // ── Finance ──
        {"Patreon",        "https://www.patreon.com/{}",                     200},
        {"Ko-fi",          "https://ko-fi.com/{}",                           200},
        {"BuyMeACoffee",   "https://www.buymeacoffee.com/{}",                200},
        {"PayPal",         "https://www.paypal.com/paypalme/{}",             200},
        {"Venmo",          "https://venmo.com/{}",                           200},
        // ── Indian platforms ──
        {"ShareChat",      "https://sharechat.com/profile/{}",               200},
        {"Koo",            "https://www.kooapp.com/profile/{}",              200},
        {"Moj",            "https://mojapp.in/@{}",                          200},
        {"Josh",           "https://share.myjosh.in/profile/{}",             200},
        // ── Shopping ──
        {"Etsy",           "https://www.etsy.com/people/{}",                 200},
        {"eBay",           "https://www.ebay.com/usr/{}",                    200},
        {"Poshmark",       "https://poshmark.com/closet/{}",                 200},
        {"Depop",          "https://www.depop.com/{}",                       200},
        {"Redbubble",      "https://www.redbubble.com/people/{}",            200},
        // ── Fitness ──
        {"Strava",         "https://www.strava.com/athletes/{}",             200},
        {"MyFitnessPal",   "https://www.myfitnesspal.com/profile/{}",        200},
        // ── Misc ──
        {"Academia",       "https://independent.academia.edu/{}",            200},
        {"ResearchGate",   "https://www.researchgate.net/profile/{}",        200},
        {"Goodreads",      "https://www.goodreads.com/{}",                   200},
        {"Letterboxd",     "https://letterboxd.com/{}",                      200},
        {"Genius",         "https://genius.com/{}",                          200},
        {"Tripadvisor",    "https://www.tripadvisor.com/Profile/{}",         200},
        {"Airbnb",         "https://www.airbnb.com/users/show/{}",           200},
        {"Eventbrite",     "https://www.eventbrite.com/u/{}",                200},
        {"Meetup",         "https://www.meetup.com/members/{}",              200},
        {"Figma",          "https://www.figma.com/@{}",                      200},
        {"Canva",          "https://www.canva.com/{}",                       200},
        {"Discord",        "https://discord.com/users/{}",                   200},
        {"Guilded",        "https://www.guilded.gg/profile/{}",              200},
        {"Coda",           "https://coda.io/@{}",                            200},
        {"Trello",         "https://trello.com/{}",                          200},
        {"Notion",         "https://notion.so/{}",                           200},
        {"Scribd",         "https://www.scribd.com/{}",                      200},
        {"Slideshare",     "https://www.slideshare.net/{}",                  200},
        {"Pocket",         "https://getpocket.com/@{}",                      200},
        {"Feedly",         "https://feedly.com/{}",                          200},
        // ── News / Journalism ──
        {"Medium",         "https://medium.com/@{}",                    200},
        {"Pressfolio",     "https://pressfolios.com/{}",                200},
        {"Muck Rack",      "https://muckrack.com/{}",                   200},
        {"Contently",      "https://contently.com/{}",                  200},

// ── Cybersecurity specific ──
        {"Vulners",        "https://vulners.com/author/{}",              200},
        {"Exploit-DB",     "https://www.exploit-db.com/author/{}",      200},
        {"0day.today",     "https://0day.today/author/{}",               200},
        {"Packet Storm",   "https://packetstormsecurity.com/files/author/{}",200},
        {"Shodan",         "https://www.shodan.io/member/{}",            200},
        {"Censys",         "https://censys.io/{}",                       200},
        {"VirusTotal",     "https://www.virustotal.com/gui/user/{}",     200},
        {"GreyNoise",      "https://viz.greynoise.io/analyst/{}",        200},

// ── Tech forums ──
        {"Lobsters",       "https://lobste.rs/u/{}",                     200},
        {"Tildes",         "https://tildes.net/user/{}",                 200},
        {"Slashdot",       "https://slashdot.org/~{}",                   200},
        {"Fark",           "https://cgi.fark.com/cgi/farkit.pl?a=UserPage&un={}", 200},
        {"Digg",           "https://digg.com/u/{}",                      200},
        {"Flipboard",      "https://flipboard.com/@{}",                  200},

// ── Open source / wiki ──
        {"Wikipedia",      "https://en.wikipedia.org/wiki/User:{}",      200},
        {"WikiData",       "https://www.wikidata.org/wiki/User:{}",      200},
        {"OpenStreetMap",  "https://www.openstreetmap.org/user/{}",      200},
        {"Wikimedia",      "https://commons.wikimedia.org/wiki/User:{}", 200},

// ── Crypto / Web3 ──
        {"Etherscan",      "https://etherscan.io/address/{}",            200},
        {"OpenSea",        "https://opensea.io/{}",                      200},
        {"Rarible",        "https://rarible.com/user/{}",                200},
        {"Foundation",     "https://foundation.app/@{}",                 200},
        {"Mirror",         "https://mirror.xyz/{}",                      200},

// ── Podcast / Audio ──
        {"Anchor",         "https://anchor.fm/{}",                       200},
        {"Podbean",        "https://{}.podbean.com",                     200},
        {"Spreaker",       "https://www.spreaker.com/user/{}",           200},
        {"Buzzsprout",     "https://www.buzzsprout.com/{}",              200},

// ── Education ──
        {"Coursera",       "https://www.coursera.org/user/{}",           200},
        {"Udemy",          "https://www.udemy.com/user/{}",              200},
        {"Skillshare",     "https://www.skillshare.com/profile/{}",      200},
        {"Khan Academy",   "https://www.khanacademy.org/profile/{}",     200},
        {"Chegg",          "https://www.chegg.com/homework-help/{}",     200},
        {"Brainly",        "https://brainly.com/app/profile/{}",         200},
    };

    std::string buildURL(const std::string& tmpl,
                         const std::string& username) const {
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

        std::cout << "[*] Scanning " << platforms.size()
                  << " platforms for: " << query << "\n";

        // ── Multithreaded scanning ──
        std::vector<ScanResult> scanResults(platforms.size());
        std::vector<std::thread> threads;
        std::mutex printMtx;

        // Split into batches of 20 threads at a time
        // to avoid overwhelming the network
        size_t batchSize = 20;
        size_t total     = platforms.size();

        for (size_t batchStart = 0; batchStart < total; batchStart += batchSize) {
            size_t batchEnd = std::min(batchStart + batchSize, total);
            threads.clear();

            for (size_t i = batchStart; i < batchEnd; i++) {
                threads.emplace_back([&, i]() {
                    // Each thread gets its own HTTPClient
                    HTTPClient localHttp;
                    std::string url  = buildURL(platforms[i].urlTemplate, query);
                    long        code = localHttp.headStatus(url);
                    bool        hit  = (code == 200);

                    scanResults[i] = { platforms[i].name, url, hit };

                    std::lock_guard<std::mutex> lock(printMtx);
                    if (hit)
                        std::cout << "  [+] FOUND: " << platforms[i].name << "\n";
                });
            }

            // Wait for this batch to finish before starting next
            for (auto& t : threads) t.join();
        }

        // Tally results
        int found = 0, notFound = 0;
        std::vector<std::string> foundList, notFoundList;

        for (auto& sr : scanResults) {
            if (sr.found) {
                foundList.push_back("[FOUND] " + sr.name + " → " + sr.url);
                found++;
            } else {
                notFoundList.push_back("[NOT FOUND] " + sr.name);
                notFound++;
            }
        }

        result.addField("Platforms Checked", std::to_string(platforms.size()));
        result.addField("Found",             std::to_string(found));
        result.addField("Not Found",         std::to_string(notFound));

        // Found first then not found
        for (auto& s : foundList)    result.list.push_back(s);
        for (auto& s : notFoundList) result.list.push_back(s);

        return result;
    }
};