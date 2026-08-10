#ifndef KOFU_WEB_RESEARCH_HPP
#define KOFU_WEB_RESEARCH_HPP

#include <string>
#include <vector>
#include <string_view>
#include "kofu/config.hpp"

namespace kofu {

struct SearchResult {
    std::string title;
    std::string url;
    std::string content;
    std::string source;
};

class WebResearcher {
public:
    WebResearcher(const Config& config);

    std::vector<SearchResult> search_web(std::string_view query, int num_results = 5);
    std::string generate_summary(std::string_view topic, const std::vector<SearchResult>& results);

private:
    Config config_;

    std::vector<SearchResult> search_google(std::string_view query, int num_results);
    std::vector<SearchResult> search_duckduckgo(std::string_view query, int num_results);
    std::vector<SearchResult> search_bing(std::string_view query, int num_results);
};

} // namespace kofu

#endif // KOFU_WEB_RESEARCH_HPP
