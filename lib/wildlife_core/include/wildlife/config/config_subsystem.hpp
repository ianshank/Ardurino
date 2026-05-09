#pragma once

// Wildlife Core — Config subsystem
// ConfigParser:    JVal → RuntimeConfig (with type-safe, default-fallback reads)
// DefaultsMerger:  deep-merges two JVals (defaults < user)
// SchemaMigrator:  upgrades a config JVal from an old schema_version to current

#include "wildlife/domain/runtime_config.hpp"
#include "wildlife/io/ilogger.hpp"
#include "wildlife/json/mini_json.hpp"

#include <functional>
#include <string>
#include <vector>

namespace wildlife {
namespace config {

using JVal = json::JVal;

// ---------------------------------------------------------------------------
// ConfigParser
// ---------------------------------------------------------------------------
class ConfigParser {
  public:
    // Optional logger for parse warnings/errors. Null means silent.
    explicit ConfigParser(ILogger* log = nullptr) noexcept : _log(log) {}

    // Parses a RuntimeConfig from a merged JVal tree.
    // Returns false and populates error_msg if a required field is invalid.
    // Missing optional fields fall back to RuntimeConfig defaults silently.
    bool parse(const JVal& doc, RuntimeConfig& out, std::string& error_msg) noexcept;

  private:
    ILogger* _log{nullptr};

    void parse_wifi(const JVal& root, RuntimeConfig& out) noexcept;
    void parse_mqtt(const JVal& root, RuntimeConfig& out) noexcept;
    void parse_ota(const JVal& root, RuntimeConfig& out) noexcept;
    void parse_inference(const JVal& root, RuntimeConfig& out) noexcept;
    void parse_snapshot(const JVal& root, RuntimeConfig& out) noexcept;
    void parse_power(const JVal& root, RuntimeConfig& out) noexcept;
    void parse_battery(const JVal& root, RuntimeConfig& out) noexcept;
    void parse_labels(const JVal& root, RuntimeConfig& out) noexcept;
    void parse_logging(const JVal& root, RuntimeConfig& out) noexcept;
    void parse_network(const JVal& root, RuntimeConfig& out) noexcept;
};

// ---------------------------------------------------------------------------
// DefaultsMerger
// ---------------------------------------------------------------------------
class DefaultsMerger {
  public:
    // Returns a deep-merged JVal: keys in `user` override keys in `defaults`.
    // Unknown keys in `user` are preserved (forward-compatibility).
    JVal merge(const JVal& defaults, const JVal& user) const noexcept;
};

// ---------------------------------------------------------------------------
// SchemaMigrator
// ---------------------------------------------------------------------------
class SchemaMigrator {
  public:
    using MigrateFn = std::function<bool(JVal& doc)>;

    // Register a migration function from `from_version` to `from_version + 1`.
    void register_migration(int from_version, MigrateFn fn);

    // Migrate `doc` from its current schema_version to `target_version`.
    // Returns false if the version is ahead of `target_version` (future schema)
    // or if any migration function returns false.
    bool migrate(JVal& doc, int target_version) noexcept;

    static constexpr int kCurrentVersion = RuntimeConfig::kCurrentSchemaVersion;

  private:
    struct Entry {
        int from;
        MigrateFn fn;
    };
    std::vector<Entry> _migrations;
};

} // namespace config
} // namespace wildlife
