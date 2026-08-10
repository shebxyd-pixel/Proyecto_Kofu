// =============================================================================
// Kofu C++ Backend — M09: PPTX Generator
// =============================================================================
// Builds .pptx files (Office Open XML Presentation) natively using libzip.
// A .pptx is a ZIP containing PresentationML XML files.
// =============================================================================

#include "kofu/pptx_generator.hpp"

#include <zip.h>
#include <sstream>
#include <iostream>
#include <cstring>
#include <algorithm>

namespace kofu {

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// get_ppt_theme
// ---------------------------------------------------------------------------
PptTheme get_ppt_theme(const std::string& theme_name) {
    if (theme_name == "modern") {
        return {"modern", {20, 20, 30}, {0, 255, 200}, {240, 240, 240}};
    }
    if (theme_name == "vibrant") {
        return {"vibrant", {255, 248, 220}, {255, 87, 51}, {30, 30, 30}};
    }
    // Default: professional
    return {"professional", {240, 245, 250}, {0, 102, 204}, {30, 30, 30}};
}

// ---------------------------------------------------------------------------
// Helper — add buffer to ZIP
// ---------------------------------------------------------------------------
namespace {

bool pptx_zip_add(zip_t* archive, const char* name, const std::string& content) {
    zip_source_t* source = zip_source_buffer(
        archive, content.data(), content.size(), 0);
    if (!source) return false;
    zip_int64_t idx = zip_file_add(
        archive, name, source, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8);
    if (idx < 0) { zip_source_free(source); return false; }
    return true;
}

/// EMU from centimeters (1 cm = 360000 EMU).
std::string cm_to_emu(double cm) {
    return std::to_string(static_cast<long long>(cm * 360000));
}

/// EMU from points (1 pt = 12700 EMU).
std::string pt_to_emu(int pt) {
    return std::to_string(pt * 12700);
}

/// Half-point to hundredths of a point (font size in PresentationML).
/// PresentationML uses hundredths of a point. 1 pt = 100.
std::string pt_to_pptx_size(int pt) {
    return std::to_string(pt * 100);
}

/// XML-escape special chars.
std::string xml_esc(const std::string& text) {
    std::string r;
    r.reserve(text.size());
    for (char c : text) {
        switch (c) {
            case '&':  r += "&amp;";  break;
            case '<':  r += "&lt;";   break;
            case '>':  r += "&gt;";   break;
            case '"':  r += "&quot;"; break;
            case '\'': r += "&apos;"; break;
            default:   r += c;        break;
        }
    }
    return r;
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
PptxGenerator::PptxGenerator(const fs::path& output_dir,
                             const fs::path& backup_dir)
    : output_dir_(output_dir), backup_dir_(backup_dir)
{
    std::error_code ec;
    fs::create_directories(output_dir_, ec);
    fs::create_directories(backup_dir_, ec);
}

// ---------------------------------------------------------------------------
// generate_slide_xml — PresentationML for a single slide
// ---------------------------------------------------------------------------
std::string PptxGenerator::generate_slide_xml(
    const SlideData& slide,
    const PptTheme& theme,
    int slide_index)
{
    std::ostringstream xml;

    // Slide dimensions: 25.4cm x 19.05cm (standard 4:3) in EMU
    const std::string slide_w = "9144000"; // 25.4 cm
    const std::string slide_h = "6858000"; // 19.05 cm

    xml << R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
        << R"(<p:sld xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main")"
        << R"( xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships")"
        << R"( xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main">)";

    // Slide background
    if (slide.apply_background) {
        xml << R"(<p:cSld>)"
            << R"(<p:bg>)"
            << R"(<p:bgPr>)"
            << R"(<a:solidFill>)"
            << R"(<a:srgbClr val=")" << theme.background.to_hex() << R"("/>)"
            << R"(</a:solidFill>)"
            << R"(<a:effectLst/>)"
            << R"(</p:bgPr>)"
            << R"(</p:bg>)";
    } else {
        xml << R"(<p:cSld>)";
    }

