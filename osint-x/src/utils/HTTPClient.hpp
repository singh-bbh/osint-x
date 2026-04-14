#pragma once
#include <string>
#include <map>
#include <curl/curl.h>

class HTTPClient {
private:
    CURL* handle;

    static size_t writeCallback(char* ptr, size_t size, size_t nmemb, std::string* data) {
        data->append(ptr, size * nmemb);
        return size * nmemb;
    }

public:
    HTTPClient() {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        handle = curl_easy_init();
    }
    ~HTTPClient() {
        if (handle) curl_easy_cleanup(handle);
        curl_global_cleanup();
    }

    std::string get(const std::string& url,
                    const std::map<std::string, std::string>& headers = {}) {
        if (!handle) return "";
        std::string response;
        curl_easy_reset(handle);
        curl_easy_setopt(handle, CURLOPT_URL,           url.c_str());
        curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(handle, CURLOPT_WRITEDATA,     &response);
        curl_easy_setopt(handle, CURLOPT_USERAGENT,     "osint-x/1.0");
        curl_easy_setopt(handle, CURLOPT_TIMEOUT,       10L);
        curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION,1L);
        curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER,1L);

        struct curl_slist* hlist = nullptr;
        for (auto& [k, v] : headers)
            hlist = curl_slist_append(hlist, (k + ": " + v).c_str());
        if (hlist) curl_easy_setopt(handle, CURLOPT_HTTPHEADER, hlist);

        CURLcode res = curl_easy_perform(handle);
        if (hlist) curl_slist_free_all(hlist);
        return (res == CURLE_OK) ? response : "";
    }

    std::string post(const std::string& url, const std::string& body,
                     const std::map<std::string, std::string>& headers = {}) {
        if (!handle) return "";
        std::string response;
        curl_easy_reset(handle);
        curl_easy_setopt(handle, CURLOPT_URL,           url.c_str());
        curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(handle, CURLOPT_WRITEDATA,     &response);
        curl_easy_setopt(handle, CURLOPT_USERAGENT,     "osint-x/1.0");
        curl_easy_setopt(handle, CURLOPT_TIMEOUT,       15L);
        curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION,1L);
        curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER,1L);
        curl_easy_setopt(handle, CURLOPT_POSTFIELDS,    body.c_str());

        struct curl_slist* hlist = nullptr;
        for (auto& [k, v] : headers)
            hlist = curl_slist_append(hlist, (k + ": " + v).c_str());
        if (hlist) curl_easy_setopt(handle, CURLOPT_HTTPHEADER, hlist);

        CURLcode res = curl_easy_perform(handle);
        if (hlist) curl_slist_free_all(hlist);
        return (res == CURLE_OK) ? response : "";
    }

    // For username checking — just need the HTTP status code
    long headStatus(const std::string& url) {
        if (!handle) return 0;
        curl_easy_reset(handle);
        curl_easy_setopt(handle, CURLOPT_URL,            url.c_str());
        curl_easy_setopt(handle, CURLOPT_NOBODY,         1L);
        curl_easy_setopt(handle, CURLOPT_TIMEOUT,        8L);
        curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(handle, CURLOPT_USERAGENT,      "osint-x/1.0");
        curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_perform(handle);
        long code = 0;
        curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &code);
        return code;
    }

    std::string urlEncode(const std::string& s) {
        char* enc = curl_easy_escape(handle, s.c_str(), (int)s.size());
        std::string result(enc);
        curl_free(enc);
        return result;
    }
};