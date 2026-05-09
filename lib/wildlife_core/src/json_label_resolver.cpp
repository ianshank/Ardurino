#include "wildlife/app/json_label_resolver.hpp"

#include "wildlife/app/snapshot_chunker.hpp" // sha256_hex
#include "wildlife/json/mini_json.hpp"

#include <cstdio>
#include <string>

namespace wildlife {
namespace app {

JsonLabelResolver::JsonLabelResolver(ILogger* log) noexcept : _log(log) {}

bool JsonLabelResolver::load(IConfigStore& store, const std::string& path) noexcept {
    std::string raw;
    if (!store.read(path, raw)) {
        if (_log) {
            char buf[128];
            std::snprintf(buf, sizeof(buf), "LabelResolver: cannot read '%s'", path.c_str());
            _log->error(buf);
        }
        return false;
    }

    // Compute hash before parsing
    _hash = sha256_hex(reinterpret_cast<const uint8_t*>(raw.data()), raw.size());

    auto root = json::parse(raw);
    if (!root || root->type() != json::JType::Object) {
        if (_log) {
            char buf[128];
            std::snprintf(buf, sizeof(buf), "LabelResolver: JSON parse failed for '%s'",
                          path.c_str());
            _log->error(buf);
        }
        return false;
    }

    _labels.clear();
    for (const auto& [k, v] : root->as_object_ref()) {
        if (v.type() != json::JType::Str)
            continue;
        // Keys are expected to be stringified integers: "0", "1", …
        char* endp = nullptr;
        long id = std::strtol(k.c_str(), &endp, 10);
        if (endp == k.c_str() || *endp != '\0')
            continue; // non-integer key
        _labels.emplace(static_cast<int32_t>(id), std::string(v.as_str()));
    }

    _loaded = true;
    if (_log) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "LabelResolver: loaded %zu labels from '%s'",
                      _labels.size(), path.c_str());
        _log->info(buf);
    }
    return true;
}

std::string JsonLabelResolver::name(int32_t class_id) const noexcept {
    auto it = _labels.find(class_id);
    if (it != _labels.end())
        return it->second;
    char buf[24];
    std::snprintf(buf, sizeof(buf), "class_%d", class_id);
    return buf;
}

} // namespace app
} // namespace wildlife
