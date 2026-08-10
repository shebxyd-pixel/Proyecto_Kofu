// =============================================================================
// Kofu C++ Backend — M08: DOCX Generator
// =============================================================================
// Builds .docx files (Office Open XML) natively using libzip + pugixml.
// A .docx is a ZIP archive containing XML files that describe the document.
// =============================================================================

#include "kofu/docx_generator.hpp"

#include <zip.h>
#include <pugixml.hpp>

#include <fstream>
#include <sstream>
#include <regex>
#include <iostream>
#include <algorithm>
#include <cstring>

namespace kofu {

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// RGBColor::to_hex
// ---------------------------------------------------------------------------
std::string RGBColor::to_hex() const {
    char buf[7];
    std::snprintf(buf, sizeof(buf), "%02X%02X%02X", r, g, b);
    return std::string(buf);
}

// ---------------------------------------------------------------------------
// get_doc_style
// ---------------------------------------------------------------------------
DocStyle get_doc_style(const std::string& style_name) {
    if (style_name == "modern") {
        return {"Segoe UI", "Segoe UI", {0, 200, 150}, {50, 50, 50}};
    }
    // Default: professional
    return {"Arial", "Calibri", {0, 102, 204}, {30, 30, 30}};
}

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
DocxGenerator::DocxGenerator(const fs::path& output_dir,
                             const fs::path& backup_dir)
    : output_dir_(output_dir)
    , backup_dir_(backup_dir)
{
    std::error_code ec;
    fs::create_directories(output_dir_, ec);
    fs::create_directories(backup_dir_, ec);
}

// ---------------------------------------------------------------------------
// Helper: add a string buffer as a file entry inside a ZIP archive
// ---------------------------------------------------------------------------
namespace {

bool zip_add_buffer(zip_t* archive, const char* entry_name,
                    const std::string& content) {
    zip_source_t* source = zip_source_buffer(
        archive, content.data(), content.size(), 0);
    if (!source) return false;

    zip_int64_t idx = zip_file_add(
        archive, entry_name, source, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8);
    if (idx < 0) {
        zip_source_free(source);
        return false;
    }
    return true;
}

/// Half-point size to OpenXML size value (1 pt = 2 half-points).
std::string pt_to_hps(int pt) {
    return std::to_string(pt * 2);
}

/// Point size to EMU (English Metric Units) for spacing.
/// 1 pt = 12700 EMU.
std::string pt_to_emu(int pt) {
    return std::to_string(pt * 12700);
}

/// Escape XML special characters in text content.
std::string xml_escape(const std::string& text) {
    std::string result;
    result.reserve(text.size());
    for (char c : text) {
        switch (c) {
            case '&':  result += "&amp;";  break;
            case '<':  result += "&lt;";   break;
            case '>':  result += "&gt;";   break;
            case '"':  result += "&quot;"; break;
            case '\'': result += "&apos;"; break;
            default:   result += c;        break;
        }
    }
    return result;
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// generate_document_xml — the core document content
// ---------------------------------------------------------------------------
std::string DocxGenerator::generate_document_xml(
    const std::vector<DocContent>& content,
    const DocStyle& style)
{
    std::ostringstream xml;

    xml << R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
        << R"(<w:document xmlns:wpc="http://schemas.microsoft.com/office/word/2010/wordprocessingCanvas")"
        << R"( xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006")"
        << R"( xmlns:o="urn:schemas-microsoft-com:office:office")"
        << R"( xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships")"
        << R"( xmlns:m="http://schemas.openxmlformats.org/officeDocument/2006/math")"
        << R"( xmlns:v="urn:schemas-microsoft-com:vml")"
        << R"( xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing")"
        << R"( xmlns:w10="urn:schemas-microsoft-com:office:word")"
        << R"( xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main")"
        << R"( xmlns:w14="http://schemas.microsoft.com/office/word/2010/wordml")"
        << R"( xmlns:wpg="http://schemas.microsoft.com/office/word/2010/wordprocessingGroup")"
        << R"( xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape")"
        << R"( mc:Ignorable="w14 wp14">)"
        << R"(<w:body>)";

    for (const auto& item : content) {
        if (item.type == "document_title" || item.type == "heading" ||
            item.type == "document_subtitle") {
            // Determine heading level and font size
            int level = (item.type == "document_title") ? 0 : item.level;
            int font_size = 24;
            if (level == 0) font_size = 28;
            else if (level == 1) font_size = 24;
            else if (level == 2) font_size = 18;
            else font_size = 14;

            std::string heading_style = "Heading" + std::to_string(
                std::max(1, std::min(level, 9)));
            if (item.type == "document_title") heading_style = "Title";
            if (item.type == "document_subtitle") heading_style = "Subtitle";

            // Clean markdown artifacts from title text
            std::string clean_text = item.text;
            clean_text = std::regex_replace(clean_text, std::regex(R"(\*\*)"), "");
            clean_text = std::regex_replace(clean_text, std::regex(R"(\*)"), "");
            clean_text = std::regex_replace(clean_text, std::regex(R"(^#+\s*)"), "");
            // Trim
            auto start = clean_text.find_first_not_of(" \t\r\n");
            if (start != std::string::npos) {
                auto end = clean_text.find_last_not_of(" \t\r\n");
                clean_text = clean_text.substr(start, end - start + 1);
            }

            xml << R"(<w:p>)"
                << R"(<w:pPr><w:pStyle w:val=")" << heading_style << R"("/></w:pPr>)"
                << R"(<w:r>)"
                << R"(<w:rPr>)"
                << R"(<w:rFonts w:ascii=")" << xml_escape(style.heading_font)
                << R"(" w:hAnsi=")" << xml_escape(style.heading_font) << R"("/>)"
                << R"(<w:b/>)"
                << R"(<w:color w:val=")" << style.heading_color.to_hex() << R"("/>)"
                << R"(<w:sz w:val=")" << pt_to_hps(font_size) << R"("/>)"
                << R"(</w:rPr>)"
                << R"(<w:t xml:space="preserve">)" << xml_escape(clean_text) << R"(</w:t>)"
                << R"(</w:r>)"
                << R"(</w:p>)";

        } else if (item.type == "paragraph") {
            // Split into blocks by newline
            std::istringstream stream(item.text);
            std::string block;
            while (std::getline(stream, block)) {
                // Trim block
                auto start = block.find_first_not_of(" \t\r");
                if (start == std::string::npos) continue;
                auto end = block.find_last_not_of(" \t\r");
                block = block.substr(start, end - start + 1);
                if (block.empty()) continue;

                // Check if block is a heading (starts with #)
                if (block[0] == '#') {
                    int level = 0;
                    while (level < static_cast<int>(block.size()) &&
                           block[static_cast<size_t>(level)] == '#') {
                        level++;
                    }
                    level = std::min(level, 9);
                    std::string heading_text = block.substr(
                        static_cast<size_t>(level));
                    auto hs = heading_text.find_first_not_of(" \t");
                    if (hs != std::string::npos)
                        heading_text = heading_text.substr(hs);

                    std::string heading_style_name = "Heading" +
                        std::to_string(std::max(1, level));

                    xml << R"(<w:p>)"
                        << R"(<w:pPr><w:pStyle w:val=")" << heading_style_name
                        << R"("/></w:pPr>)"
                        << R"(<w:r>)"
                        << R"(<w:rPr>)"
                        << R"(<w:rFonts w:ascii=")" << xml_escape(style.heading_font)
                        << R"(" w:hAnsi=")" << xml_escape(style.heading_font) << R"("/>)"
                        << R"(<w:color w:val=")" << style.heading_color.to_hex() << R"("/>)"
                        << R"(</w:rPr>)"
                        << R"(<w:t xml:space="preserve">)"
                        << xml_escape(heading_text) << R"(</w:t>)"
                        << R"(</w:r>)"
                        << R"(</w:p>)";
                    continue;
                }

                // Regular paragraph with markdown formatting
                xml << R"(<w:p>)"
                    << R"(<w:pPr><w:jc w:val="both"/></w:pPr>)"
                    << parse_paragraph_runs(block, style)
                    << R"(</w:p>)";
            }

        } else if (item.type == "table" && !item.table_data.empty()) {
            xml << R"(<w:tbl>)"
                << R"(<w:tblPr>)"
                << R"(<w:tblStyle w:val="TableGrid"/>)"
                << R"(<w:tblW w:w="5000" w:type="pct"/>)"
                << R"(<w:tblBorders>)"
                << R"(<w:top w:val="single" w:sz="4" w:space="0" w:color="auto"/>)"
                << R"(<w:left w:val="single" w:sz="4" w:space="0" w:color="auto"/>)"
                << R"(<w:bottom w:val="single" w:sz="4" w:space="0" w:color="auto"/>)"
                << R"(<w:right w:val="single" w:sz="4" w:space="0" w:color="auto"/>)"
                << R"(<w:insideH w:val="single" w:sz="4" w:space="0" w:color="auto"/>)"
                << R"(<w:insideV w:val="single" w:sz="4" w:space="0" w:color="auto"/>)"
                << R"(</w:tblBorders>)"
                << R"(</w:tblPr>)";

            for (size_t row_idx = 0; row_idx < item.table_data.size(); ++row_idx) {
                xml << R"(<w:tr>)";
                for (const auto& cell_text : item.table_data[row_idx]) {
                    xml << R"(<w:tc>)"
                        << R"(<w:p>)"
                        << R"(<w:r>)"
                        << R"(<w:rPr>)"
                        << R"(<w:rFonts w:ascii=")" << xml_escape(style.body_font)
                        << R"(" w:hAnsi=")" << xml_escape(style.body_font) << R"("/>)";
                    if (row_idx == 0) {
                        xml << R"(<w:b/>)"
                            << R"(<w:color w:val=")"
                            << style.heading_color.to_hex() << R"("/>)";
                    }
                    xml << R"(</w:rPr>)"
                        << R"(<w:t xml:space="preserve">)" << xml_escape(cell_text)
                        << R"(</w:t>)"
                        << R"(</w:r>)"
                        << R"(</w:p>)"
                        << R"(</w:tc>)";
                }
                xml << R"(</w:tr>)";
            }
            xml << R"(</w:tbl>)";
        }
    }

    // Section properties (A4, portrait, margins)
    xml << R"(<w:sectPr>)"
        << R"(<w:pgSz w:w="11906" w:h="16838"/>)"  // A4
        << R"(<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440")"
        << R"( w:header="720" w:footer="720" w:gutter="0"/>)"
        << R"(</w:sectPr>)";

    xml << R"(</w:body>)"
        << R"(</w:document>)";

    return xml.str();
}

