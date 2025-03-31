#pragma once
#include "tinyxml2.h"
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <cctype>
#include <optional>
#include <iterator>
#include <queue>
#include <ranges>
#include "utf8.h"

//--------------------------------------------
// Helper functions for UTF-8 string operations
//--------------------------------------------

/** Returns the Unicode (code point) length of a UTF-8 encoded std::string. */
int utf8_length(const std::string &s) {
    return static_cast<int>(utf8::distance(s.begin(), s.end()));
}

/** Returns a substring (in UTF-8) of the given string starting at code point offset 0
 *  and spanning at most count Unicode code points.
 */
std::string utf8_substr(const std::string &s, int count) {
    auto it = s.begin();
    int i = 0;
    while (it != s.end() && i < count) {
        utf8::next(it, s.end());
        ++i;
    }
    return std::string(s.begin(), it);
}


//--------------------------------------------
// Color type with implicit string conversion and RGB storage
//--------------------------------------------

struct Color {
    // Helper to convert hex char to int
    static int hexToInt(char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        return 0;
    }

    // Parse hex string to RGB
    void parseHex(const std::string &hex) {
        if (hex.empty()) return;

        std::string cleaned = hex;
        if (cleaned[0] == '#') {
            cleaned = cleaned.substr(1);
        }

        // Convert to uppercase
        std::transform(cleaned.begin(), cleaned.end(), cleaned.begin(), ::toupper);

        // Support both #RGB and #RRGGBB formats
        if (cleaned.length() == 3) {
            // #RGB format - expand to #RRGGBB
            r = hexToInt(cleaned[0]) * 16 + hexToInt(cleaned[0]);
            g = hexToInt(cleaned[1]) * 16 + hexToInt(cleaned[1]);
            b = hexToInt(cleaned[2]) * 16 + hexToInt(cleaned[2]);
        } else if (cleaned.length() == 6) {
            // #RRGGBB format
            r = hexToInt(cleaned[0]) * 16 + hexToInt(cleaned[1]);
            g = hexToInt(cleaned[2]) * 16 + hexToInt(cleaned[3]);
            b = hexToInt(cleaned[4]) * 16 + hexToInt(cleaned[5]);
        }
    }

    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;

    // Default constructor creates a black color
    Color() : r(0), g(0), b(0) {
    }

    // Construct from RGB values
    Color(unsigned char red, unsigned char green, unsigned char blue)
            : r(red), g(green), b(blue) {
    }

    // Construct from hex string (e.g. "#ff0000")
    Color(const std::string &hexCode) {
        parseHex(hexCode);
    }

    Color(const char *hexCode) {
        if (hexCode) parseHex(hexCode);
    }

    // Convert to hex string (always uppercase with #)
    std::string toHexString() const {
        std::stringstream ss;
        ss << '#' << std::uppercase << std::hex << std::setfill('0')
           << std::setw(2) << static_cast<int>(r)
           << std::setw(2) << static_cast<int>(g)
           << std::setw(2) << static_cast<int>(b);
        return ss.str();
    }

    // Implicit conversion to string
    operator std::string() const {
        return toHexString();
    }

    // Comparison operators for map and general use
    bool operator==(const Color &other) const {
        return r == other.r && g == other.g && b == other.b;
    }

    bool operator!=(const Color &other) const {
        return !(*this == other);
    }

    // Less than operator for map keys
    bool operator<(const Color &other) const {
        if (r != other.r) return r < other.r;
        if (g != other.g) return g < other.g;
        return b < other.b;
    }
};

//--------------------------------------------
// Enumerations for friendly configuration options
//--------------------------------------------

enum class HorizontalAlignment {
    Left, Center, Right
};

enum class FontStyle {
    Default, Monospaced, Serif, SansSerif, Casual, Cursive, SmallCaps
};

enum class EdgeType {
    None, HardShadow, Bevel, GlowOutline, SoftShadow
};

enum class TextAlignment {
    Left, Right, Center
};

//--------------------------------------------
// Conversion helpers for enum values to strings
//--------------------------------------------

std::string toString(HorizontalAlignment align) {
    switch (align) {
        case HorizontalAlignment::Center:
            return "2"; // center alignment value
        case HorizontalAlignment::Right:
            return "1"; // right alignment value
        case HorizontalAlignment::Left:
        default:
            return "0"; // left alignment value
    }
}

