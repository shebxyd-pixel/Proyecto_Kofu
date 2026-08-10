// =============================================================================
// Kofu C++ Backend — M12: HTTP Server & API Routes
// =============================================================================

#include "kofu/http_server.hpp"
#include "kofu/sanitizer.hpp"
#include "kofu/ollama_client.hpp"
#include "kofu/knowledge_base.hpp"

#include <fstream>
#include <iostream>
#include <algorithm>

namespace kofu {

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
HttpServer::HttpServer(const Config& config,
                       AIAssistant& assistant,
                       OllamaClient& ollama_client)
    : config_(config)
    , assistant_(assistant)
    , ollama_client_(ollama_client)
{
    setup_routes();
}

// ---------------------------------------------------------------------------
// CORS headers — allow all origins (matching Python's CORSMiddleware)
// ---------------------------------------------------------------------------
void HttpServer::add_cors_headers(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods",
                   "GET, POST, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers",
                   "Content-Type, Authorization, X-Requested-With");
    res.set_header("Access-Control-Expose-Headers",
                   "X-Backup-Saved, X-Backup-Path");
}

// ---------------------------------------------------------------------------
// Response helpers
// ---------------------------------------------------------------------------
void HttpServer::send_json(httplib::Response& res, const json& body,
                           int status) {
    add_cors_headers(res);
    res.status = status;
    res.set_content(to_compact_string(body), "application/json");
}

void HttpServer::send_error(httplib::Response& res, const std::string& msg,
                            int status) {
    send_json(res, make_error_response(msg, status), status);
}

void HttpServer::send_file_download(httplib::Response& res,
                                    const fs::path& path,
                                    const std::string& content_type,
                                    const json& extra_headers) {
    add_cors_headers(res);

    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        send_error(res, "File not found: " + path.string(), 404);
        return;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());

    res.set_header("Content-Disposition",
                   "attachment; filename=\"" + path.filename().string() + "\"");

    if (extra_headers.is_object()) {
        for (auto& [key, val] : extra_headers.items()) {
            if (val.is_string()) {
                res.set_header(key, val.get<std::string>());
            }
        }
    }

    res.set_content(content, content_type);
}

// ---------------------------------------------------------------------------
// Route setup
// ---------------------------------------------------------------------------
void HttpServer::setup_routes() {
    // CORS preflight for all routes
    server_.Options(R"(.*)", [this](const httplib::Request&, httplib::Response& res) {
        add_cors_headers(res);
        res.status = 204;
    });

    // Root redirect
    server_.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_redirect("/web/index.html");
    });

    // Static file serving
    if (fs::exists(config_.web_dir)) {
        server_.set_mount_point("/web", config_.web_dir.string());
    }

    // API routes
    server_.Get("/health",
        [this](const httplib::Request& req, httplib::Response& res) {
            handle_health(req, res);
        });

    server_.Get("/ollama/models",
        [this](const httplib::Request& req, httplib::Response& res) {
            handle_ollama_models(req, res);
        });

    server_.Get("/templates",
        [this](const httplib::Request& req, httplib::Response& res) {
            handle_templates(req, res);
        });

    server_.Post("/chat",
        [this](const httplib::Request& req, httplib::Response& res) {
            handle_chat(req, res);
        });

    server_.Post("/research",
        [this](const httplib::Request& req, httplib::Response& res) {
            handle_research(req, res);
        });

    server_.Post("/office/powerpoint",
        [this](const httplib::Request& req, httplib::Response& res) {
            handle_create_powerpoint(req, res);
        });

    server_.Post("/office/powerpoint/download",
        [this](const httplib::Request& req, httplib::Response& res) {
            handle_download_powerpoint(req, res);
        });

    server_.Post("/office/word",
        [this](const httplib::Request& req, httplib::Response& res) {
            handle_create_word(req, res);
        });

    server_.Post("/office/word/download",
        [this](const httplib::Request& req, httplib::Response& res) {
            handle_download_word(req, res);
        });

    server_.Post("/office/tips",
        [this](const httplib::Request& req, httplib::Response& res) {
            handle_office_tips(req, res);
        });

    server_.Post("/files/digest",
        [this](const httplib::Request& req, httplib::Response& res) {
            handle_file_digest(req, res);
        });

    server_.Post("/text/correct",
        [this](const httplib::Request& req, httplib::Response& res) {
            handle_text_correct(req, res);
        });

    server_.Post("/files/upload",
        [this](const httplib::Request& req, httplib::Response& res) {
            handle_file_upload(req, res);
        });
}

