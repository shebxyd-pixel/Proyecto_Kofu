// =============================================================================
// Kofu C++ Backend — M01: Configuration & .env Parser
// =============================================================================

#include "kofu/config.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

#ifdef _WIN32
  #include <winsock2.h>
  #pragma comment(lib, "ws2_32.lib")
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <unistd.h>
#endif

namespace kofu {

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------
namespace {

/// Trim leading and trailing whitespace from a string.
std::string trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

/// Remove surrounding quotes (single or double) from a value.
std::string unquote(const std::string& s) {
    if (s.size() >= 2) {
        char front = s.front();
        char back  = s.back();
        if ((front == '"' && back == '"') || (front == '\'' && back == '\'')) {
            return s.substr(1, s.size() - 2);
        }
    }
    return s;
}

/// Get an environment variable, or return a default value.
std::string getenv_or(const char* name, const std::string& default_val) {
    const char* val = std::getenv(name);
    if (val && val[0] != '\0') return std::string(val);
    return default_val;
}

/// Parse a .env file into a key-value map.
/// Supports: KEY=VALUE, KEY="VALUE", KEY='VALUE', # comments, empty lines.
std::vector<std::pair<std::string, std::string>> parse_env_file(
    const fs::path& filepath)
{
    std::vector<std::pair<std::string, std::string>> entries;

    std::ifstream file(filepath);
    if (!file.is_open()) {
        return entries; // File doesn't exist — not an error, use defaults
    }

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;

        // Find the first '='
        auto eq_pos = line.find('=');
        if (eq_pos == std::string::npos) continue;

        std::string key   = trim(line.substr(0, eq_pos));
        std::string value = trim(line.substr(eq_pos + 1));

        // Remove inline comments (not inside quotes)
        if (!value.empty() && value[0] != '"' && value[0] != '\'') {
            auto hash_pos = value.find('#');
            if (hash_pos != std::string::npos) {
                value = trim(value.substr(0, hash_pos));
            }
        }

        value = unquote(value);

        if (!key.empty()) {
            entries.emplace_back(std::move(key), std::move(value));
        }
    }

    return entries;
}

/// Lookup a key in parsed env entries. Returns empty string if not found.
std::string env_lookup(
    const std::vector<std::pair<std::string, std::string>>& entries,
    const std::string& key)
{
    for (auto it = entries.rbegin(); it != entries.rend(); ++it) {
        if (it->first == key) return it->second;
    }
    return "";
}

/// Get a value: first check real env vars, then .env file, then default.
std::string resolve_value(
    const std::vector<std::pair<std::string, std::string>>& env_entries,
    const char* env_name,
    const std::string& default_val)
{
    // Real environment variables take highest precedence
    const char* env_val = std::getenv(env_name);
    if (env_val && env_val[0] != '\0') return std::string(env_val);

    // Then .env file
    std::string file_val = env_lookup(env_entries, env_name);
    if (!file_val.empty()) return file_val;

    // Then default
    return default_val;
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// find_project_root — search upward for web/index.html
// ---------------------------------------------------------------------------
std::optional<fs::path> find_project_root(const fs::path& start_dir) {
    fs::path dir = fs::canonical(start_dir);

    for (int depth = 0; depth < 10; ++depth) {
        if (fs::exists(dir / "web" / "index.html")) {
            return dir;
        }
        auto parent = dir.parent_path();
        if (parent == dir) break; // Reached filesystem root
        dir = parent;
    }

    return std::nullopt;
}

// ---------------------------------------------------------------------------
// find_free_port — bind-test each candidate, fallback to OS-assigned
// ---------------------------------------------------------------------------
int find_free_port(const std::vector<int>& candidates) {
#ifdef _WIN32
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif

    for (int port : candidates) {
#ifdef _WIN32
        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) continue;
#else
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) continue;
#endif
        // Allow address reuse for rapid restart
        int opt = 1;
#ifdef _WIN32
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                   reinterpret_cast<const char*>(&opt), sizeof(opt));
#else
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