    xml << R"(<p:spTree>)"
        << R"(<p:nvGrpSpPr>)"
        << R"(<p:cNvPr id="1" name=""/>)"
        << R"(<p:cNvGrpSpPr/>)"
        << R"(<p:nvPr/>)"
        << R"(</p:nvGrpSpPr>)"
        << R"(<p:grpSpPr>)"
        << R"(<a:xfrm><a:off x="0" y="0"/><a:ext cx="0" cy="0"/>)"
        << R"(<a:chOff x="0" y="0"/><a:chExt cx="0" cy="0"/></a:xfrm>)"
        << R"(</p:grpSpPr>)";

    // Title shape
    if (!slide.title.empty()) {
        int title_font_size = (slide_index == 0) ? 36 : 28;
        std::string title_y = (slide_index == 0) ? "2000000" : "365125";

        xml << R"(<p:sp>)"
            << R"(<p:nvSpPr>)"
            << R"(<p:cNvPr id="2" name="Title"/>)"
            << R"(<p:cNvSpPr><a:spLocks noGrp="1"/></p:cNvSpPr>)"
            << R"(<p:nvPr><p:ph type="title"/></p:nvPr>)"
            << R"(</p:nvSpPr>)"
            << R"(<p:spPr>)"
            << R"(<a:xfrm>)"
            << R"(<a:off x="457200" y=")" << title_y << R"("/>)"
            << R"(<a:ext cx="8229600" cy="857250"/>)"
            << R"(</a:xfrm>)"
            << R"(</p:spPr>)"
            << R"(<p:txBody>)"
            << R"(<a:bodyPr/>)"
            << R"(<a:lstStyle/>)"
            << R"(<a:p>)"
            << R"(<a:pPr algn="ctr"/>)"
            << R"(<a:r>)"
            << R"(<a:rPr lang="es-MX" sz=")" << pt_to_pptx_size(title_font_size)
            << R"(" b="1" dirty="0"/>)"
            << R"(<a:t>)" << xml_esc(slide.title) << R"(</a:t>)"
            << R"(</a:r>)"
            << R"(</a:p>)"
            << R"(</p:txBody>)"
            << R"(</p:sp>)";
    }

    // Subtitle (for title slides, index 0)
    if (!slide.subtitle.empty() && slide_index == 0) {
        xml << R"(<p:sp>)"
            << R"(<p:nvSpPr>)"
            << R"(<p:cNvPr id="3" name="Subtitle"/>)"
            << R"(<p:cNvSpPr><a:spLocks noGrp="1"/></p:cNvSpPr>)"
            << R"(<p:nvPr><p:ph type="subTitle" idx="1"/></p:nvPr>)"
            << R"(</p:nvSpPr>)"
            << R"(<p:spPr>)"
            << R"(<a:xfrm>)"
            << R"(<a:off x="457200" y="3500000"/>)"
            << R"(<a:ext cx="8229600" cy="857250"/>)"
            << R"(</a:xfrm>)"
            << R"(</p:spPr>)"
            << R"(<p:txBody>)"
            << R"(<a:bodyPr/>)"
            << R"(<a:lstStyle/>)"
            << R"(<a:p>)"
            << R"(<a:pPr algn="ctr"/>)"
            << R"(<a:r>)"
            << R"(<a:rPr lang="es-MX" sz=")" << pt_to_pptx_size(18)
            << R"(" dirty="0"/>)"
            << R"(<a:t>)" << xml_esc(slide.subtitle) << R"(</a:t>)"
            << R"(</a:r>)"
            << R"(</a:p>)"
            << R"(</p:txBody>)"
            << R"(</p:sp>)";
    }

    // Content body (text or bullet points)
    bool has_content = !slide.text.empty() || !slide.bullet_points.empty();
    if (has_content && slide_index > 0) {
        xml << R"(<p:sp>)"
            << R"(<p:nvSpPr>)"
            << R"(<p:cNvPr id="4" name="Content"/>)"
            << R"(<p:cNvSpPr><a:spLocks noGrp="1"/></p:cNvSpPr>)"
            << R"(<p:nvPr><p:ph idx="1"/></p:nvPr>)"
            << R"(</p:nvSpPr>)"
            << R"(<p:spPr>)"
            << R"(<a:xfrm>)"
            << R"(<a:off x="457200" y="1600200"/>)"
            << R"(<a:ext cx="8229600" cy="4525963"/>)"
            << R"(</a:xfrm>)"
            << R"(</p:spPr>)"
            << R"(<p:txBody>)"
            << R"(<a:bodyPr/>)"
            << R"(<a:lstStyle/>)";

        if (!slide.bullet_points.empty()) {
            // Bullet points
            for (const auto& bullet : slide.bullet_points) {
                xml << R"(<a:p>)"
                    << R"(<a:pPr lvl="0">)"
                    << R"(<a:buChar char="•"/>)"
                    << R"(</a:pPr>)"
                    << R"(<a:r>)"
                    << R"(<a:rPr lang="es-MX" sz=")" << pt_to_pptx_size(18)
                    << R"(" dirty="0"/>)"
                    << R"(<a:t>)" << xml_esc(bullet) << R"(</a:t>)"
                    << R"(</a:r>)"
                    << R"(</a:p>)";
            }
        } else {
            // Plain text content, split by newlines into paragraphs
            std::istringstream stream(slide.text);
            std::string line;
            while (std::getline(stream, line)) {
                auto start = line.find_first_not_of(" \t\r");
                if (start == std::string::npos) continue;
                auto end = line.find_last_not_of(" \t\r");
                line = line.substr(start, end - start + 1);

                xml << R"(<a:p>)"
                    << R"(<a:r>)"
                    << R"(<a:rPr lang="es-MX" sz=")" << pt_to_pptx_size(18)
                    << R"(" dirty="0"/>)"
                    << R"(<a:t>)" << xml_esc(line) << R"(</a:t>)"
                    << R"(</a:r>)"
                    << R"(</a:p>)";
            }
        }

        xml << R"(</p:txBody>)"
            << R"(</p:sp>)";
    }

    xml << R"(</p:spTree>)"
        << R"(</p:cSld>)"
        << R"(<p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr>)"
        << R"(</p:sld>)";

    return xml.str();
}

