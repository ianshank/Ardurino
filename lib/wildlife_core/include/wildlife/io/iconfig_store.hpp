#pragma once
#include <string>

namespace wildlife {

// Storage abstraction for reading/writing raw text files (config, labels).
// Concrete implementation lives in wildlife_adapters (LittleFsConfigStore).
class IConfigStore {
  public:
    virtual ~IConfigStore() = default;
    virtual bool read(const std::string& path, std::string& out) noexcept = 0;
    virtual bool write(const std::string& path, const std::string& data) noexcept = 0;
    virtual bool exists(const std::string& path) noexcept = 0;
    virtual bool remove(const std::string& path) noexcept = 0;
};

} // namespace wildlife
