#ifndef KOFU_HTTP_SERVER_HPP
#define KOFU_HTTP_SERVER_HPP

// =============================================================================
// Kofu C++ Backend — M12: HTTP Server & API Routes
// =============================================================================
// cpp-httplib server with all 14 API endpoints, CORS, static file serving,
// multipart file upload, and JSON request/response handling.
// =============================================================================

#include "kofu/config.hpp"
#include "kofu/assistant.hpp"
#include "kofu/ollama_client.hpp"
#include "kofu/json_utils.hpp"

#include <httplib.h>
#include <memory>
#include <string>

namespace kofu {

class HttpServer {
public:
    /// Construct with configuration and all module instances.
    HttpServer(const Config& config,
               AIAssistant& assistant,
               OllamaClient& ollama_client);

    /// Start listening on the specified port. Blocks until stopped.
    void start(int port);

    /// Stop the server gracefully.
    void stop();

private:
    // -- Route handlers --
    void handle_health(const httplib::Request& req, httplib::Response& res);
    void handle_ollama_models(const httplib::Request& req, httplib::Response& res);
    void handle_templates(const httplib::Request& req, httplib::Response& res);
    void handle_chat(const httplib::Request& req, httplib::Response& res);
    void handle_research(const httplib::Request& req, httplib::Response& res);
    void handle_create_powerpoint(const httplib::Request& req, httplib::Response& res);
    void handle_download_powerpoint(const httplib::Request& req, httplib::Response& res);
    void handle_create_word(const httplib::Request& req, httplib::Response& res);
    void handle_download_word(const httplib::Request& req, httplib::Response& res);
    void handle_office_tips(const httplib::Request& req, httplib::Response& res);
    void handle_file_digest(const httplib::Request& req, httplib::Response& res);
    void handle_text_correct(const httplib::Request& req, httplib::Response& res);
    void handle_file_upload(const httplib::Request& req, httplib::Response& res);

    // -- Helpers --
    void setup_routes();
    void add_cors_headers(httplib::Response& res);
    void send_json(httplib::Response& res, const json& body, int status = 200);
    void send_error(httplib::Response& res, const std::string& message, int status);
    void send_file_download(httplib::Response& res, const std::filesystem::path& path,
                           const std::string& content_type, const json& extra_headers = {});

    Config config_;
    AIAssistant& assistant_;
    OllamaClient& ollama_client_;
    httplib::Server server_;
};

} // namespace kofu

#endif // KOFU_HTTP_SERVER_HPP
