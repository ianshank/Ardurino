#pragma once

// Test utility — FakeOtaService: controllable IOtaService.

#include "wildlife/io/iota_service.hpp"
#include <string_view>

namespace wildlife {
namespace test {

class FakeOtaService : public IOtaService {
public:
    void set_check_result(OtaCheckResult r) noexcept { _check_result = r; }
    void set_apply_result(bool ok) noexcept          { _apply_ok = ok; }
    void set_version(std::string_view v) noexcept    { _version = std::string(v); }

    OtaCheckResult check(const OtaConfig&) noexcept override {
        ++_check_count;
        return _check_result;
    }

    bool apply(const OtaConfig&) noexcept override {
        ++_apply_count;
        return _apply_ok;
    }

    void mark_valid() noexcept override { ++_mark_valid_count; }

    std::string_view current_version() const noexcept override { return _version; }

    uint32_t check_count()      const noexcept { return _check_count; }
    uint32_t apply_count()      const noexcept { return _apply_count; }
    uint32_t mark_valid_count() const noexcept { return _mark_valid_count; }

private:
    OtaCheckResult _check_result{OtaCheckResult::UpToDate};
    bool           _apply_ok{true};
    std::string    _version{"0.0.0"};
    uint32_t       _check_count{0};
    uint32_t       _apply_count{0};
    uint32_t       _mark_valid_count{0};
};

} // namespace test
} // namespace wildlife