// ---------------------------------------------------------------------------
// parse_paragraph_runs — split text on **bold** and *italic* markers
// ---------------------------------------------------------------------------
std::string DocxGenerator::parse_paragraph_runs(
    const std::string& text,
    const DocStyle& style)
{
    std::ostringstream xml;

    // Split on bold (**...**) and italic (*...*) patterns
    static const std::regex md_re(R"((\*\*.*?\*\*|\*.*?\*))");
    std::sregex_token_iterator it(text.begin(), text.end(), md_re, {-1, 0});
    std::sregex_token_iterator end;

    for (; it != end; ++it) {
        std::string part = it->str();
        if (part.empty()) continue;

        bool is_bold = (part.size() >= 4 &&
                        part.substr(0, 2) == "**" &&
                        part.substr(part.size() - 2) == "**");
        bool is_italic = !is_bold && (part.size() >= 2 &&
                         part[0] == '*' &&
                         part[part.size() - 1] == '*');

        std::string run_text = part;
        if (is_bold) run_text = part.substr(2, part.size() - 4);
        else if (is_italic) run_text = part.substr(1, part.size() - 2);

        xml << R"(<w:r>)"
            << R"(<w:rPr>)"
            << R"(<w:rFonts w:ascii=")" << xml_escape(style.body_font)
            << R"(" w:hAnsi=")" << xml_escape(style.body_font) << R"("/>)"
            << R"(<w:sz w:val=")" << pt_to_hps(11) << R"("/>)"
            << R"(<w:color w:val=")" << style.body_color.to_hex() << R"("/>)";
        if (is_bold) xml << R"(<w:b/>)";
        if (is_italic) xml << R"(<w:i/>)";
        xml << R"(</w:rPr>)"
            << R"(<w:t xml:space="preserve">)" << xml_escape(run_text) << R"(</w:t>)"
            << R"(</w:r>)";
    }

    return xml.str();
}

