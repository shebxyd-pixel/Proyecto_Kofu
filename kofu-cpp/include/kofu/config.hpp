#ifndef KOFU_CONFIG_HPP
#define KOFU_CONFIG_HPP

// =============================================================================
// Kofu C++ Backend — M01: Configuration & .env Parser
// =============================================================================
// Loads configuration from .env files and environment variables.
// Provides all global constants (URLs, timeouts, error codes, ports).
// Thread-safe: immutable after initial load.
// =============================================================================

#include <string>
#include <vector>
#include <filesystem>
#include <optional>

namespace kofu {

// ---------------------------------------------------------------------------
// Configuration struct — all Kofu runtime settings in one place
// ---------------------------------------------------------------------------
struct Config {
    // -- Ollama connection --
    std::string ollama_base_url      = "http://localhost:11434";
    std::vector<std::string> default_models = {"gemma4:latest", "llama3:latest"};

    // -- Ollama timeouts (seconds) --
    int ollama_timeout_check    = 5;
    int ollama_timeout_generate = 120;

    // -- Error codes --
    int    no_model_error_code    = 418;
    std::string no_model_error_message = "Error 418; No hay modelo disponible";

    // -- Server --
    std::vector<int> preferred_ports = {8000, 8080, 3000, 5000};
    std::string      server_host     = "0.0.0.0";

    // -- Paths (resolved at runtime relative to executable) --
    std::filesystem::path web_dir;         // Static frontend files
    std::filesystem::path templates_dir;   // Office templates (.dotx, .potx)
    std::filesystem::path output_dir;      // Generated documents output
    std::filesystem::path backup_dir;      // Backup copies (Archivos/)
    std::filesystem::path env_file;        // .env file path

    // -- Optional API keys --
    std::string google_api_key;
    std::string google_cx;

    // -- Flags --
    bool use_local_llm = true;
    std::string llm_model_name = "gemma4:latest";
};

// ---------------------------------------------------------------------------
// Load configuration
// ---------------------------------------------------------------------------

/// Parse a .env file into key=value pairs, respecting comments (#) and
/// empty lines. Does NOT modify the process environment.
/// Returns the parsed Config with values overridden from the .env file
/// and any matching environment variables (env vars take precedence).
Config load_config(const std::filesystem::path& env_file_path);

/// Overload: auto-detect the .env file by searching upward from the
/// executable's directory.
Config load_config();

// ---------------------------------------------------------------------------
// Port selection
// ---------------------------------------------------------------------------

/// Try to bind to each candidate port in order. Returns the first available
/// port, or a random OS-assigned port if none of the candidates are free.
int find_free_port(const std::vector<int>& candidates);

// ---------------------------------------------------------------------------
// Path resolution
// ---------------------------------------------------------------------------

/// Resolve the project root directory by searching upward from `start_dir`
/// for a directory containing `web/index.html` (the frontend marker).
/// Returns std::nullopt if not found within 10 levels.
std::optional<std::filesystem::path> find_project_root(
    const std::filesystem::path& start_dir);

} // namespace kofu

#endif // KOFU_CONFIG_HPP