// ---------------------------------------------------------------------------
// GET /health
// ---------------------------------------------------------------------------
void HttpServer::handle_health(const httplib::Request&,
                               httplib::Response& res) {
    bool available = ollama_client_.is_available();

    json response = {
        {"status", "ok"},
        {"ollama_available", available},
        {"ollama_modelos_instalados", available ? json(ollama_client_.list_models()) : json::array()},
        {"ollama_ultimo_modelo_usado", ollama_client_.last_model_used()},
        {"file_processor_available", false},  // MarkItDown not ported
        {"modos_disponibles", {"local", "online"}},
    };
    send_json(res, response);
}

// ---------------------------------------------------------------------------
// GET /ollama/models
// ---------------------------------------------------------------------------
void HttpServer::handle_ollama_models(const httplib::Request&,
                                      httplib::Response& res) {
    send_json(res, {{"models", ollama_client_.list_models()}});
}

// ---------------------------------------------------------------------------
// GET /templates
// ---------------------------------------------------------------------------
void HttpServer::handle_templates(const httplib::Request&,
                                  httplib::Response& res) {
    json ppt_templates = json::array();
    json word_templates = json::array();

    fs::path ppt_path = config_.templates_dir / "powerpoint";
    if (fs::exists(ppt_path)) {
        for (const auto& entry : fs::directory_iterator(ppt_path)) {
            std::string name = entry.path().filename().string();
            if (name.starts_with("~$")) continue;
            std::string ext = entry.path().extension().string();
            if (ext == ".pptx" || ext == ".potx" || ext == ".pptm") {
                ppt_templates.push_back(name);
            }
        }
    }

    fs::path word_path = config_.templates_dir / "word";
    if (fs::exists(word_path)) {
        for (const auto& entry : fs::directory_iterator(word_path)) {
            std::string name = entry.path().filename().string();
            if (name.starts_with("~$")) continue;
            std::string ext = entry.path().extension().string();
            if (ext == ".docx" || ext == ".dotx" || ext == ".docm") {
                word_templates.push_back(name);
            }
        }
    }

    send_json(res, {{"powerpoint", ppt_templates}, {"word", word_templates}});
}

// ---------------------------------------------------------------------------
// POST /chat
// ---------------------------------------------------------------------------
void HttpServer::handle_chat(const httplib::Request& req,
                             httplib::Response& res) {
    auto body = safe_parse(req.body);
    if (!body.has_value()) {
        send_error(res, "Invalid JSON body", 400);
        return;
    }

    std::string message = json_string(*body, "message");
    bool direct = json_bool(*body, "direct", false);
    std::string modo = json_string(*body, "modo", "local");
    std::string model = json_string(*body, "model", "qwen2.5:0.5b");

    if (message.empty()) {
        send_error(res, "Message is required", 400);
        return;
    }

    try {
        auto result = direct
            ? assistant_.direct_reason(message, modo, model)
            : assistant_.process_request(message, modo, model);
        auto& response = result.response;
        auto& steps = result.reasoning_steps;

        json steps_json = json::array();
        for (const auto& step : steps) {
            steps_json.push_back(step.to_string());
        }

        send_json(res, {
            {"response", response},
            {"steps", steps_json},
            {"modo", modo},
            {"model_usado", ollama_client_.last_model_used()},
        });
    } catch (const NoModelAvailableError& e) {
        send_error(res, config_.no_model_error_message,
                   config_.no_model_error_code);
    } catch (const std::exception& e) {
        send_error(res, e.what(), 500);
    }
}

// ---------------------------------------------------------------------------
// POST /research
// ---------------------------------------------------------------------------
void HttpServer::handle_research(const httplib::Request& req,
                                 httplib::Response& res) {
    auto body = safe_parse(req.body);
    if (!body.has_value()) { send_error(res, "Invalid JSON", 400); return; }

    std::string topic = json_string(*body, "topic");
    std::string modo = json_string(*body, "modo", "online");
    std::string model = json_string(*body, "model", "gemma4:latest");

    try {
        std::string summary = assistant_.research_topic(topic, modo, model);
        send_json(res, {{"summary", summary}});
    } catch (const std::exception& e) {
        send_error(res, e.what(), 500);
    }
}

