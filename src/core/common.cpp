#include "common.hpp"

#include <regex>

namespace ietf::sys {
/**
 * @brief Extracts the key from the list XPath.
 *
 * @param list List name.
 * @param key Key name.
 * @param xpath XPath of the list.
 * @return std::string Key value.
 */
const std::string extractListKeyFromXPath(const std::string& list, const std::string& key, const std::string& xpath)
{
    std::string value;

    std::stringstream ss;

    ss << list << "\\[" << key << "='([^']*)'\\]";

    const auto& xpath_expr = ss.str();

    std::regex re(xpath_expr);
    std::smatch xpath_match;

    if (std::regex_search(xpath, xpath_match, re)) {
        value = xpath_match[1];
    } else {
        throw std::runtime_error("Failed to extract key from XPath.");
    }

    return value;
}

}