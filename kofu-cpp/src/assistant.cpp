// =============================================================================
// Kofu C++ Backend — M11: AI Assistant Orchestrator
// =============================================================================

#include "kofu/assistant.hpp"
#include "kofu/sanitizer.hpp"
#include "kofu/knowledge_base.hpp"
#include "kofu/json_utils.hpp"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>

namespace kofu {

namespace fs = std::filesystem;

AIAssistant::AIAssistant(const Config& config, OllamaClient& ollama_client, ModelRouter& router)
    : config_(config)
    , ollama_(ollama_client)
    , router_(router)
    , reasoning_engine_(&ollama_client, true)
    , web_researcher_(config) 
{
}

AIAssistant::Response AIAssistant::process_request(const std::string& user_input, const std::string& modo, const std::optional<std::string>& model) {
    std::string safe_input = Sanitizer::clean_text(user_input);
    std::string selected_model = model.value_or(router_.resolve_for_task(TaskType::Reasoning));
    
    auto [resp_str, steps] = reasoning_engine_.reason(safe_input, modo == "online", selected_model);
    
    Response resp;
    resp.response = resp_str;
    resp.reasoning_steps = steps;
    return resp;
}

AIAssistant::Response AIAssistant::direct_reason(const std::string& user_input, const std::string& modo, const std::optional<std::string>& model) {
    std::string selected_model = model.value_or(router_.resolve_for_task(TaskType::Reasoning));
    
    // For direct reason, we might not sanitize as strictly, but let's follow the Python flow
    auto [resp_str, steps] = reasoning_engine_.direct_reason(user_input, modo == "online", selected_model);
    
    Response resp;
    resp.response = resp_str;
    resp.reasoning_steps = steps;
    return resp;
}

std::string AIAssistant::research_topic(const std::string& topic, const std::string& modo, const std::optional<std::string>& model, const std::string& style, int target_pages) {
    std::string safe_topic = Sanitizer::clean_text(topic);
    std::string selected_model = model.value_or(router_.resolve_for_task(TaskType::Research));
    
    auto search_results = web_researcher_.search_web(safe_topic, target_pages);
    std::string raw_summary = web_researcher_.generate_summary(safe_topic, search_results);

    // Use Ollama to synthesize the research if in online mode
    if (modo == "online") {
        std::string prompt = "Sintetiza la siguiente investigación web sobre '" + safe_topic + 
                             "' en un informe detallado con estilo " + style + ":\n\n" + raw_summary;
        try {
            auto [synthesis, used_model] = ollama_.reason(prompt, selected_model, "Eres un asistente de investigación experto.");
            return synthesis;
        } catch (...) {
            return raw_summary;
        }
    }
    
    return raw_summary;
}

AIAssistant::FileGenerationResult AIAssistant::create_presentation(const std::string& topic, const std::string& output_path, const std::string& theme, const std::string& modo, const std::string& filename, const std::optional<std::string>& template_name, const std::optional<std::string>& model) {
    std::string safe_topic = Sanitizer::clean_text(topic);
    std::string selected_model = model.value_or(router_.resolve_for_task(TaskType::Presentation));
    
    std::string prompt = "Create a presentation about " + safe_topic + ". Please return a JSON object with 'title' and 'slides'. Each slide should have 'title', 'text' and 'layout' (int 0-8). Return ONLY valid JSON, no markdown formatting.";
    
    auto [json_str, used_model] = ollama_.generate(prompt, selected_model, "Eres un generador de presentaciones. Solo responde con JSON válido.", 0.8, 4000);
    
    auto json_data = safe_parse(extract_json(json_str));
    if (!json_data.has_value()) {
        throw std::runtime_error("Failed to parse presentation JSON");
    }
    
    std::string title = json_string(*json_data, "title", "Presentation");
    std::vector<SlideData> slides;
    if (json_data->contains("slides") && (*json_data)["slides"].is_array()) {
        for (const auto& slide_json : (*json_data)["slides"]) {
            SlideData slide;
            slide.title = json_string(slide_json, "title", "");
            slide.text = json_string(slide_json, "text", "");
            slide.layout = json_int(slide_json, "layout", 0);
            slides.push_back(slide);
        }
    }
    
    PptTheme ppt_theme = get_ppt_theme(theme);
    PptxGenerator generator(config_.output_dir, config_.backup_dir);
    
    std::string final_output = output_path.empty() ? config_.output_dir.string() : output_path;
    fs::path final_path = fs::path(final_output) / Sanitizer::sanitize_filename(filename);
    if (final_path.extension() != ".pptx") {
        final_path += ".pptx";
    }
    
    std::optional<fs::path> tmpl_path;
    if (template_name && !template_name->empty()) {
        tmpl_path = config_.templates_dir / "powerpoint" / *template_name;
    }
    
    auto result = generator.create_presentation(final_path.filename().string(), slides, ppt_theme.name, tmpl_path, true);
    
    FileGenerationResult res;
    res.file_path = result.file_path.string();
    res.backup_path = result.backup_path.string();
    
    return res;
}

AIAssistant::FileGenerationResult AIAssistant::create_document(const std::string& topic, const std::string& output_path, const std::string& style, const std::string& modo, const std::string& filename, const std::optional<std::string>& template_name, const std::optional<std::string>& model) {
    std::string safe_topic = Sanitizer::clean_text(topic);
    std::string selected_model = model.value_or(router_.resolve_for_task(TaskType::Document));
    
    std::string prompt = "Create a document about " + safe_topic + ". Return a JSON object with 'title' and 'content' array. Each item in 'content' should have 'type' ('heading' or 'paragraph') and 'text'. If type is 'heading', include 'level' (1-3). Return ONLY valid JSON, no markdown formatting.";
    
    auto [json_str, used_model] = ollama_.generate(prompt, selected_model, "Eres un generador de documentos. Solo responde con JSON válido.", 0.8, 4000);
    
    auto json_data = safe_parse(extract_json(json_str));
    if (!json_data.has_value()) {
        throw std::runtime_error("Failed to parse document JSON");
    }
    
    std::vector<DocContent> content_list;
    DocContent title_content;
    title_content.type = "document_title";
    title_content.text = json_string(*json_data, "title", "Document");
    content_list.push_back(title_content);

    if (json_data->contains("content") && (*json_data)["content"].is_array()) {
        for (const auto& item : (*json_data)["content"]) {
            DocContent c;
            c.type = json_string(item, "type", "paragraph");
            c.text = json_string(item, "text", "");
            if (c.type == "heading") {
                c.level = json_int(item, "level", 1);
            }
            content_list.push_back(c);
        }
    }
    
    DocxGenerator generator(config_.output_dir, config_.backup_dir);
    
    std::string final_output = output_path.empty() ? config_.output_dir.string() : output_path;
    fs::path final_path = fs::path(final_output) / Sanitizer::sanitize_filename(filename);
    if (final_path.extension() != ".docx") {
        final_path += ".docx";
    }
    
    std::optional<fs::path> tmpl_path;
    if (template_name && !template_name->empty()) {
        tmpl_path = config_.templates_dir / "word" / *template_name;
    }
    
    auto result = generator.create_document(final_path.filename().string(), content_list, style, tmpl_path, true);
    
    FileGenerationResult res;
    res.file_path = result.file_path.string();
    res.backup_path = result.backup_path.string();
    
    return res;
}

std::map<std::string, std::string> AIAssistant::digest_file(const std::string& file_path, const std::string& output_path, const std::string& instrucciones, const std::optional<std::string>& model) {
    // Incomplete, as MarkItDown is not ported.
    throw std::runtime_error("Digest file not supported in C++ version.");
}

std::vector<std::string> AIAssistant::get_office_tips(const std::string& software) {
    return kofu::get_office_tips(software);
}

std::string AIAssistant::correct_text(const std::string& text) {
    TypoCorrector corrector;
    return corrector.correct_text(text);
}

} // namespace kofu
