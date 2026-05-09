#pragma once

// Test utility — FakeInferenceSource: controllable IInferenceSource.

#include "wildlife/io/iperipheral.hpp"

#include <vector>

namespace wildlife {
namespace test {

class FakeInferenceSource : public IInferenceSource {
  public:
    void set_invoke_result(bool ok) noexcept { _invoke_ok = ok; }
    void set_healthy(bool h) noexcept { _healthy = h; }
    void set_detections(std::vector<Detection> d) noexcept { _detections = std::move(d); }

    bool invoke() noexcept override {
        ++_invoke_count;
        return _invoke_ok;
    }

    const std::vector<Detection>& detections() const noexcept override { return _detections; }

    bool healthy() const noexcept override { return _healthy; }

    uint32_t invoke_count() const noexcept { return _invoke_count; }

  private:
    bool _invoke_ok{true};
    bool _healthy{true};
    std::vector<Detection> _detections;
    uint32_t _invoke_count{0};
};

} // namespace test
} // namespace wildlife
