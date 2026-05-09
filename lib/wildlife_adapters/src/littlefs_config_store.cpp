#include "wildlife/adapters/littlefs_config_store.hpp"

#include <LittleFS.h>

namespace wildlife {

bool LittleFsConfigStore::ensure_mounted() noexcept {
    if (_mounted) {
        return true;
    }
    if (!_auto_begin) {
        return false;
    }

    _mounted = LittleFS.begin(false);
    return _mounted;
}

bool LittleFsConfigStore::read(const std::string& path, std::string& out) noexcept {
    out.clear();
    if (!ensure_mounted()) {
        return false;
    }

    File file = LittleFS.open(path.c_str(), FILE_READ);
    if (!file) {
        return false;
    }

    const std::size_t size = static_cast<std::size_t>(file.size());
    out.resize(size);
    if (size > 0) {
        const std::size_t read = file.read(reinterpret_cast<uint8_t*>(&out[0]), size);
        if (read != size) {
            // Short read — surface the partial payload to the caller and let
            // them decide. We still return false to flag the inconsistency.
            out.resize(read);
            file.close();
            return false;
        }
    }
    file.close();
    return true;
}

bool LittleFsConfigStore::write(const std::string& path, const std::string& data) noexcept {
    if (!ensure_mounted()) {
        return false;
    }

    const std::string temp_path = path + ".tmp";
    LittleFS.remove(temp_path.c_str());

    File file = LittleFS.open(temp_path.c_str(), FILE_WRITE);
    if (!file) {
        return false;
    }

    const auto* bytes = reinterpret_cast<const uint8_t*>(data.data());
    const auto written = file.write(bytes, data.size());
    file.close();
    if (written != data.size()) {
        LittleFS.remove(temp_path.c_str());
        return false;
    }

    LittleFS.remove(path.c_str());
    if (!LittleFS.rename(temp_path.c_str(), path.c_str())) {
        LittleFS.remove(temp_path.c_str());
        return false;
    }
    return true;
}

bool LittleFsConfigStore::exists(const std::string& path) noexcept {
    return ensure_mounted() && LittleFS.exists(path.c_str());
}

bool LittleFsConfigStore::remove(const std::string& path) noexcept {
    return ensure_mounted() && LittleFS.remove(path.c_str());
}

} // namespace wildlife