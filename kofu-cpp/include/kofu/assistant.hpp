#ifndef KOFU_ASSISTANT_HPP
#define KOFU_ASSISTANT_HPP

#include "kofu/config.hpp"
#include "kofu/ollama_client.hpp"
#include "kofu/model_router.hpp"
#include "kofu/reasoning.hpp"
#include "kofu/docx_generator.hpp"
#include "kofu/pptx_generator.hpp"
#include "kofu/web_research.hpp"
#include <string>
#include <vector>
#include <optional>
#include <map>

namespace kofu {

class AIAssistant {
public:
    AIAssistant(const Config& config, OllamaClient& ollama_client, ModelRouter& router);

    struct Response {
        std::string response;
        std::vector<ReasoningStep> reasoning_steps;
    };

    struct FileGenerationResult {
        std::string file_path;
        std::string backup_path;
    };

    Response process_request(const std::string& user_input, const std::string& modo, const std::optional<std::string>& model = std::nullopt);
    
    Response direct_reason(const std::string& user_input, const std::string& modo, const std::optional<std::string>& model = std::nullopt);
    
    std::string research_topic(const std::string& topic, const std::string& modo, const std::optional<std::string>& model = std::nullopt, const std::string& style = "neutral", int target_pages = 5);
    
    FileGenerationResult create_presentation(const std::string& topic, const std::string& output_path, const std::string& theme, const std::string& modo, const std::string& filename, const std::optional<std::string>& template_name = std::nullopt, const std::optional<std::string>& model = std::nullopt);
    
    FileGenerationResult create_document(const std::string& topic, const std::string& output_path, const std::string& style, const std::string& modo, const std::string& filename, const std::optional<std::string>& template_name = std::nullopt, const std::optional<std::string>& model = std::nullopt);
    
    std::map<std::string, std::string> digest_file(const std::string& file_path, const std::string& output_path, const std::string& instrucciones, const std::optional<std::string>& model = std::nullopt);
    
    std::vector<std::string> get_office_tips(const std::string& software);
    
    std::string correct_text(const std::string& text);

private:
    Config config_;
    OllamaClient& ollama_;
    ModelRouter& router_;
    HybridReasoningEngine reasoning_engine_;
    WebResearcher web_researcher_;
};

} // namespace kofu

#endif // KOFU_ASSISTANT_HPP