std::string toString(FontStyle style) {
    // Map friendly names to numeric codes used in the XML "fs" attribute.
    switch (style) {
        case FontStyle::Monospaced:
            return "1";
        case FontStyle::Serif:
            return "2";
        case FontStyle::SansSerif:
            return "3";
        case FontStyle::Casual:
            return "4";
        case FontStyle::Cursive:
            return "5";
        case FontStyle::SmallCaps:
            return "6";
        case FontStyle::Default:
        default:
            return "0";
    }
}

std::string toString(EdgeType edge) {
    switch (edge) {
        case EdgeType::HardShadow:
            return "1";
        case EdgeType::Bevel:
            return "2";
        case EdgeType::GlowOutline:
            return "3";
        case EdgeType::SoftShadow:
            return "4";
        case EdgeType::None:
        default:
            return "";
    }
}

std::string toString(TextAlignment alignment) {
    switch (alignment) {
        case TextAlignment::Left:
            return "1";
        case TextAlignment::Right:
            return "2";
        case TextAlignment::Center:
            return "3";
        default:
            return "";
    }
}

//--------------------------------------------
// Data structures for chat messages and batches
//--------------------------------------------

struct User {
    std::string name;
    Color color;
};


// A single parsed chat message.
struct ChatMessage {
    uint64_t time = 0; // Timestamp in milliseconds
    User user;
    std::string message;
};

// A single wrapped chat line.
struct ChatLine {
    std::optional<User> user;
    std::string text;
};

// A batch represents the current accumulated chat lines at a given timestamp.
struct Batch {
    int time;
    std::deque<ChatLine> lines;
};

//--------------------------------------------
// Chat parameters using enums for friendly names
//--------------------------------------------
struct ChatParams {
    // Text style options (applied via <pen>)
    bool textBold = false;                    // default: 0 (false)
    u_char textForegroundOpacity = 254;          // default pen fo: "254"
    u_char textBackgroundOpacity = 0;            // default pen bo: "0"

    // Colors – defaults matching the Python tool.
    Color textForegroundColor = {254, 254, 254};    // default foreground color
    Color textBackgroundColor = {254, 254, 254};    // default background color

    // Edge (outline/shadow) settings
    Color textEdgeColor;                      // e.g. "#FF0000"; if not set, leave empty
    EdgeType textEdgeType = EdgeType::SoftShadow; // default pen-et: "4" (SoftShadow)

    // Font settings
    FontStyle fontStyle = FontStyle::SansSerif; // default pen-fs: "3"
    u_char fontSizePercentage = 0;              // default font scale from Python (default: 0)

    // Positioning options
    TextAlignment textAlignment = TextAlignment::Left;                   // workspace ju property (default "0")
    int horizontalMargin = 71;                // margin (default: 71)
    int verticalSpacing = 4;                  // space between lines (default: 4)
    int totalDisplayLines = 11;               // maximum number of lines on screen

    // Batch and wrapping options
    int maxCharsPerLine = 23;                 // maximum characters per line before wrapping
    std::string usernameSeparator = ":";      // separator between username and message
};

std::vector<std::string> wrapText(const std::string &text, int maxChars, int prefixSize) {
    std::vector<std::string> lines;
    if (text.empty() || maxChars <= 0 || prefixSize >= maxChars)
        return lines;

    int firstLineMaxChars = maxChars - prefixSize;
    std::istringstream iss(text);
    std::string word;
    std::string line;
    bool isFirstLine = true;

    while (iss >> word) {
        int currentMaxChars = isFirstLine ? firstLineMaxChars : maxChars;
        int lineLen = utf8_length(line);
        int wordLen = utf8_length(word);

        // If adding the word (with a space) would exceed current line limit
        if (!line.empty() && (lineLen + 1 + wordLen > currentMaxChars)) {
            lines.push_back(line);
            line.clear();
            isFirstLine = false;
        }

        // If the word itself is too long, break it up.
        if (wordLen > currentMaxChars) {
            // If there is a partial line already, push it out.
            if (!line.empty()) {
                lines.push_back(line);
                line.clear();
                isFirstLine = false;
            }
            auto wordIt = word.begin();
            while (wordIt != word.end()) {
                auto startIt = wordIt;
                int count = 0;
                // Advance wordIt by at most currentMaxChars code points.
                while (wordIt != word.end() && count < currentMaxChars) {
                    utf8::next(wordIt, word.end());
                    ++count;
                }
                lines.push_back(std::string(startIt, wordIt));
                currentMaxChars = maxChars;  // after the first broken piece, use full line limit
                isFirstLine = false;
            }
            continue;
        }

        if (!line.empty()) {
            line.append(" ");
        }
        line.append(word);
    }

    if (!line.empty())
        lines.push_back(line);
    return lines;
}