// ---------------------------------------------------------------------------
// build_docx_from_scratch — creates a complete .docx ZIP
// ---------------------------------------------------------------------------
void DocxGenerator::build_docx_from_scratch(
    const fs::path& output_path,
    const std::vector<DocContent>& content,
    const DocStyle& style)
{
    int err = 0;
    zip_t* archive = zip_open(output_path.string().c_str(),
                               ZIP_CREATE | ZIP_TRUNCATE, &err);
    if (!archive) {
        throw std::runtime_error("Cannot create ZIP archive: " +
                                 output_path.string());
    }

    // [Content_Types].xml
    std::string content_types =
        R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
        R"(<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">)"
        R"(<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>)"
        R"(<Default Extension="xml" ContentType="application/xml"/>)"
        R"(<Override PartName="/word/document.xml")"
        R"( ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>)"
        R"(<Override PartName="/word/styles.xml")"
        R"( ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>)"
        R"(<Override PartName="/word/settings.xml")"
        R"( ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>)"
        R"(<Override PartName="/word/fontTable.xml")"
        R"( ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.fontTable+xml"/>)"
        R"(<Override PartName="/docProps/core.xml")"
        R"( ContentType="application/vnd.openxmlformats-package.core-properties+xml"/>)"
        R"(<Override PartName="/docProps/app.xml")"
        R"( ContentType="application/vnd.openxmlformats-officedocument.extended-properties+xml"/>)"
        R"(</Types>)";

    // _rels/.rels
    std::string root_rels =
        R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
        R"(<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">)"
        R"(<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument")"
        R"( Target="word/document.xml"/>)"
        R"(<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties")"
        R"( Target="docProps/core.xml"/>)"
        R"(<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties")"
        R"( Target="docProps/app.xml"/>)"
        R"(</Relationships>)";

    // word/_rels/document.xml.rels
    std::string doc_rels =
        R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
        R"(<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">)"
        R"(<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles")"
        R"( Target="styles.xml"/>)"
        R"(<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings")"
        R"( Target="settings.xml"/>)"
        R"(<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/fontTable")"
        R"( Target="fontTable.xml"/>)"
        R"(</Relationships>)";

    // word/styles.xml — heading and body styles
    std::string styles_xml =
        R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
        R"(<w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">)"
        R"(<w:style w:type="paragraph" w:default="1" w:styleId="Normal">)"
        R"(<w:name w:val="Normal"/>)"
        R"(<w:rPr><w:rFonts w:ascii=")" + xml_escape(style.body_font) +
        R"(" w:hAnsi=")" + xml_escape(style.body_font) + R"("/>)"
        R"(<w:sz w:val=")" + pt_to_hps(11) + R"("/>)"
        R"(</w:rPr></w:style>)";

    // Generate heading styles 1-9
    for (int i = 1; i <= 9; ++i) {
        int size = (i == 1) ? 24 : (i == 2) ? 18 : 14;
        styles_xml +=
            R"(<w:style w:type="paragraph" w:styleId="Heading)" +
            std::to_string(i) + R"(">)"
            R"(<w:name w:val="heading )" + std::to_string(i) + R"("/>)"
            R"(<w:basedOn w:val="Normal"/>)"
            R"(<w:next w:val="Normal"/>)"
            R"(<w:pPr><w:outlineLvl w:val=")" + std::to_string(i - 1) + R"("/>)"
            R"(<w:spacing w:before="240" w:after="120"/>)"
            R"(</w:pPr>)"
            R"(<w:rPr><w:rFonts w:ascii=")" + xml_escape(style.heading_font) +
            R"(" w:hAnsi=")" + xml_escape(style.heading_font) + R"("/>)"
            R"(<w:b/><w:sz w:val=")" + pt_to_hps(size) + R"("/>)"
            R"(<w:color w:val=")" + style.heading_color.to_hex() + R"("/>)"
            R"(</w:rPr></w:style>)";
    }

    // Title and Subtitle styles
    styles_xml +=
        R"(<w:style w:type="paragraph" w:styleId="Title">)"
        R"(<w:name w:val="Title"/>)"
        R"(<w:basedOn w:val="Normal"/>)"
        R"(<w:pPr><w:jc w:val="center"/><w:spacing w:after="200"/></w:pPr>)"
        R"(<w:rPr><w:rFonts w:ascii=")" + xml_escape(style.heading_font) +
        R"(" w:hAnsi=")" + xml_escape(style.heading_font) + R"("/>)"
        R"(<w:b/><w:sz w:val=")" + pt_to_hps(28) + R"("/>)"
        R"(<w:color w:val=")" + style.heading_color.to_hex() + R"("/>)"
        R"(</w:rPr></w:style>)";

    styles_xml +=
        R"(<w:style w:type="paragraph" w:styleId="Subtitle">)"
        R"(<w:name w:val="Subtitle"/>)"
        R"(<w:basedOn w:val="Normal"/>)"
        R"(<w:pPr><w:jc w:val="center"/></w:pPr>)"
        R"(<w:rPr><w:rFonts w:ascii=")" + xml_escape(style.heading_font) +
        R"(" w:hAnsi=")" + xml_escape(style.heading_font) + R"("/>)"
        R"(<w:b/><w:sz w:val=")" + pt_to_hps(18) + R"("/>)"
        R"(<w:color w:val=")" + style.heading_color.to_hex() + R"("/>)"
        R"(</w:rPr></w:style>)";

    // Table Grid style
    styles_xml +=
        R"(<w:style w:type="table" w:styleId="TableGrid">)"
        R"(<w:name w:val="Table Grid"/>)"
        R"(<w:tblPr>)"
        R"(<w:tblBorders>)"
        R"(<w:top w:val="single" w:sz="4" w:space="0" w:color="auto"/>)"
        R"(<w:left w:val="single" w:sz="4" w:space="0" w:color="auto"/>)"
        R"(<w:bottom w:val="single" w:sz="4" w:space="0" w:color="auto"/>)"
        R"(<w:right w:val="single" w:sz="4" w:space="0" w:color="auto"/>)"
        R"(<w:insideH w:val="single" w:sz="4" w:space="0" w:color="auto"/>)"
        R"(<w:insideV w:val="single" w:sz="4" w:space="0" w:color="auto"/>)"
        R"(</w:tblBorders>)"
        R"(</w:tblPr>)"
        R"(</w:style>)";

    styles_xml += R"(</w:styles>)";

    // word/settings.xml
    std::string settings_xml =
        R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
        R"(<w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">)"
        R"(<w:defaultTabStop w:val="720"/>)"
        R"(</w:settings>)";

    // word/fontTable.xml
    std::string font_table =
        R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
        R"(<w:fonts xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">)"
        R"(<w:font w:name=")" + xml_escape(style.heading_font) + R"(">)"
        R"(<w:charset w:val="00"/></w:font>)"
        R"(<w:font w:name=")" + xml_escape(style.body_font) + R"(">)"
        R"(<w:charset w:val="00"/></w:font>)"
        R"(</w:fonts>)";

    // docProps/core.xml
    std::string core_xml =
        R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
        R"(<cp:coreProperties xmlns:cp="http://schemas.openxmlformats.org/package/2006/metadata/core-properties")"
        R"( xmlns:dc="http://purl.org/dc/elements/1.1/")"
        R"( xmlns:dcterms="http://purl.org/dc/terms/">)"
        R"(<dc:creator>Kofu AI Assistant</dc:creator>)"
        R"(<dc:description>Document generated by Kofu</dc:description>)"
        R"(</cp:coreProperties>)";

    // docProps/app.xml
    std::string app_xml =
        R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
        R"(<Properties xmlns="http://schemas.openxmlformats.org/officeDocument/2006/extended-properties">)"
        R"(<Application>Kofu AI Assistant</Application>)"
        R"(<AppVersion>2.0</AppVersion>)"
        R"(</Properties>)";

    // Document content
    std::string document_xml = generate_document_xml(content, style);

    // Add all files to ZIP
    zip_add_buffer(archive, "[Content_Types].xml", content_types);
    zip_add_buffer(archive, "_rels/.rels", root_rels);
    zip_add_buffer(archive, "word/_rels/document.xml.rels", doc_rels);
    zip_add_buffer(archive, "word/document.xml", document_xml);
    zip_add_buffer(archive, "word/styles.xml", styles_xml);
    zip_add_buffer(archive, "word/settings.xml", settings_xml);
    zip_add_buffer(archive, "word/fontTable.xml", font_table);
    zip_add_buffer(archive, "docProps/core.xml", core_xml);
    zip_add_buffer(archive, "docProps/app.xml", app_xml);

    if (zip_close(archive) < 0) {
        throw std::runtime_error("Failed to close ZIP archive: " +
                                 std::string(zip_strerror(archive)));
    }
}

