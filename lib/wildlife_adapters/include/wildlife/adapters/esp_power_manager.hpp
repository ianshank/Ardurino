#pragma once

#include "wildlife/domain/board_profile.hpp"
#include "wildlife/domain/runtime_config.hpp"
#include "wildlife/io/ipower_transport.hpp"

namespace wildlife {

class EspPowerManager final : public IPowerManager {
  public:
    EspPowerManager(const BoardProfile& board, const RuntimeConfig::Power& power_cfg,
                    const RuntimeConfig::Battery& battery_cfg) noexcept
        : _board(&board), _power_cfg(power_cfg), _battery_cfg(battery_cfg) {}

    WakeCause wake_cause() const noexcept override;
    void enter_sleep(uint32_t seconds) noexcept override;
    int32_t battery_mv() noexcept override;

  private:
    const BoardProfile* _board;
    RuntimeConfig::Power _power_cfg;
    RuntimeConfig::Battery _battery_cfg;
};

} // namespace wildlife