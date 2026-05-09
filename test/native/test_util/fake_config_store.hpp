#pragma once

// Test utility — FakeConfigStore: in-memory IConfigStore for host tests.

#include "wildlife/io/iconfig_store.hpp"
#include <string>
#include <unordered_map>

namespace wildlife {
namespace test {

class FakeConfigStore : public IConfigStore {
public:
    // Pre-seed a path with content (useful for read-only scenarios).
    void seed(const std::string& path, const std::string& content) {
        _store[path] = content;
    }

    bool read(const std::string& path, std::string& out) noexcept override {
        auto it = _store.find(path);
        if (it == _store.end()) return false;
        out = it->second;
        return true;
    }

    bool write(const std::string& path, const std::string& data) noexcept override {
        _store[path] = data;
        return true;
    }

    bool exists(const std::string& path) noexcept override {
        return _store.count(path) > 0;
    }

    bool remove(const std::string& path) noexcept override {
        return _store.erase(path) > 0;
    }

private:
    std::unordered_map<std::string, std::string> _store;
};

} // namespace test
} // namespace wildlife
