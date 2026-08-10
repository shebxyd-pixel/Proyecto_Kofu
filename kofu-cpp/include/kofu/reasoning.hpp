#ifndef KOFU_REASONING_HPP
#define KOFU_REASONING_HPP

// =============================================================================
// Kofu C++ Backend — M07: Reasoning Engine
// =============================================================================
// Hybrid reasoning: local rules first, then Ollama LLM, then optional
// external cloud LLM fallback. Exact port of reasoning.py.
// =============================================================================

#include "kofu/ollama_client.hpp"

#include <string>
#include <vector>
#include <optional>

namespace kofu {

// ---------------------------------------------------------------------------
// ReasoningStep — single step in the reasoning chain
// ---------------------------------------------------------------------------
struct ReasoningStep {
    int step_num;
    std::string thought;
    std::string evidence;

    std::string to_string() const;
};

// ---------------------------------------------------------------------------
// ChainOfThought — ordered list of reasoning steps
// ---------------------------------------------------------------------------
class ChainOfThought {
public:
    void add_step(const std::string& step);
    std::string get_chain() const;
    std::string explain() const;

private:
    std::vector<std::string> steps_;
};

// ---------------------------------------------------------------------------
// KnowledgeRule — condition→conclusion rule
// ---------------------------------------------------------------------------
struct KnowledgeRule {
    std::string name;
    std::vector<std::string> conditions;
    std::string conclusion;
};

// ---------------------------------------------------------------------------
// ReasoningEngine — fast local rule-based engine (no external deps)
// ---------------------------------------------------------------------------
class ReasoningEngine {
public:
    ReasoningEngine();

    void add_fact(const std::string& fact);
    void clear_facts();

    /// Reason using local rules only. Returns (response, steps).
    std::pair<std::string, std::vector<ReasoningStep>> reason(
        const std::string& user_input);

    std::string get_thinking_process() const;

protected:
    std::string extract_topic(const std::string& text) const;
    std::string fallback_response(const std::string& user_input) const;

    std::vector<KnowledgeRule> rules_;
    std::vector<std::string> facts_;
    std::vector<ReasoningStep> thinking_steps_;
};

// ---------------------------------------------------------------------------
// HybridReasoningEngine — rules + Ollama + optional cloud fallback
// ---------------------------------------------------------------------------
class HybridReasoningEngine : public ReasoningEngine {
public:
    /// Construct with an OllamaClient reference. If use_ollama is false,
    /// the engine will only use local rules.
    HybridReasoningEngine(const OllamaClient* ollama_client = nullptr,
                          bool use_ollama = true);

    /// Direct reasoning: skips sanitization, goes straight to LLM.
    /// Returns (response, steps). Throws NoModelAvailableError if no model.
    std::pair<std::string, std::vector<ReasoningStep>> direct_reason(
        const std::string& user_input,
        bool allow_external = false,
        const std::string& model = "");

    /// Full reasoning: local rules first, then LLM enrichment.
    /// Returns (response, steps). Throws NoModelAvailableError if no model.
    std::pair<std::string, std::vector<ReasoningStep>> reason(
        const std::string& user_input,
        bool allow_external = false,
        const std::string& model = "");

    /// Last model used for an LLM call.
    std::string last_model_used() const { return last_model_used_; }

private:
    /// Try to get a response from Ollama, with optional cloud fallback.
    /// Returns std::nullopt if both fail. Re-throws NoModelAvailableError.
    std::optional<std::string> response_with_ollama(
        const std::string& prompt,
        const std::string& system_prompt = "",
        bool allow_external = false,
        const std::string& model = "");

    const OllamaClient* ollama_client_;
    bool use_ollama_;
    ChainOfThought chain_of_thought_;
    std::string last_model_used_;
};

} // namespace kofu

#endif // KOFU_REASONING_HPP
