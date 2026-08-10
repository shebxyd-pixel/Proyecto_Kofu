// =============================================================================
// Kofu C++ Backend — M13: Entry Point (main.cpp)
// =============================================================================
// Initializes the configuration, sets up the core modules (OllamaClient,
// ModelRouter, AIAssistant), and starts the HttpServer on the configured port.
// =============================================================================

#include "kofu/config.hpp"
#include "kofu/ollama_client.hpp"
#include "kofu/model_router.hpp"
#include "kofu/assistant.hpp"
#include "kofu/http_server.hpp"

#include <iostream>
#include <exception>

int main() {
    try {
        // 1. Load configuration
        kofu::Config config = kofu::load_config();

        // 2. Initialize Core Modules
        kofu::OllamaClient ollama_client(config);
        kofu::ModelRouter model_router(ollama_client);
        kofu::AIAssistant assistant(config, ollama_client, model_router);

        // 3. Initialize HTTP Server
        kofu::HttpServer server(config, assistant, ollama_client);

        // 4. Start Server (blocks until stopped)
        server.start(config.server_port);

    } catch (const std::exception& e) {
        std::cerr << "CRITICAL ERROR: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
