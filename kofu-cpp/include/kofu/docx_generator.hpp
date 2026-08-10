#ifndef KOFU_DOCX_GENERATOR_HPP
#define KOFU_DOCX_GENERATOR_HPP

// =============================================================================
// Kofu C++ Backend — M08: DOCX Generator
// =============================================================================
// Creates .docx files (Office Open XML) natively using libzip + pugixml.
// Supports: from-scratch creation, .dotx template conversion, styled content
// (headings, paragraphs with bold/italic, tables), and automatic backups.
// =============================================================================

#include <string>
#include <vector>
#include <optional>
#include <filesystem>
#include <tuple>

namespace kofu {

// ---------------------------------------------------------------------------
// Color — RGB triplet
// ---------------------------------------------------------------------------
struct RGBColor {
    int r, g, b;

    /// Convert to 6-char hex string (e.g. "006699").
    std::string to_hex() const;
};

// ---------------------------------------------------------------------------
// Document style definitions
// ---------------------------------------------------------------------------
struct DocStyle {
    std::string heading_font = "Arial";
    std::string body_font    = "Calibri";
    RGBColor heading_color   = {0, 102, 204};
    RGBColor body_color      = {30, 30, 30};
};

// Get a named style ("professional" or "modern").
DocStyle get_doc_style(const std::string& style_name);

// ---------------------------------------------------------------------------
// Content blocks — what the AI generates
// ---------------------------------------------------------------------------
struct DocContent {
    std::string type;   // "heading", "document_title", "document_subtitle",
                        // "paragraph", "table"
    std::string text;
    int level = 1;      // For headings (0-9)
    std::vector<std::vector<std::string>> table_data; // For tables
};

// ---------------------------------------------------------------------------
// DocxGenerator
// ---------------------------------------------------------------------------
class DocxGenerator {
public:
    /// Set the directories for output and backup.
    DocxGenerator(const std::filesystem::path& output_dir,
                  const std::filesystem::path& backup_dir);

    /// Create a Word document from structured content data.
    /// If template_path is provided and exists, converts .dotx → .docx first.
    /// Returns {file_path, backup_path}.
    struct Result {
        std::filesystem::path file_path;
        std::filesystem::path backup_path;
    };

    Result create_document(
        const std::string& output_filename,
        const std::vector<DocContent>& content,
        const std::string& style_name = "professional",
        const std::optional<std::filesystem::path>& template_path = std::nullopt,
        bool save_backup = true);

    /// Convert a .dotx template to .docx by patching [Content_Types].xml.
    /// Returns path to the converted temp file, or nullopt on failure.
    static std::optional<std::filesystem::path> convert_template(
        const std::filesystem::path& template_path,
        const std::filesystem::path& output_dir);

private:
    /// Build a complete .docx ZIP archive from scratch with the given content.
    void build_docx_from_scratch(
        const std::filesystem::path& output_path,
        const std::vector<DocContent>& content,
        const DocStyle& style);

    /// Modify an existing .docx (from template) by adding content.
    void modify_existing_docx(
        const std::filesystem::path& source_docx,
        const std::filesystem::path& output_path,
        const std::vector<DocContent>& content,
        const DocStyle& style);

    /// Generate document.xml content with styled paragraphs.
    std::string generate_document_xml(
        const std::vector<DocContent>& content,
        const DocStyle& style);

    /// Parse a paragraph block for markdown formatting (**bold**, *italic*).
    /// Returns XML runs (<w:r> elements).
    std::string parse_paragraph_runs(
        const std::string& text,
        const DocStyle& style);

    /// Copy file to backup directory.
    std::filesystem::path save_backup(const std::filesystem::path& source);

    std::filesystem::path output_dir_;
    std::filesystem::path backup_dir_;
};

} // namespace kofu

#endif // KOFU_DOCX_GENERATOR_HPP