// ---------------------------------------------------------------------------
// convert_template — patch [Content_Types].xml in a .dotx to make it .docx
// ---------------------------------------------------------------------------
std::optional<fs::path> DocxGenerator::convert_template(
    const fs::path& template_path,
    const fs::path& output_dir)
{
    if (!fs::exists(template_path)) return std::nullopt;

    fs::path temp_output = output_dir /
        (template_path.stem().string() + "_temp.docx");

    int err = 0;
    zip_t* src_zip = zip_open(template_path.string().c_str(), ZIP_RDONLY, &err);
    if (!src_zip) return std::nullopt;

    zip_t* dst_zip = zip_open(temp_output.string().c_str(),
                               ZIP_CREATE | ZIP_TRUNCATE, &err);
    if (!dst_zip) {
        zip_close(src_zip);
        return std::nullopt;
    }

    zip_int64_t num_entries = zip_get_num_entries(src_zip, 0);

    for (zip_int64_t i = 0; i < num_entries; ++i) {
        const char* name = zip_get_name(src_zip, static_cast<zip_uint64_t>(i), 0);
        if (!name) continue;

        struct zip_stat st;
        zip_stat_init(&st);
        if (zip_stat_index(src_zip, static_cast<zip_uint64_t>(i), 0, &st) != 0)
            continue;

        zip_file_t* file = zip_fopen_index(src_zip, static_cast<zip_uint64_t>(i), 0);
        if (!file) continue;

        std::string file_content(static_cast<size_t>(st.size), '\0');
        zip_fread(file, file_content.data(), static_cast<zip_uint64_t>(st.size));
        zip_fclose(file);

        // Patch [Content_Types].xml
        std::string entry_name(name);
        if (entry_name == "[Content_Types].xml") {
            // Replace template content type with document content type
            std::string old_type =
                "application/vnd.openxmlformats-officedocument."
                "wordprocessingml.template.main+xml";
            std::string new_type =
                "application/vnd.openxmlformats-officedocument."
                "wordprocessingml.document.main+xml";

            size_t pos = file_content.find(old_type);
            if (pos != std::string::npos) {
                file_content.replace(pos, old_type.size(), new_type);
            }
        }

        zip_add_buffer(dst_zip, name, file_content);
    }

    zip_close(src_zip);
    if (zip_close(dst_zip) < 0) {
        return std::nullopt;
    }

    return temp_output;
}

