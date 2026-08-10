// =============================================================================
// Kofu C++ Backend — M02: JSON Utilities
// =============================================================================

#include "kofu/json_utils.hpp"

#include <regex>
#include <iostream>

namespace kofu {

// ---------------------------------------------------------------------------
// safe_parse
// ---------------------------------------------------------------------------
std::optional<json> safe_parse(const std::string& raw) {
    try {
        return json::parse(raw);
    } catch (const json::parse_error&) {
        return std::nullopt;
    }
}

// ---------------------------------------------------------------------------
// extract_json — multi-strategy JSON extraction from LLM output
// ---------------------------------------------------------------------------
std::optional<json> extract_json(const std::string& text) {
    // Strategy 1: Direct parse
    if (auto result = safe_parse(text); result.has_value()) {
        return result;
    }

    // Strategy 2: Extract from ```json ... ``` code blocks
    {
        static const std::regex json_block_re(
            R"(```json\s*\n([\s\S]*?)```)",
            std::regex::ECMAScript);

        std::smatch match;
        if (std::regex_search(text, match, json_block_re) && match.size() > 1) {
            if (auto result = safe_parse(match[1].str()); result.has_value()) {
                return result;
            }
        }
    }

    // Strategy 3: Extract from generic ``` ... ``` code blocks
    {
        static const std::regex code_block_re(
            R"(```\s*\n([\s\S]*?)```)",
            std::regex::ECMAScript);

        std::smatch match;
        if (std::regex_search(text, match, code_block_re) && match.size() > 1) {
            if (auto result = safe_parse(match[1].str()); result.has_value()) {
                return result;
            }
        }
    }

    // Strategy 4: Find first balanced { ... } or [ ... ]
    {
        auto find_balanced = [](const std::string& s, char open,
                                char close) -> std::optional<std::string> {
            auto start_pos = s.find(open);
            if (start_pos == std::string::npos) return std::nullopt;

            int depth = 0;
            bool in_string = false;
            bool escape_next = false;

            for (size_t i = start_pos; i < s.size(); ++i) {
                char c = s[i];

                if (escape_next) {
                    escape_next = false;
                    continue;
                }

                if (c == '\\' && in_string) {
                    escape_next = true;
                    continue;
                }

                if (c == '"') {
                    in_string = !in_string;
                    continue;
                }

                if (!in_string) {
                    if (c == open) ++depth;
                    if (c == close) --depth;
                    if (depth == 0) {
                        return s.substr(start_pos, i - start_pos + 1);
                    }
                }
            }
            return std::nullopt;
        };

        // Try object first, then array
        if (auto obj_str = find_balanced(text, '{', '}'); obj_str.has_value()) {
            if (auto result = safe_parse(*obj_str); result.has_value()) {
                return result;
            }
        }
        if (auto arr_str = find_balanced(text, '[', ']'); arr_str.has_value()) {
            if (auto result = safe_parse(*arr_str); result.has_value()) {
                return result;
            }
        }
    }

    return std::nullopt;
}

// ---------------------------------------------------------------------------
// Type-safe accessors
// ---------------------------------------------------------------------------
std::string json_string(const json& obj, const std::string& key,
                        const std::string& default_val) {
    try {
        if (obj.contains(key) && obj[key].is_string()) {
            return obj[key].get<std::string>();
        }
    } catch (...) {}
    return default_val;
}

int json_int(const json& obj, const std::string& key, int default_val) {
    try {
        if (obj.contains(key) && obj[key].is_number()) {
            return obj[key].get<int>();
        }
    } catch (...) {}
    return default_val;
}

bool json_bool(const json& obj, const std::string& key, bool default_val) {
    try {
        if (obj.contains(key) && obj[key].is_boolean()) {
            return obj[key].get<bool>();
        }
    } catch (...) {}
    return default_val;
}

std::vector<std::string> json_string_array(const json& obj,
                                           const std::string& key) {
    std::vector<std::string> result;
    try {
        if (obj.contains(key) && obj[key].is_array()) {
            for (const auto& item : obj[key]) {
                if (item.is_string()) {
                    result.push_back(item.get<std::string>());
                } else {
                    // Convert non-string items to string representation
                    result.push_back(item.dump());
                }
            }
        }
    } catch (...) {}
    return result;
}

// ---------------------------------------------------------------------------
// Serialization helpers
// ---------------------------------------------------------------------------
std::string to_compact_string(const json& obj) {
    return obj.dump(-1);
}

std::string to_pretty_string(const json& obj) {
    return obj.dump(2);
}

json make_error_response(const std::string& message, int code) {
    return json{
        {"error", message},
        {"code", code},
        {"status", "error"}
    };
}

json make_success_response(const json& extra_fields) {
    json response = {{"status", "ok"}};
    if (extra_fields.is_object()) {
        response.merge_patch(extra_fields);
    }
    return response;
}

} // namespace kofu
