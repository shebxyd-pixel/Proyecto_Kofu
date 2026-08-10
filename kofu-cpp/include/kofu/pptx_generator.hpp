#ifndef KOFU_PPTX_GENERATOR_HPP
#define KOFU_PPTX_GENERATOR_HPP

// =============================================================================
// Kofu C++ Backend — M09: PPTX Generator
// =============================================================================
// Creates .pptx files (Office Open XML Presentation) natively using libzip.
// Supports: from-scratch creation, .potx template conversion, themed slides,
// title+content layouts, and automatic backups.
// =============================================================================

#include "kofu/docx_generator.hpp"  // For RGBColor

#include <string>
#include <vector>
#include <optional>
#include <filesystem>

namespace kofu {

// ---------------------------------------------------------------------------
// Presentation theme — colors for slides
// ---------------------------------------------------------------------------
struct PptTheme {
    std::string name;
    RGBColor background;
    RGBColor accent;
    RGBColor text;
};

/// Get a named theme ("professional", "modern", or "vibrant").
PptTheme get_ppt_theme(const std::string& theme_name);

// ---------------------------------------------------------------------------
// Slide data — what the AI generates per slide
// ---------------------------------------------------------------------------
struct SlideData {
    std::string title;
    std::string text;           // Body content
    std::string subtitle;       // Optional subtitle (usually slide 0 only)
    int layout = 1;             // 0 = title slide, 1 = title+content, etc.
    bool apply_background = true;
    std::vector<std::string> bullet_points; // Alternative to text
};

// ---------------------------------------------------------------------------
// PptxGenerator
// ---------------------------------------------------------------------------
class PptxGenerator {
public:
    PptxGenerator(const std::filesystem::path& output_dir,
                  const std::filesystem::path& backup_dir);

    struct Result {
        std::filesystem::path file_path;
        std::filesystem::path backup_path;
    };

    /// Create a PowerPoint from structured slide data.
    Result create_presentation(
        const std::string& output_filename,
        const std::vector<SlideData>& slides,
        const std::string& theme_name = "professional",
        const std::optional<std::filesystem::path>& template_path = std::nullopt,
        bool save_backup = true);

    /// Convert a .potx template to .pptx by patching [Content_Types].xml.
    static std::optional<std::filesystem::path> convert_template(
        const std::filesystem::path& template_path,
        const std::filesystem::path& output_dir);

private:
    /// Build a complete .pptx from scratch.
    void build_pptx_from_scratch(
        const std::filesystem::path& output_path,
        const std::vector<SlideData>& slides,
        const PptTheme& theme);

    /// Modify a template-based .pptx by adding slides.
    void modify_existing_pptx(
        const std::filesystem::path& source_pptx,
        const std::filesystem::path& output_path,
        const std::vector<SlideData>& slides,
        const PptTheme& theme);

    /// Generate a single slide XML.
    std::string generate_slide_xml(
        const SlideData& slide,
        const PptTheme& theme,
        int slide_index);

    std::filesystem::path save_backup(const std::filesystem::path& source);

    std::filesystem::path output_dir_;
    std::filesystem::path backup_dir_;
};

} // namespace kofu

#endif // KOFU_PPTX_GENERATOR_HPP
