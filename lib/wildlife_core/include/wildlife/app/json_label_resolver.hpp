#pragma once

// Wildlife Core — JsonLabelResolver
// Loads labels from a JSON file via IConfigStore, resolves class_id → label.
// Falls back to "class_<id>" for unknown IDs.

#include "wildlife/io/iconfig_store.hpp"
#include "wildlife/io/ilogger.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace wildlife {
namespace app {

class JsonLabelResolver {
  public:
    // Optional logger for load errors and diagnostics.
    explicit JsonLabelResolver(ILogger* log = nullptr) noexcept;

    // Loads labels from `path` using `store`.
    // Returns false if the file cannot be read or JSON is malformed.
    bool load(IConfigStore& store, const std::string& path) noexcept;

    // Returns label for class_id; "class_<id>" if not found.
    std::string name(int32_t class_id) const noexcept;

    // SHA-256 hex digest of the raw file bytes (empty if not loaded).
    const std::string& hash() const noexcept { return _hash; }

    // True if load() has succeeded at least once.
    bool loaded() const noexcept { return _loaded; }

  private:
    ILogger* _log{nullptr};
    bool _loaded{false};
    std::unordered_map<int32_t, std::string> _labels;
    std::string _hash;
};

} // namespace app
} // namespace wildlife
