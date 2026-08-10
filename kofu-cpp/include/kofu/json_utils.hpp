#ifndef KOFU_JSON_UTILS_HPP
#define KOFU_JSON_UTILS_HPP

// =============================================================================
// Kofu C++ Backend — M02: JSON Utilities
// =============================================================================
// Safe JSON parsing wrappers around nlohmann/json.
// Handles: markdown code block extraction, graceful error recovery,
// type-safe accessors with defaults.
// =============================================================================

#include <nlohmann/json.hpp>
#include <string>
#include <optional>
#include <vector>

namespace kofu {

using json = nlohmann::json;

// ---------------------------------------------------------------------------
// Safe parsing — never throws, returns std::nullopt on failure
// ---------------------------------------------------------------------------

/// Parse a JSON string. Returns std::nullopt if parsing fails.
std::optional<json> safe_parse(const std::string& raw);

/// Extract a JSON object from text that may contain markdown code blocks.
/// Tries in order:
///   1. Direct JSON parse
///   2. Extract from ```json ... ``` blocks
///   3. Extract from ``` ... ``` blocks
///   4. Find first { ... } or [ ... ] substring
/// Returns std::nullopt if all strategies fail.
std::optional<json> extract_json(const std::string& text);

// ---------------------------------------------------------------------------
// Type-safe accessors with defaults — never throw
// ---------------------------------------------------------------------------

/// Get a string value from a JSON object, or return default_val.
std::string json_string(const json& obj, const std::string& key,
                        const std::string& default_val = "");

/// Get an int value from a JSON object, or return default_val.
int json_int(const json& obj, const std::string& key,
             int default_val = 0);

/// Get a bool value from a JSON object, or return default_val.
bool json_bool(const json& obj, const std::string& key,
               bool default_val = false);

/// Get an array of strings from a JSON object, or return empty vector.
std::vector<std::string> json_string_array(const json& obj,
                                           const std::string& key);

// ---------------------------------------------------------------------------
// Serialization helpers
// ---------------------------------------------------------------------------

/// Serialize a JSON object to a compact string (no pretty printing).
std::string to_compact_string(const json& obj);

/// Serialize a JSON object to a pretty-printed string (2-space indent).
std::string to_pretty_string(const json& obj);

/// Build a standard API error response JSON: {"error": message, "code": code}
json make_error_response(const std::string& message, int code);

/// Build a standard API success response JSON: {"status": "ok", ...extra_fields}
json make_success_response(const json& extra_fields = json::object());

} // namespace kofu

#endif // KOFU_JSON_UTILS_HPP