// ---------------------------------------------------------------------------
// build_pptx_from_scratch
// ---------------------------------------------------------------------------
void PptxGenerator::build_pptx_from_scratch(
    const fs::path& output_path,
    const std::vector<SlideData>& slides,
    const PptTheme& theme)
{
    int err = 0;
    zip_t* archive = zip_open(output_path.string().c_str(),
                               ZIP_CREATE | ZIP_TRUNCATE, &err);
    if (!archive) {
        throw std::runtime_error("Cannot create PPTX archive: " +
                                 output_path.string());
    }

    // [Content_Types].xml
    std::ostringstream ct;
    ct << R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
       << R"(<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">)"
       << R"(<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>)"
       << R"(<Default Extension="xml" ContentType="application/xml"/>)"
       << R"(<Override PartName="/ppt/presentation.xml")"
       << R"( ContentType="application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml"/>)";

    for (size_t i = 0; i < slides.size(); ++i) {
        ct << R"(<Override PartName="/ppt/slides/slide)" << (i + 1) << R"(.xml")"
           << R"( ContentType="application/vnd.openxmlformats-officedocument.presentationml.slide+xml"/>)";
    }

    ct << R"(<Override PartName="/ppt/slideMasters/slideMaster1.xml")"
       << R"( ContentType="application/vnd.openxmlformats-officedocument.presentationml.slideMaster+xml"/>)"
       << R"(<Override PartName="/ppt/slideLayouts/slideLayout1.xml")"
       << R"( ContentType="application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml"/>)"
       << R"(<Override PartName="/ppt/theme/theme1.xml")"
       << R"( ContentType="application/vnd.openxmlformats-officedocument.theme+xml"/>)"
       << R"(<Override PartName="/docProps/core.xml")"
       << R"( ContentType="application/vnd.openxmlformats-package.core-properties+xml"/>)"
       << R"(<Override PartName="/docProps/app.xml")"
       << R"( ContentType="application/vnd.openxmlformats-officedocument.extended-properties+xml"/>)"
       << R"(</Types>)";

    // _rels/.rels
    std::string root_rels =
        R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
        R"(<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">)"
        R"(<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument")"
        R"( Target="ppt/presentation.xml"/>)"
        R"(<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties")"
        R"( Target="docProps/core.xml"/>)"
        R"(<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties")"
        R"( Target="docProps/app.xml"/>)"
        R"(</Relationships>)";

    // ppt/_rels/presentation.xml.rels
    std::ostringstream pres_rels;
    pres_rels << R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
              << R"(<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">)";

    for (size_t i = 0; i < slides.size(); ++i) {
        pres_rels << R"(<Relationship Id="rId)" << (i + 1)
                  << R"(" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slide")"
                  << R"( Target="slides/slide)" << (i + 1) << R"(.xml"/>)";
    }

    size_t master_rid = slides.size() + 1;
    pres_rels << R"(<Relationship Id="rId)" << master_rid
              << R"(" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideMaster")"
              << R"( Target="slideMasters/slideMaster1.xml"/>)"
              << R"(</Relationships>)";

    // ppt/presentation.xml
    std::ostringstream pres;
    pres << R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
         << R"(<p:presentation xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main")"
         << R"( xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships")"
         << R"( xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main">)"
         << R"(<p:sldMasterIdLst>)"
         << R"(<p:sldMasterId id="2147483648" r:id="rId)" << master_rid << R"("/>)"
         << R"(</p:sldMasterIdLst>)"
         << R"(<p:sldIdLst>)";

    for (size_t i = 0; i < slides.size(); ++i) {
        pres << R"(<p:sldId id=")" << (256 + i) << R"(" r:id="rId)" << (i + 1) << R"("/>)";
    }

    pres << R"(</p:sldIdLst>)"
         << R"(<p:sldSz cx="9144000" cy="6858000" type="screen4x3"/>)"
         << R"(<p:notesSz cx="6858000" cy="9144000"/>)"
         << R"(</p:presentation>)";

    // Slide master (minimal)
    std::string slide_master =
        R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
        R"(<p:sldMaster xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main")"
        R"( xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships")"
        R"( xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main">)"
        R"(<p:cSld><p:bg><p:bgRef idx="1001"><a:schemeClr val="bg1"/></p:bgRef></p:bg>)"
        R"(<p:spTree><p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/><p:nvPr/>)"
        R"(</p:nvGrpSpPr><p:grpSpPr><a:xfrm><a:off x="0" y="0"/>)"
        R"(<a:ext cx="0" cy="0"/><a:chOff x="0" y="0"/><a:chExt cx="0" cy="0"/>)"
        R"(</a:xfrm></p:grpSpPr></p:spTree></p:cSld>)"
        R"(<p:clrMap bg1="lt1" tx1="dk1" bg2="lt2" tx2="dk2" accent1="accent1")"
        R"( accent2="accent2" accent3="accent3" accent4="accent4" accent5="accent5")"
        R"( accent6="accent6" hlink="hlink" folHlink="folHlink"/>)"
        R"(<p:sldLayoutIdLst>)"
        R"(<p:sldLayoutId id="2147483649" r:id="rId1"/>)"
        R"(</p:sldLayoutIdLst>)"
        R"(</p:sldMaster>)";

    // Slide layout (minimal)
    std::string slide_layout =
        R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
        R"(<p:sldLayout xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main")"
        R"( xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships")"
        R"( xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main")"
        R"( type="blank" preserve="1">)"
        R"(<p:cSld name="Blank"><p:spTree>)"
        R"(<p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/><p:nvPr/>)"
        R"(</p:nvGrpSpPr><p:grpSpPr><a:xfrm><a:off x="0" y="0"/>)"
        R"(<a:ext cx="0" cy="0"/><a:chOff x="0" y="0"/><a:chExt cx="0" cy="0"/>)"
        R"(</a:xfrm></p:grpSpPr></p:spTree></p:cSld>)"
        R"(<p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr>)"
        R"(</p:sldLayout>)";

    // Theme with accent color from theme
    std::string theme_xml =
        R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
        R"(<a:theme xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" name="Kofu Theme">)"
        R"(<a:themeElements>)"
        R"(<a:clrScheme name="Kofu">)"
        R"(<a:dk1><a:srgbClr val=")" + theme.text.to_hex() + R"("/></a:dk1>)"
        R"(<a:lt1><a:srgbClr val=")" + theme.background.to_hex() + R"("/></a:lt1>)"
        R"(<a:dk2><a:srgbClr val=")" + theme.text.to_hex() + R"("/></a:dk2>)"
        R"(<a:lt2><a:srgbClr val=")" + theme.background.to_hex() + R"("/></a:lt2>)"
        R"(<a:accent1><a:srgbClr val=")" + theme.accent.to_hex() + R"("/></a:accent1>)"
        R"(<a:accent2><a:srgbClr val=")" + theme.accent.to_hex() + R"("/></a:accent2>)"
        R"(<a:accent3><a:srgbClr val=")" + theme.accent.to_hex() + R"("/></a:accent3>)"
        R"(<a:accent4><a:srgbClr val=")" + theme.accent.to_hex() + R"("/></a:accent4>)"
        R"(<a:accent5><a:srgbClr val=")" + theme.accent.to_hex() + R"("/></a:accent5>)"
        R"(<a:accent6><a:srgbClr val=")" + theme.accent.to_hex() + R"("/></a:accent6>)"
        R"(<a:hlink><a:srgbClr val="0563C1"/></a:hlink>)"
        R"(<a:folHlink><a:srgbClr val="954F72"/></a:folHlink>)"
        R"(</a:clrScheme>)"
        R"(<a:fontScheme name="Kofu">)"
        R"(<a:majorFont><a:latin typeface="Arial"/><a:ea typeface=""/><a:cs typeface=""/></a:majorFont>)"
        R"(<a:minorFont><a:latin typeface="Calibri"/><a:ea typeface=""/><a:cs typeface=""/></a:minorFont>)"
        R"(</a:fontScheme>)"
        R"(<a:fmtScheme name="Kofu">)"
        R"(<a:fillStyleLst><a:solidFill><a:schemeClr val="phClr"/></a:solidFill>)"
        R"(<a:solidFill><a:schemeClr val="phClr"/></a:solidFill>)"
        R"(<a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:fillStyleLst>)"
        R"(<a:lnStyleLst><a:ln w="9525"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln>)"
        R"(<a:ln w="9525"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln>)"
        R"(<a:ln w="9525"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln></a:lnStyleLst>)"
        R"(<a:effectStyleLst><a:effectStyle><a:effectLst/></a:effectStyle>)"
        R"(<a:effectStyle><a:effectLst/></a:effectStyle>)"
        R"(<a:effectStyle><a:effectLst/></a:effectStyle></a:effectStyleLst>)"
        R"(<a:bgFillStyleLst><a:solidFill><a:schemeClr val="phClr"/></a:solidFill>)"
        R"(<a:solidFill><a:schemeClr val="phClr"/></a:solidFill>)"
        R"(<a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:bgFillStyleLst>)"
        R"(</a:fmtScheme>)"
        R"(</a:themeElements>)"
        R"(<a:objectDefaults/>)"
        R"(<a:extraClrSchemeLst/>)"
        R"(</a:theme>)";

    // Rels for slide master
    std::string master_rels =
        R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
        R"(<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">)"
        R"(<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout")"
        R"( Target="../slideLayouts/slideLayout1.xml"/>)"
        R"(<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme")"
        R"( Target="../theme/theme1.xml"/>)"
        R"(</Relationships>)";

    // Rels for slide layout
    std::string layout_rels =
        R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
        R"(<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">)"
        R"(<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideMaster")"
        R"( Target="../slideMasters/slideMaster1.xml"/>)"
        R"(</Relationships>)";

    // docProps/core.xml
    std::string core_xml =
        R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
        R"(<cp:coreProperties xmlns:cp="http://schemas.openxmlformats.org/package/2006/metadata/core-properties")"
        R"( xmlns:dc="http://purl.org/dc/elements/1.1/">)"
        R"(<dc:creator>Kofu AI Assistant</dc:creator>)"
        R"(<dc:description>Presentation generated by Kofu</dc:description>)"
        R"(</cp:coreProperties>)";

    // docProps/app.xml
    std::string app_xml =
        R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
        R"(<Properties xmlns="http://schemas.openxmlformats.org/officeDocument/2006/extended-properties">)"
        R"(<Application>Kofu AI Assistant</Application>)"
        R"(<Slides>)" + std::to_string(slides.size()) + R"(</Slides>)"
        R"(</Properties>)";

    // Add all files to ZIP
    pptx_zip_add(archive, "[Content_Types].xml", ct.str());
    pptx_zip_add(archive, "_rels/.rels", root_rels);
    pptx_zip_add(archive, "ppt/_rels/presentation.xml.rels", pres_rels.str());
    pptx_zip_add(archive, "ppt/presentation.xml", pres.str());
    pptx_zip_add(archive, "ppt/slideMasters/slideMaster1.xml", slide_master);
    pptx_zip_add(archive, "ppt/slideMasters/_rels/slideMaster1.xml.rels", master_rels);
    pptx_zip_add(archive, "ppt/slideLayouts/slideLayout1.xml", slide_layout);
    pptx_zip_add(archive, "ppt/slideLayouts/_rels/slideLayout1.xml.rels", layout_rels);
    pptx_zip_add(archive, "ppt/theme/theme1.xml", theme_xml);
    pptx_zip_add(archive, "docProps/core.xml", core_xml);
    pptx_zip_add(archive, "docProps/app.xml", app_xml);

    // Generate individual slide files
    for (size_t i = 0; i < slides.size(); ++i) {
        std::string slide_xml = generate_slide_xml(slides[i], theme,
                                                    static_cast<int>(i));
        std::string slide_path = "ppt/slides/slide" + std::to_string(i + 1) + ".xml";
        pptx_zip_add(archive, slide_path.c_str(), slide_xml);

        // Slide rels (reference to slide layout)
        std::string slide_rels =
            R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
            R"(<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">)"
            R"(<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout")"
            R"( Target="../slideLayouts/slideLayout1.xml"/>)"
            R"(</Relationships>)";
        std::string rels_path = "ppt/slides/_rels/slide" +
                                std::to_string(i + 1) + ".xml.rels";
        pptx_zip_add(archive, rels_path.c_str(), slide_rels);
    }

    if (zip_close(archive) < 0) {
        throw std::runtime_error("Failed to close PPTX archive");
    }
}

