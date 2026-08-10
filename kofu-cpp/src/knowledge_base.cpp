// =============================================================================
// Kofu C++ Backend — M06: Knowledge Base & Tips
// =============================================================================

#include "kofu/knowledge_base.hpp"

#include <algorithm>
#include <cctype>

namespace kofu {

// Static tips arrays — 1:1 port from knowledge_base.py
static const std::vector<std::string> POWERPOINT_TIPS = {
    "Usa la regla de 6x6: máximo 6 líneas por diapositiva y 6 palabras por línea",
    "Aplica temas consistentes para mantener el diseño profesional",
    "Usa transiciones moderadas para no distraer",
    "Las animaciones deben ser funcionales, no decorativas",
    "Incluye notas del orador para información adicional",
};

static const std::vector<std::string> WORD_TIPS = {
    "Usa estilos predefinidos para títulos y párrafos",
    "Aplica sangría francesa para listas de referencias",
    "Usa la función de navegación para moverte rápidamente",
    "Habilita la revisión de cambios para colaboraciones",
    "Usa plantillas (.dotx) para mantener la coherencia",
};

std::vector<std::string> get_office_tips(const std::string& software) {
    // Case-insensitive comparison
    std::string lower = software;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (lower == "powerpoint" || lower == "ppt" || lower == "pptx") {
        return POWERPOINT_TIPS;
    }
    if (lower == "word" || lower == "doc" || lower == "docx") {
        return WORD_TIPS;
    }
    return {};
}

} // namespace kofu