// ---------------------------------------------------------------------------
// modify_existing_docx — add content to a template-based document
// ---------------------------------------------------------------------------
void DocxGenerator::modify_existing_docx(
    const fs::path& source_docx,
    const fs::path& output_path,
    const std::vector<DocContent>& content,
    const DocStyle& style)
{
    // Read the existing document.xml from the template
    int err = 0;
    zip_t* src_zip = zip_open(source_docx.string().c_str(), ZIP_RDONLY, &err);
    if (!src_zip) {
        // Fallback to from-scratch
        build_docx_from_scratch(output_path, content, style);
        return;
    }

    // Create output ZIP
    zip_t* dst_zip = zip_open(output_path.string().c_str(),
                               ZIP_CREATE | ZIP_TRUNCATE, &err);
    if (!dst_zip) {
        zip_close(src_zip);
        build_docx_from_scratch(output_path, content, style);
        return;
    }

    // Copy all entries from source, replacing document.xml with our content
    std::string new_doc_xml = generate_document_xml(content, style);
    zip_int64_t num_entries = zip_get_num_entries(src_zip, 0);

    for (zip_int64_t i = 0; i < num_entries; ++i) {
        const char* name = zip_get_name(src_zip, static_cast<zip_uint64_t>(i), 0);
        if (!name) continue;

        std::string entry_name(name);

        if (entry_name == "word/document.xml") {
            // Replace with our generated content
            zip_add_buffer(dst_zip, name, new_doc_xml);
        } else {
            // Copy from source
            struct zip_stat st;
            zip_stat_init(&st);
            zip_stat_index(src_zip, static_cast<zip_uint64_t>(i), 0, &st);

            zip_file_t* file = zip_fopen_index(src_zip,
                static_cast<zip_uint64_t>(i), 0);
            if (!file) continue;

            std::string file_content(static_cast<size_t>(st.size), '\0');
            zip_fread(file, file_content.data(),
                      static_cast<zip_uint64_t>(st.size));
            zip_fclose(file);

            zip_add_buffer(dst_zip, name, file_content);
        }
    }

    zip_close(src_zip);
    zip_close(dst_zip);
}

