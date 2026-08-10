#ifndef KOFU_MODEL_ROUTER_HPP
#define KOFU_MODEL_ROUTER_HPP

// =============================================================================
// Kofu C++ Backend — M05: Dynamic Model Router
// =============================================================================
// Maps task types to optimal Ollama models based on the Python backend's
// hardcoded strategy. Validates model availability before returning.
// =============================================================================

#include "kofu/ollama_client.hpp"
#include <string>

namespace kofu {

// ---------------------------------------------------------------------------
// Task types recognized by the router
// ---------------------------------------------------------------------------
enum class TaskType {
    Chat,           // Simple conversation → smallest model
    Document,       // Word document generation → capable model
    Presentation,   // PowerPoint generation → capable model
    Reasoning,      // Analysis / multi-step reasoning → balanced model
    Research,       // Web research synthesis → capable model
    CodeGen,        // Code generation → code-specialized model
    Sanitization,   // Input sanitization → smallest model
};

// ---------------------------------------------------------------------------
// ModelRouter — dynamic model selection
// ---------------------------------------------------------------------------
class ModelRouter {
public:
    /// Construct with a reference to an OllamaClient for availability checks.
    explicit ModelRouter(const OllamaClient& client);

    /// Resolve the best model for a given task type.
    /// If `user_requested_model` is non-empty and available, it takes priority.
    /// Otherwise, uses the preferred model for the task type.
    /// Throws NoModelAvailableError if no suitable model is found.
    std::string resolve_for_task(
        TaskType task,
        const std::string& user_requested_model = "") const;

    /// Get the preferred model name for a task type (without availability check).
    static std::string preferred_model(TaskType task);

    /// Get a human-readable name for a task type.
    static std::string task_name(TaskType task);

private:
    const OllamaClient& client_;
};

} // namespace kofu

#endif // KOFU_MODEL_ROUTER_HPP