//--------------------------------------------
// Function: generateXML
// Builds batches from ChatMessages, maps friendly ChatParams (with enums) into the XML attributes,
// and returns the XML string using tinyxml2.
//--------------------------------------------
std::string generateXML(const std::vector<ChatMessage> &messages, const ChatParams &params) {
    using namespace tinyxml2;
    XMLDocument doc;

    std::map<Color, std::string> colors;
    colors[params.textForegroundColor] = "";
    for (const auto &m: messages) {
        colors[m.user.color] = "";
    }

    std::vector<Batch> batches;
    std::deque<ChatLine> currentLines;
    for (const auto &msg: messages) {
        std::string username = msg.user.name;
        std::string prefix = username + params.usernameSeparator;

        // Instead of using username.size(), use UTF-8 code point count.
        if (utf8_length(prefix) > params.maxCharsPerLine) {
            int charsToKeep = params.maxCharsPerLine - utf8_length(params.usernameSeparator);
            if (charsToKeep < 0) charsToKeep = 0;
            username = utf8_substr(username, charsToKeep);
            prefix = username + params.usernameSeparator;
        }

        auto wrapped = wrapText(msg.message, params.maxCharsPerLine, utf8_length(prefix));
        if (wrapped.empty())
            continue;

        currentLines.emplace_back(std::make_optional<User>(username, msg.user.color), wrapped[0]);
        if (currentLines.size() > params.totalDisplayLines) currentLines.pop_front();
        for (size_t i = 1; i < wrapped.size(); ++i) {
            currentLines.emplace_back(std::nullopt, wrapped[i]);
            if (currentLines.size() > params.totalDisplayLines) currentLines.pop_front();
        }
        if (!batches.empty() && batches.back().time == msg.time)
            continue;
        batches.emplace_back(msg.time, currentLines);
    }

    // Create the root <timedtext> element.
    XMLElement *root = doc.NewElement("timedtext");
    // Format attribute always set to "3" as in the Python code.
    root->SetAttribute("format", "3");
    doc.InsertFirstChild(root);

    // Create head and body elements.
    XMLElement *head = doc.NewElement("head");
    root->InsertEndChild(head);
    XMLElement *body = doc.NewElement("body");
    root->InsertEndChild(body);

    // Create pen elements for each unique color.
    int penIndex = 0;
    for (auto &kv: colors) {
        const Color &color = kv.first;
        XMLElement *pen = doc.NewElement("pen");
        pen->SetAttribute("id", std::to_string(penIndex).c_str());
        pen->SetAttribute("b", (params.textBold ? "1" : "0"));

        // Use the friendly textForegroundColor if it differs from default white.
        pen->SetAttribute("fc", static_cast<std::string>(color).c_str());
        pen->SetAttribute("fo", std::to_string(params.textForegroundOpacity).c_str());
        pen->SetAttribute("bc", static_cast<std::string>(params.textBackgroundColor).c_str());
        pen->SetAttribute("bo", std::to_string(params.textBackgroundOpacity).c_str());

        // Set edge attributes if provided.
        std::string textEdgeType = toString(params.textEdgeType);
        if (!textEdgeType.empty()) {
            pen->SetAttribute("ec", static_cast<std::string>(params.textEdgeColor).c_str());
            pen->SetAttribute("et", textEdgeType.c_str());
        }

        pen->SetAttribute("fs", toString(params.fontStyle).c_str());
        pen->SetAttribute("sz", std::to_string(params.fontSizePercentage).c_str());
        head->InsertEndChild(pen);
        kv.second = std::to_string(penIndex);
        penIndex++;
    }

    // Create workspace element for positioning.
    XMLElement *ws = doc.NewElement("ws");
    ws->SetAttribute("id", "1"); // default workspace id
    // Use wsJu parameter instead of horizontalAlignment conversion.
    ws->SetAttribute("ju", toString(params.textAlignment).c_str());
    head->InsertEndChild(ws);

    // Create write positioning (wp) elements.
    for (int i = 0; i < params.totalDisplayLines; ++i) {
        XMLElement *wp = doc.NewElement("wp");
        wp->SetAttribute("id", std::to_string(i).c_str());
        wp->SetAttribute("ap", "0"); // anchor point
        wp->SetAttribute("ah", std::to_string(params.horizontalMargin).c_str());
        wp->SetAttribute("av", std::to_string(i * params.verticalSpacing).c_str());
        head->InsertEndChild(wp);
    }
    // Zero-width space (ZWSP) as a UTF-8 string.
    const char *ZWSP = "\xE2\x80\x8B";

    // Build the body: add <p> elements from each batch (except the last one).
    for (size_t batchIndex = 0; batchIndex + 1 < batches.size(); ++batchIndex) {
        const Batch &batch = batches[batchIndex];
        const Batch &nextBatch = batches[batchIndex + 1];
        auto defaultPen = colors[params.textForegroundColor];
        for (const auto &[idx, line]: batch.lines | std::ranges::views::enumerate) {
            XMLElement *pElem = doc.NewElement("p");
            pElem->SetAttribute("t", std::to_string(batch.time).c_str());
            int duration = nextBatch.time - batch.time;
            pElem->SetAttribute("d", std::to_string(duration).c_str());
            pElem->SetAttribute("wp", std::to_string(idx).c_str());
            pElem->SetAttribute("ws", "1");
            pElem->SetAttribute("p", defaultPen.c_str());

            pElem->LinkEndChild(doc.NewText(""));
            if (line.user.has_value()) {
                XMLElement *sUser = doc.NewElement("s");
                sUser->SetAttribute("p", colors[line.user->color].c_str());
                std::string userText = line.user->name + params.usernameSeparator;
                sUser->SetText(userText.c_str());
                pElem->InsertEndChild(sUser);
                pElem->LinkEndChild(doc.NewText(ZWSP));
            }

            XMLElement *sText = doc.NewElement("s");
            sText->SetAttribute("p", defaultPen.c_str());
            sText->SetText(line.text.c_str());
            pElem->InsertEndChild(sText);
            pElem->LinkEndChild(doc.NewText(""));

            body->InsertEndChild(pElem);
        }
    }

    XMLPrinter printer;
    doc.Print(&printer);
    return printer.CStr();
}

