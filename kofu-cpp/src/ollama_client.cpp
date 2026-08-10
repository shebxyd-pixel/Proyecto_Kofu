// =============================================================================
// Kofu C++ Backend — M04: Ollama REST API Client
// =============================================================================

#include "kofu/ollama_client.hpp"

#include <httplib.h>
#include <iostream>
#include <algorithm>

namespace kofu {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
namespace {

/// Parse "http://host:port" into (host, port, path_prefix).
struct ParsedUrl {
    std::string host;
    int port;
};

ParsedUrl parse_ollama_url(const std::string& url) {
    ParsedUrl result;
    std::string u = url;

    // Strip trailing slash
    while (!u.empty() && u.back() == '/') u.pop_back();

    // Remove scheme
    if (u.find("http://") == 0) u = u.substr(7);
    else if (u.find("https://") == 0) u = u.substr(8);

    // Split host:port
    auto colon_pos = u.find(':');
    if (colon_pos != std::string::npos) {
        result.host = u.substr(0, colon_pos);
        try {
            result.port = std::stoi(u.substr(colon_pos + 1));
        } catch (...) {
            result.port = 11434;
        }
    } else {
        result.host = u;
        result.port = 11434;
    }

    if (result.host.empty()) result.host = "localhost";
    return result;
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------------
OllamaClient::OllamaClient(const Config& config)
    : base_url_(config.ollama_base_url)
    , timeout_check_(config.ollama_timeout_check)
    , timeout_generate_(config.ollama_timeout_generate)
    , default_models_(config.default_models)
{}

OllamaClient::OllamaClient(const std::string& base_url,
                             int timeout_check,
                             int timeout_generate)
    : base_url_(base_url)
    , timeout_check_(timeout_check)
    , timeout_generate_(timeout_generate)
    , default_models_({"gemma4:latest", "llama3:latest"})
{}

// ---------------------------------------------------------------------------
// fetch_available_models — GET /api/tags
// ---------------------------------------------------------------------------
std::vector<std::string> OllamaClient::fetch_available_models() const {
    auto parsed = parse_ollama_url(base_url_);
    httplib::Client client(parsed.host, parsed.port);
    client.set_connection_timeout(timeout_check_);
    client.set_read_timeout(timeout_check_);

    auto res = client.Get("/api/tags");
    if (!res || res->status != 200) {
        return {};
    }

    auto body = safe_parse(res->body);
    if (!body.has_value() || !body->contains("models")) {
        return {};
    }

    std::vector<std::string> models;
    for (const auto& m : (*body)["models"]) {
        std::string name = json_string(m, "name");
        if (!name.empty()) {
            models.push_back(name);
        }
    }
    return models;
}

// ---------------------------------------------------------------------------
// is_available
// ---------------------------------------------------------------------------
bool OllamaClient::is_available() const {
    return !fetch_available_models().empty();
}

// ---------------------------------------------------------------------------
// list_models
// ---------------------------------------------------------------------------
std::vector<std::string> OllamaClient::list_models() const {
    return fetch_available_models();
}

// ---------------------------------------------------------------------------
// resolve_model — pick the best available model
// ---------------------------------------------------------------------------
std::string OllamaClient::resolve_model(
    const std::string& requested_model,
    const std::vector<std::string>& fallback_models) const
{
    auto available = fetch_available_models();
    if (available.empty()) {
        throw NoModelAvailableError();
    }

    auto is_installed = [&available](const std::string& name) -> bool {
        return std::find(available.begin(), available.end(), name) !=
               available.end();
    };

    // 1. Check explicitly requested model
    std::string requested = requested_model;
    // Trim whitespace
    while (!requested.empty() && std::isspace(static_cast<unsigned char>(requested.front())))
        requested.erase(requested.begin());
    while (!requested.empty() && std::isspace(static_cast<unsigned char>(requested.back())))
        requested.pop_back();

    if (!requested.empty() && is_installed(requested)) {
        last_model_used_ = requested;
        return requested;
    }

    // 2. Check caller-provided fallback list
    for (const auto& candidate : fallback_models) {
        if (is_installed(candidate)) {
            last_model_used_ = candidate;
            return candidate;
        }
    }

    // 3. Check default models from config
    for (const auto& candidate : default_models_) {
        if (is_installed(candidate)) {
            last_model_used_ = candidate;
            return candidate;
        }
    }

    throw NoModelAvailableError();
}

// ---------------------------------------------------------------------------
// generate — POST /api/generate (non-streaming)
// ---------------------------------------------------------------------------
std::string OllamaClient::generate(
    const std::string& prompt,
    const std::string& model,
    float temperature,
    int num_predict,
    int timeout_seconds) const
{
    int timeout = (timeout_seconds > 0) ? timeout_seconds : timeout_generate_;

    auto parsed = parse_ollama_url(base_url_);
    httplib::Client client(parsed.host, parsed.port);
    client.set_connection_timeout(timeout);
    client.set_read_timeout(timeout);
    client.set_write_timeout(timeout);

    json payload = {
        {"model", model},
        {"prompt", prompt},
        {"stream", false},
        {"options", {
            {"temperature", temperature},
            {"num_predict", num_predict}
        }}
    };

    auto res = client.Post("/api/generate",
                           to_compact_string(payload),
                           "application/json");

    if (!res) {
        throw std::runtime_error(
            "Ollama no responde. Verifica que el servicio esté corriendo. "
            "(URL: " + base_url_ + ")");
    }

    if (res->status != 200) {
        throw std::runtime_error(
            "Ollama devolvió código " + std::to_string(res->status) +
            ": " + res->body);
    }

    auto body = safe_parse(res->body);
    if (!body.has_value()) {
        throw std::runtime_error(
            "Respuesta inesperada de Ollama: JSON inválido");
    }

    std::string response = json_string(*body, "response");
    // Trim whitespace
    auto start = response.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    auto end = response.find_last_not_of(" \t\r\n");
    return response.substr(start, end - start + 1);
}

// ---------------------------------------------------------------------------
// reason — resolve model + prepend system prompt + generate
// ---------------------------------------------------------------------------
std::pair<std::string, std::string> OllamaClient::reason(
    const std::string& prompt,
    const std::string& requested_model,
    const std::string& system_prompt) const
{
    std::string model = resolve_model(requested_model);

    std::string sys_msg = system_prompt;
    if (sys_msg.empty()) {
        sys_msg = "Eres Kofu, un asistente de IA para crear documentos y "
                  "presentaciones. Responde de forma clara y útil.";
    }

    std::string full_prompt = sys_msg + "\n\nUSUARIO: " + prompt;
    std::string response = generate(full_prompt, model);

    last_model_used_ = model;
    return {response, model};
}

// ---------------------------------------------------------------------------
// sanitize_input — LLM-assisted JSON cleaning (never throws)
// ---------------------------------------------------------------------------
std::pair<json, bool> OllamaClient::sanitize_input(
    const std::string& dirty_text,
    const std::string& requested_model) const
{
    json fallback = {
        {"texto_limpio", dirty_text},
        {"entidades_detectadas", json::array()},
        {"proceso_exitoso", false}
    };

    std::string model;
    try {
        model = resolve_model(requested_model);
    } catch (const NoModelAvailableError&) {
        return {fallback, false};
    }

    std::string prompt =
        "Devuelve ÚNICAMENTE un JSON con los campos 'texto_limpio', "
        "'entidades_detectadas' y 'proceso_exitoso'.\n\nDATOS: '" +
        dirty_text + "'";

    try {
        std::string raw = generate(prompt, model, 0.1f, 512, 60);
        auto parsed = safe_parse(raw);
        if (parsed.has_value()) {
            return {*parsed, true};
        }
        // Try extract_json as fallback
        auto extracted = extract_json(raw);
        if (extracted.has_value()) {
            return {*extracted, true};
        }
        return {fallback, false};
    } catch (...) {
        return {fallback, false};
    }
}

} // namespace kofu