// ---------------------------------------------------------------------------
// POST /office/powerpoint
// ---------------------------------------------------------------------------
void HttpServer::handle_create_powerpoint(const httplib::Request& req,
                                          httplib::Response& res) {
    auto body = safe_parse(req.body);
    if (!body.has_value()) { send_error(res, "Invalid JSON", 400); return; }

    std::string topic = json_string(*body, "topic");
    std::string theme = json_string(*body, "theme", "professional");
    std::string modo = json_string(*body, "modo", "online");
    std::string filename = json_string(*body, "filename");
    std::string tmpl = json_string(*body, "template");
    std::string model = json_string(*body, "model", "gemma4:latest");

    try {
        auto result = assistant_.create_presentation(
            topic, output_path, theme, modo, filename, tmpl, model);

        std::string backup_name = result.backup_path.empty()
            ? "" : fs::path(result.backup_path).filename().string();

        send_json(res, {
            {"success", true},
            {"file_path", result.file_path},
            {"filename", fs::path(result.file_path).filename().string()},
            {"backup_path", result.backup_path},
            {"backup_saved", !result.backup_path.empty()},
            {"backup_message", backup_name.empty() ? json(nullptr)
                : json("✅ Copia guardada en Archivos/" + backup_name)},
        });
    } catch (const std::exception& e) {
        send_error(res, e.what(), 500);
    }
}

// ---------------------------------------------------------------------------
// POST /office/powerpoint/download
// ---------------------------------------------------------------------------
void HttpServer::handle_download_powerpoint(const httplib::Request& req,
                                            httplib::Response& res) {
    auto body = safe_parse(req.body);
    if (!body.has_value()) { send_error(res, "Invalid JSON", 400); return; }

    std::string topic = json_string(*body, "topic");
    std::string theme = json_string(*body, "theme", "professional");
    std::string modo = json_string(*body, "modo", "online");
    std::string filename = json_string(*body, "filename");
    std::string tmpl = json_string(*body, "template");
    std::string model = json_string(*body, "model", "gemma4:latest");

    try {
        auto result = assistant_.create_presentation(
            topic, output_path, theme, modo, filename, tmpl, model);

        json headers = {
            {"X-Backup-Saved", result.backup_path.empty() ? "false" : "true"},
            {"X-Backup-Path", result.backup_path},
        };

        send_file_download(res, result.file_path,
            "application/vnd.openxmlformats-officedocument.presentationml.presentation",
            headers);
    } catch (const std::exception& e) {
        send_error(res, e.what(), 500);
    }
}

// ---------------------------------------------------------------------------
// POST /office/word
// ---------------------------------------------------------------------------
void HttpServer::handle_create_word(const httplib::Request& req,
                                    httplib::Response& res) {
    auto body = safe_parse(req.body);
    if (!body.has_value()) { send_error(res, "Invalid JSON", 400); return; }

    std::string topic = json_string(*body, "topic");
    std::string style = json_string(*body, "style", "professional");
    std::string modo = json_string(*body, "modo", "online");
    std::string filename = json_string(*body, "filename");
    std::string tmpl = json_string(*body, "template");
    std::string model = json_string(*body, "model", "gemma4:latest");

    try {
        auto result = assistant_.create_document(
            topic, output_path, style, modo, filename, tmpl, model);

        std::string backup_name = result.backup_path.empty()
            ? "" : fs::path(result.backup_path).filename().string();

        send_json(res, {
            {"success", true},
            {"file_path", result.file_path},
            {"filename", fs::path(result.file_path).filename().string()},
            {"backup_path", result.backup_path},
            {"backup_saved", !result.backup_path.empty()},
            {"backup_message", backup_name.empty() ? json(nullptr)
                : json("✅ Copia guardada en Archivos/" + backup_name)},
        });
    } catch (const std::exception& e) {
        send_error(res, e.what(), 500);
    }
}

