#ifndef KOFU_KNOWLEDGE_BASE_HPP
#define KOFU_KNOWLEDGE_BASE_HPP

// =============================================================================
// Kofu C++ Backend — M06: Knowledge Base & Tips
// =============================================================================
// Static tips for PowerPoint and Word, matching knowledge_base.py.
// =============================================================================

#include <string>
#include <vector>

namespace kofu {

/// Get tips for the specified software ("powerpoint" or "word").
/// Returns an empty vector if the software is not recognized.
std::vector<std::string> get_office_tips(const std::string& software);

} // namespace kofu

#endif // KOFU_KNOWLEDGE_BASE_HPP
