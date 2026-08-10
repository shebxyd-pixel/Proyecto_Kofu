#include "kofu/web_research.hpp"
#include "kofu/json_utils.hpp"
#include <httplib.h>
#include <regex>
#include <iostream>
#include <algorithm>

namespace kofu {

WebResearcher::WebResearcher(const Config& config) : config_(config) {}

std::vector<SearchResult> WebResearcher::search_web(std::string_view query, int num_results) {
    std::vector<SearchResult> results;

    // 1. Google API
    if (!config_.google_api_key.empty() && !config_.google_cx.empty()) {
        try {
            results = search_google(query, num_results);
            if (!results.empty()) return results;
        } catch (...) {
            // fallback
        }
    }

    // 2. DuckDuckGo scraper
    try {
        results = search_duckduckgo(query, num_results);
        if (!results.empty()) return results;
    } catch (...) {
        // fallback
    }

    // 3. Bing scraper
    try {
        results = search_bing(query, num_results);
        if (!results.empty()) return results;
    } catch (...) {
        // fallback
    }

    return results;
}

std::vector<SearchResult> WebResearcher::search_google(std::string_view query, int num_results) {
    std::vector<SearchResult> results;
    
    // cpp-httplib might need HTTPS for Google API, we try via Client
    httplib::Client cli("https://www.googleapis.com");
    cli.set_connection_timeout(5);

    std::string path = "/customsearch/v1?key=" + httplib::detail::encode_url(config_.google_api_key) + 
                       "&cx=" + httplib::detail::encode_url(config_.google_cx) + 
                       "&q=" + httplib::detail::encode_url(std::string(query)) + 
                       "&num=" + std::to_string(std::min(num_results, 10));

    if (auto res = cli.Get(path)) {
        if (res->status == 200) {
            auto j = safe_parse(res->body);
            if (j.contains("items") && j["items"].is_array()) {
                for (const auto& item : j["items"]) {
                    SearchResult sr;
                    if (item.contains("title") && item["title"].is_string()) sr.title = item["title"];
                    if (item.contains("link") && item["link"].is_string()) sr.url = item["link"];
                    if (item.contains("snippet") && item["snippet"].is_string()) sr.content = item["snippet"];
                    sr.source = "Google Custom Search";
                    results.push_back(sr);
                    if (results.size() >= static_cast<size_t>(num_results)) break;
                }
            }
        }
    }
    return results;
}

std::vector<SearchResult> WebResearcher::search_duckduckgo(std::string_view query, int num_results) {
    std::vector<SearchResult> results;
    
    // DuckDuckGo non-SSL endpoint
    httplib::Client cli("http://html.duckduckgo.com");
    cli.set_connection_timeout(5);

    std::string path = "/html/?q=" + httplib::detail::encode_url(std::string(query));

    if (auto res = cli.Get(path)) {
        if (res->status == 200 || res->status == 301 || res->status == 302) {
            std::string html = res->body;
            
            // Simple regex to extract DuckDuckGo results
            std::regex result_re(R"(class="result__title"[^>]*>\s*<a[^>]*href="([^"]+)"[^>]*>(.*?)</a>.*?"result__snippet[^>]*>(.*?)</a>)");
            
            std::sregex_iterator next(html.begin(), html.end(), result_re);
            std::sregex_iterator end;
            while (next != end && results.size() < static_cast<size_t>(num_results)) {
                std::smatch match = *next;
                SearchResult sr;
                sr.url = match[1].str();
                sr.title = match[2].str();
                sr.content = match[3].str();
                sr.source = "DuckDuckGo";
                
                std::regex tag_re(R"(<[^>]+>)");
                sr.title = std::regex_replace(sr.title, tag_re, "");
                sr.content = std::regex_replace(sr.content, tag_re, "");
                
                if (sr.url.find("uddg=") != std::string::npos) {
                    size_t pos = sr.url.find("uddg=");
                    std::string encoded_url = sr.url.substr(pos + 5);
                    size_t end_pos = encoded_url.find("&");
                    if (end_pos != std::string::npos) {
                        encoded_url = encoded_url.substr(0, end_pos);
                    }
                    sr.url = encoded_url; // It's still URL encoded, but close enough for fallback
                }
                
                results.push_back(sr);
                ++next;
            }
        }
    }
    return results;
}

std::vector<SearchResult> WebResearcher::search_bing(std::string_view query, int num_results) {
    std::vector<SearchResult> results;
    
    // Bing endpoint
    httplib::Client cli("http://www.bing.com");
    cli.set_connection_timeout(5);

    std::string path = "/search?q=" + httplib::detail::encode_url(std::string(query));

    if (auto res = cli.Get(path)) {
        if (res->status == 200) {
            std::string html = res->body;
            
            // Simple regex to extract Bing results
            std::regex result_re(R"(<li class="b_algo">.*?<h2[^>]*>\s*<a[^>]*href="([^"]+)"[^>]*>(.*?)</a>.*?<p[^>]*>(.*?)</p>)");
            
            std::sregex_iterator next(html.begin(), html.end(), result_re);
            std::sregex_iterator end;
            while (next != end && results.size() < static_cast<size_t>(num_results)) {
                std::smatch match = *next;
                SearchResult sr;
                sr.url = match[1].str();
                sr.title = match[2].str();
                sr.content = match[3].str();
                sr.source = "Bing";
                
                std::regex tag_re(R"(<[^>]+>)");
                sr.title = std::regex_replace(sr.title, tag_re, "");
                sr.content = std::regex_replace(sr.content, tag_re, "");
                
                results.push_back(sr);
                ++next;
            }
        }
    }
    return results;
}

std::string WebResearcher::generate_summary(std::string_view topic, const std::vector<SearchResult>& results) {
    std::string summary = "# Research Summary: " + std::string(topic) + "\n\n";
    
    if (results.empty()) {
        summary += "No results found for this topic.\n";
        return summary;
    }

    for (size_t i = 0; i < results.size(); ++i) {
        summary += "## " + std::to_string(i + 1) + ". " + results[i].title + "\n";
        summary += "**URL:** " + results[i].url + "\n";
        summary += "**Source:** " + results[i].source + "\n\n";
        summary += results[i].content + "\n\n";
    }

    return summary;
}

} // namespace kofu
