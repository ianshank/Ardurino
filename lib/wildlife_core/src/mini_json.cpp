#include "wildlife/json/mini_json.hpp"

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>

namespace wildlife {
namespace json {

// ---------------------------------------------------------------------------
// JVal accessors
// ---------------------------------------------------------------------------

bool JVal::as_bool(bool def) const noexcept {
    return _type == JType::Bool ? _b : def;
}

int64_t JVal::as_int(int64_t def) const noexcept {
    if (_type == JType::Int64)  return _i;
    if (_type == JType::Double) return static_cast<int64_t>(_d);
    return def;
}

double JVal::as_double(double def) const noexcept {
    if (_type == JType::Double) return _d;
    if (_type == JType::Int64)  return static_cast<double>(_i);
    return def;
}

std::string_view JVal::as_str(std::string_view def) const noexcept {
    return _type == JType::Str ? std::string_view{_s} : def;
}

std::size_t JVal::size() const noexcept {
    if (_type == JType::Array)  return _arr.size();
    if (_type == JType::Object) return _obj.size();
    return 0;
}

bool JVal::contains(std::string_view key) const noexcept {
    if (_type != JType::Object) return false;
    for (const auto& [k, v] : _obj) {
        if (k == key) return true;
    }
    return false;
}

const JVal& JVal::null_sentinel() noexcept {
    static JVal s{};
    return s;
}

const std::vector<JVal::Pair>& JVal::empty_obj() noexcept {
    static std::vector<Pair> s{};
    return s;
}

const std::vector<JVal>& JVal::empty_arr() noexcept {
    static std::vector<JVal> s{};
    return s;
}

const JVal& JVal::operator[](std::string_view key) const noexcept {
    if (_type != JType::Object) return null_sentinel();
    for (const auto& [k, v] : _obj) {
        if (k == key) return v;
    }
    return null_sentinel();
}

const JVal& JVal::operator[](std::size_t idx) const noexcept {
    if (_type != JType::Array || idx >= _arr.size()) return null_sentinel();
    return _arr[idx];
}

void JVal::set(std::string key, JVal val) {
    if (_type != JType::Object) return;
    for (auto& [k, v] : _obj) {
        if (k == key) { v = std::move(val); return; }
    }
    _obj.emplace_back(std::move(key), std::move(val));
}

void JVal::remove(std::string_view key) noexcept {
    if (_type != JType::Object) return;
    for (auto it = _obj.begin(); it != _obj.end(); ++it) {
        if (it->first == key) { _obj.erase(it); return; }
    }
}

void JVal::push_back(JVal val) {
    if (_type != JType::Array) return;
    _arr.push_back(std::move(val));
}

const std::vector<JVal::Pair>& JVal::as_object_ref() const noexcept {
    return _type == JType::Object ? _obj : empty_obj();
}

const std::vector<JVal>& JVal::as_array_ref() const noexcept {
    return _type == JType::Array ? _arr : empty_arr();
}

// ---------------------------------------------------------------------------
// Parser — recursive descent
// ---------------------------------------------------------------------------

namespace {

struct Parser {
    const char* cur;
    const char* end;

    bool at_end() const noexcept { return cur >= end; }

    void skip_ws() noexcept {
        while (cur < end && (*cur == ' ' || *cur == '\t' ||
                             *cur == '\n' || *cur == '\r')) {
            ++cur;
        }
    }

    bool peek(char c) const noexcept { return !at_end() && *cur == c; }

    bool consume(char c) noexcept {
        if (!peek(c)) return false;
        ++cur;
        return true;
    }

