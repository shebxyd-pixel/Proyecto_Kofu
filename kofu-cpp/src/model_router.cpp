// =============================================================================
// Kofu C++ Backend — M05: Dynamic Model Router
// =============================================================================

#include "kofu/model_router.hpp"

#include <vector>

namespace kofu {

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
ModelRouter::ModelRouter(const OllamaClient& client)
    : client_(client)
{}

// ---------------------------------------------------------------------------
// preferred_model — hardcoded task→model mapping from Python backend
// ---------------------------------------------------------------------------
std::string ModelRouter::preferred_model(TaskType task) {
    switch (task) {
        case TaskType::Chat:          return "qwen2.5:0.5b";
        case TaskType::Document:      return "gemma4:latest";
        case TaskType::Presentation:  return "gemma4:latest";
        case TaskType::Reasoning:     return "llama3.2:3b";
        case TaskType::Research:      return "gemma4:latest";
        case TaskType::CodeGen:       return "qwen2.5-coder:3b";
        case TaskType::Sanitization:  return "qwen2.5:0.5b";
        default:                      return "gemma4:latest";
    }
}

// ---------------------------------------------------------------------------
// task_name — human-readable labels
// ---------------------------------------------------------------------------
std::string ModelRouter::task_name(TaskType task) {
    switch (task) {
        case TaskType::Chat:          return "Chat";
        case TaskType::Document:      return "Documento Word";
        case TaskType::Presentation:  return "Presentación PowerPoint";
        case TaskType::Reasoning:     return "Razonamiento";
        case TaskType::Research:      return "Investigación Web";
        case TaskType::CodeGen:       return "Generación de Código";
        case TaskType::Sanitization:  return "Sanitización";
        default:                      return "General";
    }
}

// ---------------------------------------------------------------------------
// resolve_for_task — with availability validation and fallback chain
// ---------------------------------------------------------------------------
std::string ModelRouter::resolve_for_task(
    TaskType task,
    const std::string& user_requested_model) const
{
    // Build a fallback chain: preferred model for this task, then generics
    std::vector<std::string> fallbacks = {
        preferred_model(task),
        "gemma4:latest",
        "llama3.2:3b",
        "llama3:latest",
        "qwen2.5:0.5b",
    };

    return client_.resolve_model(user_requested_model, fallbacks);
}

} // namespace kofu
