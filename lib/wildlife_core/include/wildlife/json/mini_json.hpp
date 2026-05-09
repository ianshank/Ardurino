#pragma once

// Wildlife Core — MiniJson
// Minimal JSON DOM: null, bool, int64, double, string, array, object.
// No exceptions; no Arduino dependencies; no streaming; no Unicode escapes.
// parse() returns std::nullopt on any error. JVal is copyable and movable.

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace wildlife {
namespace json {

enum class JType : uint8_t { Null, Bool, Int64, Double, Str, Array, Object };

class JVal {
public:
    // --- Constructors ---
    JVal() noexcept = default;
    explicit JVal(bool v)        noexcept : _type(JType::Bool),   _b(v)            {}
    explicit JVal(int64_t v)     noexcept : _type(JType::Int64),  _i(v)            {}
    explicit JVal(int v)         noexcept : _type(JType::Int64),  _i(v)            {}
    explicit JVal(double v)      noexcept : _type(JType::Double), _d(v)            {}
    explicit JVal(std::string v) noexcept : _type(JType::Str),    _s(std::move(v)) {}

    static JVal make_array()  noexcept { JVal v; v._type = JType::Array;  return v; }
    static JVal make_object() noexcept { JVal v; v._type = JType::Object; return v; }

    // --- Type ---
    JType type()    const noexcept { return _type; }
    bool  is_null() const noexcept { return _type == JType::Null; }

    // --- Read accessors (return default on type mismatch) ---
    bool             as_bool  (bool           def = false) const noexcept;
    int64_t          as_int   (int64_t        def = 0)     const noexcept;
    double           as_double(double         def = 0.0)   const noexcept;
    std::string_view as_str   (std::string_view def = {})  const noexcept;

    std::size_t size() const noexcept;
    bool contains(std::string_view key) const noexcept;

    // Object access — returns static null sentinel on miss / wrong type.
    const JVal& operator[](std::string_view key) const noexcept;
    // Array access — returns static null sentinel on OOB / wrong type.
    const JVal& operator[](std::size_t idx) const noexcept;

    // --- Mutation (for DefaultsMerger and SchemaMigrator) ---
    // Set or overwrite key in an Object JVal.
    void set(std::string key, JVal val);
    // Remove key from an Object JVal (no-op if missing).
    void remove(std::string_view key) noexcept;
    // Append to an Array JVal.
    void push_back(JVal val);

    // --- Raw iteration (use with care) ---
    using Pair = std::pair<std::string, JVal>;
    const std::vector<Pair>& as_object_ref() const noexcept;
    const std::vector<JVal>& as_array_ref()  const noexcept;

private:
    JType  _type{JType::Null};
    bool   _b{false};
    int64_t _i{0};
    double  _d{0.0};
    std::string   _s;
    std::vector<JVal> _arr;
    std::vector<Pair> _obj;

    static const JVal& null_sentinel() noexcept;
    static const std::vector<Pair>&    empty_obj() noexcept;
    static const std::vector<JVal>&    empty_arr() noexcept;
};

// Parse JSON from string_view. Returns nullopt on any syntax error.
std::optional<JVal> parse(std::string_view input) noexcept;

// Serialize JVal back to a compact JSON string.
std::string stringify(const JVal& v) noexcept;

} // namespace json
} // namespace wildlife