        struct sockaddr_in addr{};
        addr.sin_family      = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port        = htons(static_cast<uint16_t>(port));

        int result = bind(sock, reinterpret_cast<struct sockaddr*>(&addr),
                          sizeof(addr));

#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
        if (result == 0) return port;
    }

    // Fallback: let OS assign a random free port
#ifdef _WIN32
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
#else
    int sock = socket(AF_INET, SOCK_STREAM, 0);
#endif
    struct sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = 0; // OS assigns

    bind(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));

    socklen_t len = sizeof(addr);
    getsockname(sock, reinterpret_cast<struct sockaddr*>(&addr), &len);
    int assigned_port = ntohs(addr.sin_port);

#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif

    return assigned_port;
}

// ---------------------------------------------------------------------------
// load_config — from a specific .env file path
// ---------------------------------------------------------------------------
Config load_config(const fs::path& env_file_path) {
    Config cfg;
    cfg.env_file = env_file_path;

    // Parse the .env file
    auto env_entries = parse_env_file(env_file_path);

    // -- Ollama connection --
    cfg.ollama_base_url = resolve_value(
        env_entries, "OLLAMA_BASE_URL", "http://localhost:11434");

    // -- Default models (support primary + fallback override via env) --
    std::string primary  = resolve_value(
        env_entries, "OLLAMA_MODEL_PRIMARY", "");
    std::string fallback = resolve_value(
        env_entries, "OLLAMA_MODEL_FALLBACK", "");
    std::string ollama_model = resolve_value(
        env_entries, "OLLAMA_MODEL", "");

    cfg.default_models.clear();
    if (!ollama_model.empty())
        cfg.default_models.push_back(ollama_model);
    if (!primary.empty() && primary != ollama_model)
        cfg.default_models.push_back(primary);
    if (!fallback.empty())
        cfg.default_models.push_back(fallback);
    // Always ensure at least the hardcoded defaults
    if (cfg.default_models.empty()) {
        cfg.default_models = {"gemma4:latest", "llama3:latest"};
    }

    // -- Flags --
    std::string use_local = resolve_value(env_entries, "USE_LOCAL_LLM", "true");
    cfg.use_local_llm = (use_local == "true" || use_local == "1" ||
                         use_local == "yes" || use_local == "True");

    cfg.llm_model_name = resolve_value(
        env_entries, "LLM_MODEL_NAME", cfg.default_models.front());

    // -- API keys --
    cfg.google_api_key = resolve_value(env_entries, "GOOGLE_API_KEY", "");
    cfg.google_cx      = resolve_value(env_entries, "GOOGLE_CX", "");

    // -- Resolve paths relative to project root --
    auto project_root = find_project_root(env_file_path.parent_path());
    if (!project_root) {
        // Fallback: assume .env is at project root
        project_root = env_file_path.parent_path();
    }

    cfg.web_dir       = *project_root / "web";
    cfg.templates_dir = *project_root / "templates";
    cfg.output_dir    = *project_root / "output";
    cfg.backup_dir    = *project_root / "Archivos";

    // Ensure output directories exist
    std::error_code ec;
    fs::create_directories(cfg.output_dir, ec);
    fs::create_directories(cfg.backup_dir, ec);

    return cfg;
}

// ---------------------------------------------------------------------------
// load_config — auto-detect .env file
// ---------------------------------------------------------------------------
Config load_config() {
    // Strategy: search upward from current working directory
    auto cwd = fs::current_path();
    auto project_root = find_project_root(cwd);

    fs::path env_path;
    if (project_root) {
        env_path = *project_root / ".env";
    } else {
        env_path = cwd / ".env";
    }

    std::cout << "[Config] Loading from: " << env_path << std::endl;
    return load_config(env_path);
}

} // namespace kofu