// ---------------------------------------------------------------------------
// convert_template — .potx → .pptx
// ---------------------------------------------------------------------------
std::optional<fs::path> PptxGenerator::convert_template(
    const fs::path& template_path,
    const fs::path& output_dir)
{
    if (!fs::exists(template_path)) return std::nullopt;

    fs::path temp_output = output_dir /
        (template_path.stem().string() + "_temp.pptx");

    int err = 0;
    zip_t* src_zip = zip_open(template_path.string().c_str(), ZIP_RDONLY, &err);
    if (!src_zip) return std::nullopt;

    zip_t* dst_zip = zip_open(temp_output.string().c_str(),
                               ZIP_CREATE | ZIP_TRUNCATE, &err);
    if (!dst_zip) { zip_close(src_zip); return std::nullopt; }

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

        std::string entry_name(name);
        if (entry_name == "[Content_Types].xml") {
            std::string old_type =
                "application/vnd.openxmlformats-officedocument."
                "presentationml.template.main+xml";
            std::string new_type =
                "application/vnd.openxmlformats-officedocument."
                "presentationml.presentation.main+xml";
            size_t pos = file_content.find(old_type);
            if (pos != std::string::npos) {
                file_content.replace(pos, old_type.size(), new_type);
            }
        }

        pptx_zip_add(dst_zip, name, file_content);
    }

    zip_close(src_zip);
    if (zip_close(dst_zip) < 0) return std::nullopt;
    return temp_output;
}

