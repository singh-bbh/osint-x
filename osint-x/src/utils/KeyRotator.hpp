#pragma once
#include <vector>
#include <string>
#include <atomic>

class KeyRotator {
private:
    std::vector<std::string> keys;
    std::atomic<int>         index{0};

public:
    KeyRotator(const std::vector<std::string>& k) : keys(k) {}

    // Get current active key
    std::string current() const {
        if (keys.empty()) return "";
        return keys[index % keys.size()];
    }

    // Rotate to next key
    void rotate() {
        if (!keys.empty())
            index = (index + 1) % keys.size();
    }

    // Get key and auto-rotate for next call
    std::string next() {
        std::string k = current();
        rotate();
        return k;
    }

    bool empty() const { return keys.empty(); }
    int  size()  const { return keys.size(); }
};