// ---------------------------------------------------------------------------
// save_backup
// ---------------------------------------------------------------------------
fs::path DocxGenerator::save_backup(const fs::path& source) {
    std::error_code ec;
    fs::create_directories(backup_dir_, ec);
    fs::path dest = backup_dir_ / source.filename();
    fs::copy_file(source, dest, fs::copy_options::overwrite_existing, ec);
    return dest;
}

// ---------------------------------------------------------------------------
// create_document — public API
// ---------------------------------------------------------------------------
DocxGenerator::Result DocxGenerator::create_document(
    const std::string& output_filename,
    const std::vector<DocContent>& content,
    const std::string& style_name,
    const std::optional<fs::path>& template_path,
    bool save_backup_flag)
{
    DocStyle style = get_doc_style(style_name);
    fs::path output_path = output_dir_ / output_filename;

    if (template_path.has_value() && fs::exists(*template_path)) {
        // Convert template and use it as base
        auto converted = convert_template(*template_path, output_dir_);
        if (converted.has_value()) {
            modify_existing_docx(*converted, output_path, content, style);
            // Clean up temp file
            std::error_code ec;
            fs::remove(*converted, ec);
        } else {
            build_docx_from_scratch(output_path, content, style);
        }
    } else {
        build_docx_from_scratch(output_path, content, style);
    }

    Result result;
    result.file_path = fs::absolute(output_path);

    if (save_backup_flag) {
        result.backup_path = save_backup(output_path);
    }

    return result;
}

} // namespace kofu