// ---------------------------------------------------------------------------
// modify_existing_pptx — add slides to a template-based presentation
// ---------------------------------------------------------------------------
void PptxGenerator::modify_existing_pptx(
    const fs::path& source_pptx,
    const fs::path& output_path,
    const std::vector<SlideData>& slides,
    const PptTheme& theme)
{
    // For template-based presentations, we copy the template and add slides.
    // This is simplified: we copy the whole template and inject new slides.
    int err = 0;
    zip_t* src_zip = zip_open(source_pptx.string().c_str(), ZIP_RDONLY, &err);
    if (!src_zip) {
        build_pptx_from_scratch(output_path, slides, theme);
        return;
    }

    zip_t* dst_zip = zip_open(output_path.string().c_str(),
                               ZIP_CREATE | ZIP_TRUNCATE, &err);
    if (!dst_zip) {
        zip_close(src_zip);
        build_pptx_from_scratch(output_path, slides, theme);
        return;
    }

    // Copy all existing entries
    zip_int64_t num_entries = zip_get_num_entries(src_zip, 0);
    for (zip_int64_t i = 0; i < num_entries; ++i) {
        const char* name = zip_get_name(src_zip, static_cast<zip_uint64_t>(i), 0);
        if (!name) continue;

        struct zip_stat st;
        zip_stat_init(&st);
        zip_stat_index(src_zip, static_cast<zip_uint64_t>(i), 0, &st);

        zip_file_t* file = zip_fopen_index(src_zip,
            static_cast<zip_uint64_t>(i), 0);
        if (!file) continue;

        std::string file_content(static_cast<size_t>(st.size), '\0');
        zip_fread(file, file_content.data(), static_cast<zip_uint64_t>(st.size));
        zip_fclose(file);

        pptx_zip_add(dst_zip, name, file_content);
    }

    // Add new slides
    for (size_t i = 0; i < slides.size(); ++i) {
        std::string slide_xml = generate_slide_xml(slides[i], theme,
                                                    static_cast<int>(i));
        std::string slide_path = "ppt/slides/slide" +
            std::to_string(num_entries + i + 1) + ".xml";
        pptx_zip_add(dst_zip, slide_path.c_str(), slide_xml);
    }

    zip_close(src_zip);
    zip_close(dst_zip);
}

