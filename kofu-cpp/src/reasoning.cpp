// =============================================================================
// Kofu C++ Backend — M07: Reasoning Engine
// =============================================================================

#include "kofu/reasoning.hpp"

#include <regex>
#include <algorithm>
#include <sstream>
#include <cctype>

namespace kofu {

// ---------------------------------------------------------------------------
// ReasoningStep
// ---------------------------------------------------------------------------
std::string ReasoningStep::to_string() const {
    std::string s = "Paso " + std::to_string(step_num) + ": " + thought;
    if (!evidence.empty()) {
        s += " (" + evidence + ")";
    }
    return s;
}

// ---------------------------------------------------------------------------
// ChainOfThought
// ---------------------------------------------------------------------------
void ChainOfThought::add_step(const std::string& step) {
    steps_.push_back(step);
}

std::string ChainOfThought::get_chain() const {
    std::string result;
    for (size_t i = 0; i < steps_.size(); ++i) {
        result += std::to_string(i + 1) + ". " + steps_[i] + "\n";
    }
    return result;
}

std::string ChainOfThought::explain() const {
    return "Proceso de razonamiento:\n" + get_chain() +
           "\nConclusión final derivada de los pasos anteriores.";
}

// ---------------------------------------------------------------------------
// ReasoningEngine — local rule-based engine
// ---------------------------------------------------------------------------
ReasoningEngine::ReasoningEngine() {
    rules_ = {
        {"crear_documento_pentesting",
         {"necesita informacion sobre pentesting",
          "necesita un documento de seguridad"},
         "crear_documento_pentesting"},

        {"crear_presentacion_pentesting",
         {"necesita una presentacion de pentesting",
          "presentacion de seguridad"},
         "crear_presentacion_pentesting"},

        {"investigar_tema",
         {"necesita informacion sobre", "quiere saber sobre",
          "investiga", "busca"},
         "investigar_tema"},

        {"dar_consejos_office",
         {"consejos de", "tips de", "trucos de", "como usar"},
         "dar_consejos_office"},

        {"explicar_pentesting",
         {"que es", "explica", "que son"},
         "explicar_pentesting"},
    };
}

void ReasoningEngine::add_fact(const std::string& fact) {
    std::string lower = fact;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    facts_.push_back(lower);
}

void ReasoningEngine::clear_facts() {
    facts_.clear();
    thinking_steps_.clear();
}

std::string ReasoningEngine::extract_topic(const std::string& text) const {
    std::string lower = text;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    static const std::vector<std::regex> patterns = {
        std::regex(R"(sobre\s+(.+?)(?:\?|$|,|\.))"),
        std::regex(R"(de\s+(.+?)(?:\?|$|,|\.))"),
        std::regex(R"(acerca\s+de\s+(.+?)(?:\?|$|,|\.))"),
    };

    for (const auto& pattern : patterns) {
        std::smatch match;
        if (std::regex_search(lower, match, pattern) && match.size() > 1) {
            std::string topic = match[1].str();
            // Trim
            auto start = topic.find_first_not_of(" \t");
            auto end = topic.find_last_not_of(" \t");
            if (start != std::string::npos) {
                return topic.substr(start, end - start + 1);
            }
        }
    }
    return "";
}

std::string ReasoningEngine::fallback_response(const std::string& user_input) const {
    std::string lower = user_input;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    auto contains_any = [&lower](const std::vector<std::string>& words) {
        return std::any_of(words.begin(), words.end(),
            [&lower](const std::string& w) {
                return lower.find(w) != std::string::npos;
            });
    };

    if (contains_any({"pentesting", "seguridad", "hacker"})) {
        return "Sobre pentesting:\n"
               "• Metodologías: OWASP, PTES, OSSTMM, NIST\n"
               "• Fases: Reconocimiento → Escaneo → Enumeración → "
               "Explotación → Post-explotación → Informes\n"
               "• Herramientas: Nmap, Metasploit, Wireshark, Burp Suite\n\n"
               "¿Creo un documento o presentación?";
    }

    if (contains_any({"powerpoint", "presentación", "pptx", "ppt"})) {
        return "Puedo crear una presentación con temas professional, modern "
               "o vibrant. ¿Sobre qué tema?";
    }

    if (contains_any({"word", "documento", "docx"})) {
        return "Puedo crear un documento Word con estilo professional o "
               "modern. ¿Sobre qué tema?";
    }

    return "Puedo ayudarte con:\n"
           "• Pentesting\n"
           "• Documentos Word\n"
           "• Presentaciones PowerPoint\n"
           "• Investigación web\n"
           "• Consejos de Office\n\n"
           "¿Qué te gustaría hacer?";
}

std::pair<std::string, std::vector<ReasoningStep>>
ReasoningEngine::reason(const std::string& user_input) {
    clear_facts();
    add_fact(user_input);
    thinking_steps_.push_back({1, "Analizando entrada del usuario", user_input});

    std::string topic = extract_topic(user_input);
    if (!topic.empty()) {
        thinking_steps_.push_back({2, "Tema identificado: " + topic, ""});
    }

    std::string lower = user_input;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    for (const auto& rule : rules_) {
        bool matches = std::any_of(
            rule.conditions.begin(), rule.conditions.end(),
            [&lower](const std::string& cond) {
                return lower.find(cond) != std::string::npos;
            });

        if (matches) {
            thinking_steps_.push_back(
                {3, "Regla aplicada: " + rule.name, ""});
            return {fallback_response(user_input), thinking_steps_};
        }
    }

    thinking_steps_.push_back({3, "Ninguna regla específica aplicada", ""});
    return {fallback_response(user_input), thinking_steps_};
}

std::string ReasoningEngine::get_thinking_process() const {
    std::string result;
    for (const auto& step : thinking_steps_) {
        result += step.to_string() + "\n";
    }
    return result;
}

// ---------------------------------------------------------------------------
// HybridReasoningEngine
// ---------------------------------------------------------------------------
HybridReasoningEngine::HybridReasoningEngine(
    const OllamaClient* ollama_client, bool use_ollama)
    : ReasoningEngine()
    , ollama_client_(ollama_client)
    , use_ollama_(use_ollama && ollama_client != nullptr)
{}

std::optional<std::string> HybridReasoningEngine::response_with_ollama(
    const std::string& prompt,
    const std::string& system_prompt,
    bool allow_external,
    const std::string& model)
{
    if (use_ollama_ && ollama_client_) {
        try {
            auto [response, model_used] = ollama_client_->reason(
                prompt, model, system_prompt);
            last_model_used_ = model_used;
            return response;
        } catch (const NoModelAvailableError&) {
            // Explicit case: no model installed. Re-throw so HTTP returns 418.
            throw;
        } catch (const std::runtime_error&) {
            // Transient error (timeout, connection). Fall through to fallback.
        }
    }

    // Cloud LLM fallback is not implemented in the C++ version.
    // The Python version used OpenAI API, but we focus on local-only.
    // If allow_external and an external provider were configured, it would
    // go here. For now, return nullopt.
    (void)allow_external;
    return std::nullopt;
}

std::pair<std::string, std::vector<ReasoningStep>>
HybridReasoningEngine::direct_reason(
    const std::string& user_input,
    bool allow_external,
    const std::string& model)
{
    clear_facts();
    add_fact(user_input);
    thinking_steps_.push_back(
        {1, "Acceso directo sin sanitización", user_input});

    auto response = response_with_ollama(
        user_input,
        "Eres un asistente de razonamiento directo. "
        "Responde de forma técnica y concisa.",
        allow_external,
        model);

    if (response.has_value()) {
        thinking_steps_.push_back({2, "Respuesta generada con IA", ""});
        return {*response, thinking_steps_};
    }

    thinking_steps_.push_back(
        {2, "Respuesta directa sin motor generativo", ""});
    return {user_input, thinking_steps_};
}

std::pair<std::string, std::vector<ReasoningStep>>
HybridReasoningEngine::reason(
    const std::string& user_input,
    bool allow_external,
    const std::string& model)
{
    // First: local rules
    auto [rule_response, steps] = ReasoningEngine::reason(user_input);

    // Then: try to enrich with Ollama
    auto ai_response = response_with_ollama(
        user_input, "", allow_external, model);

    if (ai_response.has_value()) {
        steps.push_back({static_cast<int>(steps.size()) + 1,
                        "Respuesta enriquecida con IA generativa", ""});
        return {*ai_response, steps};
    }

    return {rule_response, steps};
}

} // namespace kofu