// ---------------------------------------------------------------------------
// POST /office/word/download
// ---------------------------------------------------------------------------
void HttpServer::handle_download_word(const httplib::Request& req,
                                      httplib::Response& res) {
    auto body = safe_parse(req.body);
    if (!body.has_value()) { send_error(res, "Invalid JSON", 400); return; }

    std::string topic = json_string(*body, "topic");
    std::string style = json_string(*body, "style", "professional");
    std::string modo = json_string(*body, "modo", "online");
    std::string filename = json_string(*body, "filename");
    std::string tmpl = json_string(*body, "template");
    std::string model = json_string(*body, "model", "gemma4:latest");

    try {
        auto result = assistant_.create_document(
            topic, output_path, style, modo, filename, tmpl, model);

        json headers = {
            {"X-Backup-Saved", result.backup_path.empty() ? "false" : "true"},
            {"X-Backup-Path", result.backup_path},
        };

        send_file_download(res, result.file_path,
            "application/vnd.openxmlformats-officedocument.wordprocessingml.document",
            headers);
    } catch (const std::exception& e) {
        send_error(res, e.what(), 500);
    }
}

// ---------------------------------------------------------------------------
// POST /office/tips
// ---------------------------------------------------------------------------
void HttpServer::handle_office_tips(const httplib::Request& req,
                                    httplib::Response& res) {
    auto body = safe_parse(req.body);
    if (!body.has_value()) { send_error(res, "Invalid JSON", 400); return; }

    std::string software = json_string(*body, "software");
    auto tips = get_office_tips(software);
    send_json(res, {{"tips", tips}});
}

// ---------------------------------------------------------------------------
// POST /files/digest
// ---------------------------------------------------------------------------
void HttpServer::handle_file_digest(const httplib::Request&,
                                    httplib::Response& res) {
    // MarkItDown is not ported to C++ (Python-only library).
    send_error(res,
        "File digest no está disponible en la versión C++. "
        "Usa la versión Python para procesar archivos.", 503);
}

// ---------------------------------------------------------------------------
// POST /text/correct
// ---------------------------------------------------------------------------
void HttpServer::handle_text_correct(const httplib::Request& req,
                                     httplib::Response& res) {
    auto body = safe_parse(req.body);
    if (!body.has_value()) { send_error(res, "Invalid JSON", 400); return; }

    std::string text = json_string(*body, "text");

    std::string sanitized = Sanitizer::is_safe_input(text)
        ? text : Sanitizer::clean_text(text);

    TypoCorrector corrector;
    std::string corrected = corrector.correct_text(sanitized);

    send_json(res, {
        {"original", text},
        {"sanitizado", sanitized},
        {"corregido", corrected},
    });
}

// ---------------------------------------------------------------------------
// POST /files/upload
// ---------------------------------------------------------------------------
void HttpServer::handle_file_upload(const httplib::Request& req,
                                    httplib::Response& res) {
    if (!req.has_file("file")) {
        send_error(res, "No file provided", 400);
        return;
    }

    const auto& file = req.get_file_value("file");
    std::string filename = Sanitizer::sanitize_filename(file.filename);

    if (filename.empty()) {
        send_error(res, "Invalid filename", 400);
        return;
    }

    // Save to Archivos/ directory
    std::error_code ec;
    fs::create_directories(config_.backup_dir, ec);
    fs::path file_path = config_.backup_dir / filename;

    std::ofstream ofs(file_path, std::ios::binary);
    if (!ofs.is_open()) {
        send_error(res, "Cannot save file", 500);
        return;
    }
    ofs.write(file.content.data(), static_cast<std::streamsize>(file.content.size()));
    ofs.close();

    send_json(res, {
        {"success", true},
        {"filename", filename},
        {"file_path", file_path.string()},
        {"size_bytes", file.content.size()},
        {"message", "Archivo guardado. Nota: conversión de contenido no disponible en versión C++."},
    });
}

// ---------------------------------------------------------------------------
// start / stop
// ---------------------------------------------------------------------------
void HttpServer::start(int port) {
    std::cout << "\n"
              << "╔════════════════════════════════════════╗\n"
              << "║     Kofu C++ Backend v2.0.0            ║\n"
              << "║     http://localhost:" << port << "               ║\n"
              << "╚════════════════════════════════════════╝\n"
              << std::endl;

    server_.listen(config_.server_host, port);
}

void HttpServer::stop() {
    server_.stop();
}

} // namespace kofu