// ---------------------------------------------------------------------------
// save_backup
// ---------------------------------------------------------------------------
fs::path PptxGenerator::save_backup(const fs::path& source) {
    std::error_code ec;
    fs::create_directories(backup_dir_, ec);
    fs::path dest = backup_dir_ / source.filename();
    fs::copy_file(source, dest, fs::copy_options::overwrite_existing, ec);
    return dest;
}

// ---------------------------------------------------------------------------
// create_presentation — public API
// ---------------------------------------------------------------------------
PptxGenerator::Result PptxGenerator::create_presentation(
    const std::string& output_filename,
    const std::vector<SlideData>& slides,
    const std::string& theme_name,
    const std::optional<fs::path>& template_path,
    bool save_backup_flag)
{
    PptTheme theme = get_ppt_theme(theme_name);
    fs::path output_path = output_dir_ / output_filename;

    if (template_path.has_value() && fs::exists(*template_path)) {
        auto converted = convert_template(*template_path, output_dir_);
        if (converted.has_value()) {
            modify_existing_pptx(*converted, output_path, slides, theme);
            std::error_code ec;
            fs::remove(*converted, ec);
        } else {
            build_pptx_from_scratch(output_path, slides, theme);
        }
    } else {
        build_pptx_from_scratch(output_path, slides, theme);
    }

    Result result;
    result.file_path = fs::absolute(output_path);

    if (save_backup_flag) {
        result.backup_path = save_backup(output_path);
    }

    return result;
}

} // namespace kofu