//--------------------------------------------
// CSV parser function: parses CSV files with header fields
// ("time", "user_name", "user_color", "message").
// If the user_color field is empty, it uses "#ffffff" as default.
//--------------------------------------------
std::vector<ChatMessage> parseCSV(const std::string &filename) {
    std::vector<ChatMessage> messages;
    std::ifstream file(filename);
    std::string line;
    std::vector<Color> defaultColors = {"#ff0000", "#0000ff", "#008000", "#b22222", "#ff7f50",
                                        "#9acd32", "#ff4500", "#2e8b57", "#daa520", "#d2691e",
                                        "#5f9ea0", "#1e90ff", "#ff69b4", "#8a2be2", "#00ff7f"};
    std::hash<std::string> hasher;
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << "\n";
        return messages;
    }

    // Skip the header
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string field;
        ChatMessage msg;

        // Parse time (and optionally multiply by a factor if needed)
        std::getline(ss, field, ',');
        msg.time = std::stoi(field) * 1000;

        // Parse user name (assumed to be in the second field, e.g. "user_name")
        std::getline(ss, msg.user.name, ',');

        // Parse color: if empty, use "#ffffff"
        std::getline(ss, field, ',');
        const auto &randomColor = defaultColors[hasher(msg.user.name) % defaultColors.size()];
        msg.user.color = field.empty() ? randomColor : Color(field);

        // Parse message (the rest of the line)
        std::getline(ss, msg.message);
        msg.message = msg.message.substr(1, msg.message.size() - 2);
        messages.push_back(msg);
    }

    return messages;
}

//--------------------------------------------
// Test function demonstrating hard-coded chat messages.
//--------------------------------------------
int test() {
    // Example chat messages with different color creation methods
    std::vector<ChatMessage> messages = parseCSV("../chat.csv");

    // Set up friendly parameters using enums.
    ChatParams params;

    std::ofstream out("../out.srv3");
    std::string xmlOutput = generateXML(messages, params);
    out << xmlOutput << std::endl;

    return 0;
}

