#ifndef KOFU_SANITIZER_HPP
#define KOFU_SANITIZER_HPP

// =============================================================================
// Kofu C++ Backend — M03: Input Sanitization
// =============================================================================
// Ports SanitizadorEntrada and TypoCorrector from sanitization.py.
// Dangerous pattern removal, HTML stripping, filename sanitization,
// and Spanish/English typo correction.
// =============================================================================

#include <string>
#include <vector>
#include <unordered_map>

namespace kofu {

// ---------------------------------------------------------------------------
// SanitizadorEntrada — input security sanitizer
// ---------------------------------------------------------------------------
class Sanitizer {
public:
    /// Clean text: remove dangerous patterns, strip HTML tags (keeping only
    /// safe formatting tags), remove control characters, collapse whitespace.
    static std::string clean_text(const std::string& text);

    /// Check if cleaned text is within safe bounds (0 < length <= 10000).
    static bool is_safe_input(const std::string& text);

    /// Sanitize a filename: remove dangerous characters (<>:"/\|?*),
    /// extract basename, and truncate to 255 characters.
    static std::string sanitize_filename(const std::string& name);

    /// Strip all HTML tags from text.
    static std::string strip_html(const std::string& text);

    /// Remove control characters (0x00-0x1F, 0x7F-0x9F) from text.
    static std::string strip_control_chars(const std::string& text);

private:
    /// List of dangerous regex patterns (XSS, injection, shell commands).
    static const std::vector<std::string>& dangerous_patterns();
};

// ---------------------------------------------------------------------------
// TypoCorrector — Spanish/English typo correction
// ---------------------------------------------------------------------------
class TypoCorrector {
public:
    TypoCorrector();

    /// Correct a single word using the typo dictionary.
    std::string correct_word(const std::string& word) const;

    /// Correct an entire text: first replace known phrase variants,
    /// then correct individual words.
    std::string correct_text(const std::string& text) const;

private:
    /// Map: correct_word -> [typo1, typo2, ...]
    std::unordered_map<std::string, std::vector<std::string>> dictionary_;

    /// Map: correct_phrase -> [variant1, variant2, ...]
    std::unordered_map<std::string, std::vector<std::string>> phrases_;

    /// Reverse lookup: typo -> correct_word (built from dictionary_)
    std::unordered_map<std::string, std::string> reverse_dict_;
};

} // namespace kofu

#endif // KOFU_SANITIZER_HPP
