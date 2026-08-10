// =============================================================================
// Kofu C++ Backend — M03: Input Sanitization
// =============================================================================

#include "kofu/sanitizer.hpp"

#include <regex>
#include <algorithm>
#include <cctype>
#include <filesystem>

namespace kofu {

// ---------------------------------------------------------------------------
// Sanitizer — dangerous patterns list
// ---------------------------------------------------------------------------
const std::vector<std::string>& Sanitizer::dangerous_patterns() {
    static const std::vector<std::string> patterns = {
        // XSS / script injection
        R"(<script\b[^<]*(?:(?!</script>)<[^<]*)*</script>)",
        R"(javascript:)",
        R"(vbscript:)",
        R"(onload=)",
        R"(onclick=)",
        R"(onerror=)",

        // Code execution
        R"(eval\s*\()",
        R"(exec\s*\()",
        R"(system\s*\()",
        R"(shell\s*\()",

        // SQL injection
        R"(drop\s+table)",
        R"(select\s+.*\s+from)",
        R"(insert\s+into)",
        R"(delete\s+from)",

        // Shell commands
        R"(rm\s+-rf)",
        R"(mkdir\s+)",
        R"(rmdir\s+)",
        R"(chmod\s+)",
        R"(sudo\s+)",

        // Dangerous HTML tags
        R"(<iframe)",
        R"(<object)",
        R"(<embed)",
        R"(<svg)",
        R"(<link)",
        R"(<meta)",
    };
    return patterns;
}

// ---------------------------------------------------------------------------
// strip_html — remove all HTML tags
// ---------------------------------------------------------------------------
std::string Sanitizer::strip_html(const std::string& text) {
    static const std::regex html_tag_re(R"(<[^>]+>)", std::regex::icase);
    return std::regex_replace(text, html_tag_re, "");
}

// ---------------------------------------------------------------------------
// strip_control_chars — remove 0x00-0x1F and 0x7F-0x9F
// ---------------------------------------------------------------------------
std::string Sanitizer::strip_control_chars(const std::string& text) {
    std::string result;
    result.reserve(text.size());
    for (unsigned char c : text) {
        if ((c >= 0x00 && c <= 0x1F && c != '\n' && c != '\r' && c != '\t') ||
            (c >= 0x7F && c <= 0x9F)) {
            continue; // Skip control characters (keep newline, CR, tab)
        }
        result.push_back(static_cast<char>(c));
    }
    return result;
}

// ---------------------------------------------------------------------------
// clean_text — full sanitization pipeline
// ---------------------------------------------------------------------------
std::string Sanitizer::clean_text(const std::string& text) {
    std::string cleaned = text;

    // Step 1: Remove dangerous patterns (case-insensitive)
    for (const auto& pattern : dangerous_patterns()) {
        try {
            std::regex re(pattern, std::regex::icase);
            cleaned = std::regex_replace(cleaned, re, "");
        } catch (const std::regex_error&) {
            // If a regex is malformed, skip it rather than crashing
        }
    }

    // Step 2: Strip HTML tags (keeping none — simpler than bleach allowlist
    // since we're generating documents, not rendering HTML)
    cleaned = strip_html(cleaned);

    // Step 3: Remove control characters
    cleaned = strip_control_chars(cleaned);

    // Step 4: Collapse whitespace
    static const std::regex multi_space_re(R"(\s+)");
    cleaned = std::regex_replace(cleaned, multi_space_re, " ");

    // Step 5: Trim leading and trailing whitespace
    auto start = cleaned.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    auto end = cleaned.find_last_not_of(" \t\r\n");
    return cleaned.substr(start, end - start + 1);
}

// ---------------------------------------------------------------------------
// is_safe_input
// ---------------------------------------------------------------------------
bool Sanitizer::is_safe_input(const std::string& text) {
    std::string cleaned = clean_text(text);
    return !cleaned.empty() && cleaned.size() <= 10000;
}

// ---------------------------------------------------------------------------
// sanitize_filename
// ---------------------------------------------------------------------------
std::string Sanitizer::sanitize_filename(const std::string& name) {
    // Remove dangerous filesystem characters
    static const std::regex dangerous_chars_re(R"([<>:"/\\|?*])");
    std::string cleaned = std::regex_replace(name, dangerous_chars_re, "");

