#pragma once

#include "wildlife/io/iconfig_store.hpp"

namespace wildlife {

class LittleFsConfigStore final : public IConfigStore {
public:
    explicit LittleFsConfigStore(bool auto_begin = true) noexcept
        : _auto_begin(auto_begin) {}

    bool read(const std::string& path, std::string& out) noexcept override;
    bool write(const std::string& path, const std::string& data) noexcept override;
    bool exists(const std::string& path) noexcept override;
    bool remove(const std::string& path) noexcept override;

private:
    bool ensure_mounted() noexcept;

    bool _auto_begin;
    bool _mounted{false};
};

} // namespace wildlife