    // Parse a 4-digit hex escape (after \u).  We only accept ASCII codepoints
    // (\u0000–\u007F) and map them; anything above returns false so callers
    // can reject the string rather than produce a mojibake.
    bool parse_hex4(char& out) noexcept {
        if (cur + 4 > end) return false;
        uint32_t val = 0;
        for (int i = 0; i < 4; ++i) {
            char c = cur[i];
            uint32_t nibble;
            if      (c >= '0' && c <= '9') nibble = static_cast<uint32_t>(c - '0');
            else if (c >= 'a' && c <= 'f') nibble = static_cast<uint32_t>(c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') nibble = static_cast<uint32_t>(c - 'A' + 10);
            else return false;
            val = (val << 4) | nibble;
        }
        if (val > 127) return false;  // only ASCII for now
        out = static_cast<char>(val);
        cur += 4;
        return true;
    }

    std::optional<std::string> parse_string() noexcept {
        if (!consume('"')) return std::nullopt;
        std::string s;
        while (!at_end() && *cur != '"') {
            if (*cur == '\\') {
                ++cur;
                if (at_end()) return std::nullopt;
                char esc = *cur++;
                switch (esc) {
                    case '"':  s += '"';  break;
                    case '\\': s += '\\'; break;
                    case '/':  s += '/';  break;
                    case 'n':  s += '\n'; break;
                    case 'r':  s += '\r'; break;
                    case 't':  s += '\t'; break;
                    case 'f':  s += '\f'; break;
                    case 'b':  s += '\b'; break;
                    case 'u': {
                        char c = 0;
                        if (!parse_hex4(c)) return std::nullopt;
                        s += c;
                        break;
                    }
                    default: return std::nullopt;
                }
            } else {
                s += *cur++;
            }
        }
        if (!consume('"')) return std::nullopt;
        return s;
    }

    std::optional<JVal> parse_number() noexcept {
        const char* start = cur;
        bool negative = false;
        if (peek('-')) { negative = true; ++cur; }
        if (at_end()) return std::nullopt;

        bool is_double = false;
        while (!at_end() && *cur >= '0' && *cur <= '9') ++cur;
        if (!at_end() && *cur == '.') { is_double = true; ++cur; }
        while (!at_end() && *cur >= '0' && *cur <= '9') ++cur;
        if (!at_end() && (*cur == 'e' || *cur == 'E')) {
            is_double = true;
            ++cur;
            if (!at_end() && (*cur == '+' || *cur == '-')) ++cur;
            while (!at_end() && *cur >= '0' && *cur <= '9') ++cur;
        }

        if (cur == start || (cur == start + 1 && negative)) return std::nullopt;

        // Null-terminate a local copy for strtod/strtoll.
        const std::size_t len = static_cast<std::size_t>(cur - start);
        if (len >= 64) return std::nullopt;  // unreasonably large
        char buf[64];
        std::memcpy(buf, start, len);
        buf[len] = '\0';

        if (is_double) {
            char* endp = nullptr;
            double d = std::strtod(buf, &endp);
            if (endp != buf + len) return std::nullopt;
            return JVal{d};
        } else {
            char* endp = nullptr;
            int64_t i = std::strtoll(buf, &endp, 10);
            if (endp != buf + len) return std::nullopt;
            return JVal{i};
        }
    }

    std::optional<JVal> parse_literal() noexcept {
        if (cur + 4 <= end && std::memcmp(cur, "true", 4) == 0) {
            cur += 4; return JVal{true};
        }
        if (cur + 5 <= end && std::memcmp(cur, "false", 5) == 0) {
            cur += 5; return JVal{false};
        }
        if (cur + 4 <= end && std::memcmp(cur, "null", 4) == 0) {
            cur += 4; return JVal{};
        }
        return std::nullopt;
    }

    std::optional<JVal> parse_array() noexcept {
        if (!consume('[')) return std::nullopt;
        JVal arr = JVal::make_array();
        skip_ws();
        if (consume(']')) return arr;  // empty array
        while (true) {
            skip_ws();
            auto elem = parse_value();
            if (!elem) return std::nullopt;
            arr.push_back(std::move(*elem));
            skip_ws();
            if (consume(']')) return arr;
            if (!consume(',')) return std::nullopt;
        }
    }

    std::optional<JVal> parse_object() noexcept {
        if (!consume('{')) return std::nullopt;
        JVal obj = JVal::make_object();
        skip_ws();
        if (consume('}')) return obj;  // empty object
        while (true) {
            skip_ws();
            auto key = parse_string();
            if (!key) return std::nullopt;
            skip_ws();
            if (!consume(':')) return std::nullopt;
            skip_ws();
            auto val = parse_value();
            if (!val) return std::nullopt;
            obj.set(std::move(*key), std::move(*val));
            skip_ws();
            if (consume('}')) return obj;
            if (!consume(',')) return std::nullopt;
        }
    }

    std::optional<JVal> parse_value() noexcept {
        skip_ws();
        if (at_end()) return std::nullopt;
        char c = *cur;
        if (c == '"') { auto s = parse_string(); if (!s) return std::nullopt; return JVal{std::move(*s)}; }
        if (c == '{') return parse_object();
        if (c == '[') return parse_array();
        if (c == 't' || c == 'f' || c == 'n') return parse_literal();
        if (c == '-' || (c >= '0' && c <= '9')) return parse_number();
        return std::nullopt;
    }
};

// ---------------------------------------------------------------------------
// Stringify helper
// ---------------------------------------------------------------------------

void append_escaped(std::string& out, std::string_view s) noexcept {
    out += '"';
    for (unsigned char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:
                if (c < 0x20) {
                    // control character — escape as \uXXXX
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += static_cast<char>(c);
                }
        }
    }
    out += '"';
}

void do_stringify(std::string& out, const JVal& v) noexcept {
    switch (v.type()) {
        case JType::Null:   out += "null"; break;
        case JType::Bool:   out += v.as_bool() ? "true" : "false"; break;
        case JType::Int64: {
            char buf[24];
            std::snprintf(buf, sizeof(buf), "%lld", static_cast<long long>(v.as_int()));
            out += buf;
            break;
        }
        case JType::Double: {
            char buf[32];
            std::snprintf(buf, sizeof(buf), "%.17g", v.as_double());
            out += buf;
            break;
        }
        case JType::Str:
            append_escaped(out, v.as_str());
            break;
        case JType::Array: {
            out += '[';
            bool first = true;
            for (std::size_t i = 0; i < v.size(); ++i) {
                if (!first) out += ',';
                do_stringify(out, v[i]);
                first = false;
            }
            out += ']';
            break;
        }
        case JType::Object: {
            out += '{';
            bool first = true;
            for (const auto& [k, val] : v.as_object_ref()) {
                if (!first) out += ',';
                append_escaped(out, k);
                out += ':';
                do_stringify(out, val);
                first = false;
            }
            out += '}';
            break;
        }
    }
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

std::optional<JVal> parse(std::string_view input) noexcept {
    if (input.empty()) return std::nullopt;
    Parser p{input.data(), input.data() + input.size()};
    auto result = p.parse_value();
    if (!result) return std::nullopt;
    p.skip_ws();
    if (!p.at_end()) return std::nullopt;  // trailing garbage
    return result;
}

std::string stringify(const JVal& v) noexcept {
    std::string out;
    out.reserve(128);
    do_stringify(out, v);
    return out;
}

} // namespace json
} // namespace wildlife