    // Extract basename (remove any path components)
    std::filesystem::path p(cleaned);
    cleaned = p.filename().string();

    // Truncate to 255 characters
    if (cleaned.size() > 255) {
        cleaned = cleaned.substr(0, 255);
    }

    return cleaned;
}

// ---------------------------------------------------------------------------
// TypoCorrector — constructor, builds dictionaries
// ---------------------------------------------------------------------------
TypoCorrector::TypoCorrector() {
    // Word dictionary: correct_word -> [typos]
    dictionary_ = {
        {"powerpoint",    {"powerpoin", "powerpint", "powepoint", "powerpiont", "ppt"}},
        {"word",          {"wrod", "wor", "wrd", "wordd"}},
        {"documento",     {"documeto", "document", "documnto", "doc"}},
        {"presentacion",  {"presentacio", "presentacionn", "presntacion"}},
        {"plantilla",     {"plantila", "plantllla", "plantil", "template"}},
        {"crear",         {"crea", "crar", "hacer"}},
        {"investigar",    {"investiga", "investgar", "buscar"}},
        {"consejos",      {"consejo", "tips"}},
        {"ayuda",         {"aydua", "help"}},
    };

    // Build reverse lookup: typo -> correct_word
    for (const auto& [correct, typos] : dictionary_) {
        for (const auto& typo : typos) {
            reverse_dict_[typo] = correct;
        }
    }

    // Phrase dictionary: correct_phrase -> [variants]
    phrases_ = {
        {"crear documento de word",
            {"crear doc de word", "hacer documento de word", "crear word"}},
        {"crear presentacion de powerpoint",
            {"crear ppt", "hacer presentacion powerpoint", "crear powerpoint"}},
        {"consejos de powerpoint",
            {"tips de powerpoint", "consejos ppt", "ayuda powerpoint"}},
        {"consejos de word",
            {"tips de word", "consejos doc", "ayuda word"}},
        {"investigar sobre",
            {"buscar sobre", "investiga sobre", "busca sobre"}},
    };
}

// ---------------------------------------------------------------------------
// correct_word — single word lookup
// ---------------------------------------------------------------------------
std::string TypoCorrector::correct_word(const std::string& word) const {
    // Lowercase the word for lookup
    std::string lower = word;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    // Check reverse dictionary
    auto it = reverse_dict_.find(lower);
    if (it != reverse_dict_.end()) {
        return it->second;
    }

    // Check if it's already a correct word in our dictionary
    if (dictionary_.count(lower)) {
        return lower;
    }

    return word; // Return unchanged
}

// ---------------------------------------------------------------------------
// correct_text — full text correction (phrases first, then words)
// ---------------------------------------------------------------------------
std::string TypoCorrector::correct_text(const std::string& text) const {
    std::string result = text;

    // Step 1: Replace known phrase variants (case-insensitive)
    for (const auto& [correct_phrase, variants] : phrases_) {
        for (const auto& variant : variants) {
            // Case-insensitive search and replace
            try {
                std::regex variant_re(
                    std::regex_replace(variant,
                        std::regex(R"([-[\]{}()*+?.,\\^$|#\s])"), R"(\$&)"),
                    std::regex::icase);
                result = std::regex_replace(result, variant_re, correct_phrase);
            } catch (const std::regex_error&) {
                // Skip malformed patterns
            }
        }
    }

    // Step 2: Correct individual words
    static const std::regex word_re(R"(\b\w+\b)");
    std::string corrected;
    std::sregex_iterator it(result.begin(), result.end(), word_re);
    std::sregex_iterator end;

    size_t last_pos = 0;
    for (; it != end; ++it) {
        const auto& match = *it;
        // Append text before this word
        corrected.append(result, last_pos,
                         static_cast<size_t>(match.position()) - last_pos);
        // Append corrected word
        corrected.append(correct_word(match.str()));
        last_pos = static_cast<size_t>(match.position() + match.length());
    }
    // Append remaining text
    corrected.append(result, last_pos, std::string::npos);

    return corrected;
}

} // namespace kofu
