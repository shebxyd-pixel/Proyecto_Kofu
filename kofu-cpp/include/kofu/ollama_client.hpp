#ifndef KOFU_OLLAMA_CLIENT_HPP
#define KOFU_OLLAMA_CLIENT_HPP

// =============================================================================
// Kofu C++ Backend — M04: Ollama REST API Client
// =============================================================================
// Communicates with the Ollama local inference server via its REST API.
// Handles: model listing, dynamic model resolution, text generation,
// and LLM-assisted input sanitization.
// =============================================================================

#include "kofu/config.hpp"
#include "kofu/json_utils.hpp"

#include <string>
#include <vector>
#include <optional>
#include <stdexcept>
#include <tuple>

namespace kofu {

// ---------------------------------------------------------------------------
// Exception: no usable model available on Ollama
// ---------------------------------------------------------------------------
class NoModelAvailableError : public std::runtime_error {
public:
    explicit NoModelAvailableError(
        const std::string& message = "Error 418; No hay modelo disponible")
        : std::runtime_error(message) {}
};

// ---------------------------------------------------------------------------
// OllamaClient — HTTP client for Ollama REST API
// ---------------------------------------------------------------------------
class OllamaClient {
public:
    /// Construct with a Config (uses ollama_base_url and timeouts from it).
    explicit OllamaClient(const Config& config);

    /// Construct with explicit base URL and timeouts.
    OllamaClient(const std::string& base_url,
                 int timeout_check = 5,
                 int timeout_generate = 120);

    // -- Model management --

    /// Check if Ollama is reachable and has at least one model.
    bool is_available() const;

    /// List all installed model names.
    std::vector<std::string> list_models() const;

    /// Resolve which model to use: checks requested model first,
    /// then iterates default_models. Throws NoModelAvailableError if none.
    std::string resolve_model(
        const std::string& requested_model = "",
        const std::vector<std::string>& fallback_models = {}) const;

    // -- Text generation --

    /// Generate text using POST /api/generate (non-streaming).
    /// Returns the response text.
    /// Throws std::runtime_error on connection/timeout/parse errors.
    std::string generate(
        const std::string& prompt,
        const std::string& model,
        float temperature = 0.7f,
        int num_predict = 1024,
        int timeout_seconds = 0) const;  // 0 = use default timeout

    /// Reason about a prompt: resolves model, prepends system prompt,
    /// calls generate(). Returns (response_text, model_used).
    std::pair<std::string, std::string> reason(
        const std::string& prompt,
        const std::string& requested_model = "",
        const std::string& system_prompt = "") const;

    // -- LLM-assisted sanitization --

    /// Ask the LLM to clean/structure input text as JSON.
    /// Returns (json_result, success_flag).
    /// Never throws — returns fallback JSON on any error.
    std::pair<json, bool> sanitize_input(
        const std::string& dirty_text,
        const std::string& requested_model = "") const;

    // -- Accessors --
    const std::string& base_url() const { return base_url_; }
    std::string last_model_used() const { return last_model_used_; }

private:
    std::string base_url_;
    int timeout_check_;
    int timeout_generate_;
    std::vector<std::string> default_models_;
    mutable std::string last_model_used_; // Track for health endpoint

    /// Fetch the list of available model names from GET /api/tags.
    std::vector<std::string> fetch_available_models() const;
};

} // namespace kofu

#endif // KOFU_OLLAMA_CLIENT_HPP
