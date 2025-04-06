#pragma once

#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <algorithm>
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
#include "tinyxml2.h"
#include "SimpleIni.h"
#include "magic_enum.hpp"


int utf8_length(const std::string &s) {
    return static_cast<int>(utf8::distance(s.begin(), s.end()));
}

std::string utf8_substr(const std::string &s, int count) {
    auto it = s.begin();
    int i = 0;
    while (it != s.end() && i < count) {
        utf8::next(it, s.end());
        ++i;
    }
    return std::string(s.begin(), it);
}

template<typename T, T Max>
struct Clamped {
    T value;

    Clamped(T v = 0) : value(clamp(v)) {}

    static T clamp(T v) {
        return (v > Max) ? Max : v;
    }

    operator T() const {
        return value;
    }

    Clamped &operator=(T v) {
        value = clamp(v);
        return *this;
    }
};

struct Color {
    static const int maxValue = 254;
    typedef unsigned char cType;
    // Use the clamped wrapper for each channel
    Clamped<cType, maxValue> r;
    Clamped<cType, maxValue> g;
    Clamped<cType, maxValue> b;
    Clamped<cType, maxValue> a;

    // Helper to convert hex char to int
    static int hexToInt(char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        return 0;
    }

    // Parse hex string to RGBA
    void parseHex(const std::string &hex) {
        if (hex.empty()) return;

        std::string cleaned = hex;
        if (cleaned[0] == '#') {
            cleaned = cleaned.substr(1);
        }

        // Convert to uppercase for consistency
        std::transform(cleaned.begin(), cleaned.end(), cleaned.begin(), ::toupper);

        // Support formats:
        // - #RGB : 3-digit, assume opaque (alpha = maxValue)
        // - #RGBA : 4-digit, includes alpha
        // - #RRGGBB : 6-digit, assume opaque
        // - #RRGGBBAA : 8-digit, includes alpha
        if (cleaned.length() == 3) {
            r = static_cast<cType>(hexToInt(cleaned[0]) * 16 + hexToInt(cleaned[0]));
            g = static_cast<cType>(hexToInt(cleaned[1]) * 16 + hexToInt(cleaned[1]));
            b = static_cast<cType>(hexToInt(cleaned[2]) * 16 + hexToInt(cleaned[2]));
            a = maxValue;
        } else if (cleaned.length() == 4) {
            r = static_cast<cType>(hexToInt(cleaned[0]) * 16 + hexToInt(cleaned[0]));
            g = static_cast<cType>(hexToInt(cleaned[1]) * 16 + hexToInt(cleaned[1]));
            b = static_cast<cType>(hexToInt(cleaned[2]) * 16 + hexToInt(cleaned[2]));
            a = static_cast<cType>(hexToInt(cleaned[3]) * 16 + hexToInt(cleaned[3]));
        } else if (cleaned.length() == 6) {
            r = static_cast<cType>(hexToInt(cleaned[0]) * 16 + hexToInt(cleaned[1]));
            g = static_cast<cType>(hexToInt(cleaned[2]) * 16 + hexToInt(cleaned[3]));
            b = static_cast<cType>(hexToInt(cleaned[4]) * 16 + hexToInt(cleaned[5]));
            a = maxValue;
        } else if (cleaned.length() == 8) {
            r = static_cast<cType>(hexToInt(cleaned[0]) * 16 + hexToInt(cleaned[1]));
            g = static_cast<cType>(hexToInt(cleaned[2]) * 16 + hexToInt(cleaned[3]));
            b = static_cast<cType>(hexToInt(cleaned[4]) * 16 + hexToInt(cleaned[5]));
            a = static_cast<cType>(hexToInt(cleaned[6]) * 16 + hexToInt(cleaned[7]));
        }
    }

    Color() = default;

    Color(cType red, cType green, cType blue, cType alpha = maxValue)
            : r(red), g(green), b(blue), a(alpha) {}

    Color(const std::string &hexCode) {
        parseHex(hexCode);
    }

    Color(const char *hexCode) {
        if (hexCode) parseHex(hexCode);
    }

    // Convert to hex string (always uppercase with #)
    // If alpha is maxValue, the output will be in #RRGGBB format;
    // otherwise, it will include alpha as #RRGGBBAA.
    std::string toHexString() const {
        std::stringstream ss;
        ss << '#' << std::uppercase << std::hex << std::setfill('0')
           << std::setw(2) << static_cast<int>(r)
           << std::setw(2) << static_cast<int>(g)
           << std::setw(2) << static_cast<int>(b);
        if (a != maxValue) {
            ss << std::setw(2) << static_cast<int>(a);
        }
        return ss.str();
    }

    operator std::string() const {
        return toHexString();
    }

    bool operator==(const Color &other) const {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }

    bool operator!=(const Color &other) const {
        return !(*this == other);
    }

    bool operator<(const Color &other) const {
        if (r != other.r) return r < other.r;
        if (g != other.g) return g < other.g;
        if (b != other.b) return b < other.b;
        return a < other.a;
    }
};

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

//— helpers for enum↔string via magic_enum —//
template<typename E>
std::string enumToString(E e) {
    auto name = magic_enum::enum_name(e);
    return name.empty()
           ? std::to_string(static_cast<std::underlying_type_t<E>>(e))
           : std::string(name);
}

template<typename E>
E enumFromString(const std::string &s) {
    if (auto v = magic_enum::enum_cast<E>(s))
        return *v;
    // try numeric
    try {
        auto i = std::stoi(s);
        return static_cast<E>(i);
    } catch (...) {
        throw std::invalid_argument(
                "Invalid " + std::string(magic_enum::enum_type_name<E>())
                + " value: '" + s + "'");
    }
}

// build a "Options: A, B, C" comment
template<typename E>
std::string enumOptionsComment() {
    std::string c = ";Options: ";
    bool first = true;
    for (auto name: magic_enum::enum_names<E>()) {
        if (!first) c += ", ";
        c += name;
        first = false;
    }
    return c;
}


struct ChatParams {
    bool textBold = false;
    bool textItalic = false;
    bool textUnderline = false;

    Color textForegroundColor = {254, 254, 254, 254};
    Color textBackgroundColor = {254, 254, 254, 0};
    Color textEdgeColor = {254, 254, 254, 254};

    EdgeType textEdgeType = EdgeType::SoftShadow;
    FontStyle fontStyle = FontStyle::SansSerif;
    int fontSizePercent = 0;

    TextAlignment textAlignment = TextAlignment::Left;
    int horizontalMargin = 71;
    int verticalMargin = 0;
    int verticalSpacing = 4;
    int totalDisplayLines = 13;

    int maxCharsPerLine = 25;
    std::string usernameSeparator = ":";

    void saveToFile(const char *filename) const {
        CSimpleIniCaseA ini;
        ini.SetUnicode();  // UTF‑8
        ini.SetQuotes();
        constexpr auto S = "General";

        // bools
        ini.SetBoolValue(S, "bold", textBold,
                         ";true/false");
        ini.SetBoolValue(S, "italic", textItalic,
                         ";true/false");
        ini.SetBoolValue(S, "underline", textUnderline,
                         ";true/false");

        // colors with format comment
        {
            auto hf = textForegroundColor.toHexString();
            ini.SetValue(S, "textForegroundColor", hf.c_str(),
                         ";Hex color: #RGB, #RGBA, #RRGGBB or #RRGGBBAA");
        }
        {
            auto hb = textBackgroundColor.toHexString();
            ini.SetValue(S, "textBackgroundColor", hb.c_str(),
                         ";Hex color: #RGB, #RGBA, #RRGGBB or #RRGGBBAA");
        }
        {
            auto he = textEdgeColor.toHexString();
            ini.SetValue(S, "textEdgeColor", he.c_str(),
                         ";Hex color: #RGB, #RGBA, #RRGGBB or #RRGGBBAA");
        }

        // enums with generated option lists
        {
            auto val = enumToString(textEdgeType);
            auto cm = enumOptionsComment<EdgeType>();
            ini.SetValue(S, "textEdgeType", val.c_str(), cm.c_str());
        }
        {
            auto val = enumToString(fontStyle);
            auto cm = enumOptionsComment<FontStyle>();
            ini.SetValue(S, "fontStyle", val.c_str(), cm.c_str());
        }
        ini.SetLongValue(S, "fontSizePercent", fontSizePercent,
                         ";0–300 (virtual percent)");

        {
            auto val = enumToString(textAlignment);
            auto cm = enumOptionsComment<TextAlignment>();
            ini.SetValue(S, "textAlignment", val.c_str(), cm.c_str());
        }
        ini.SetLongValue(S, "horizontalMargin", horizontalMargin,
                         ";0–100 (virtual percent)");
        ini.SetLongValue(S, "verticalMargin", verticalMargin,
                         ";0-100 (virtual percent)");
        ini.SetLongValue(S, "verticalSpacing", verticalSpacing,
                         ";virtual pixels");
        ini.SetLongValue(S, "totalDisplayLines", totalDisplayLines,
                         ";lines");

        ini.SetLongValue(S, "maxCharsPerLine", maxCharsPerLine,
                         ";characters");
        ini.SetValue(S, "usernameSeparator", usernameSeparator.c_str(),
                     ";string between name and message");

        ini.SaveFile(filename);
    }

    bool loadFromFile(const char *filename) {

        CSimpleIniCaseA ini;
        ini.SetUnicode();
        ini.SetQuotes();
        if (ini.LoadFile(filename) < 0) return false;
        constexpr auto S = "General";

        // bools (fallback to existing defaults)
        textBold = ini.GetBoolValue(S, "bold", textBold);
        textItalic = ini.GetBoolValue(S, "italic", textItalic);
        textUnderline = ini.GetBoolValue(S, "underline", textUnderline);

        // colors (fallback to defaults)
        textForegroundColor = Color{
                ini.GetValue(S, "textForegroundColor",
                             textForegroundColor.toHexString().c_str())
        };
        textBackgroundColor = Color{
                ini.GetValue(S, "textBackgroundColor",
                             textBackgroundColor.toHexString().c_str())
        };
        textEdgeColor = Color{
                ini.GetValue(S, "textEdgeColor",
                             textEdgeColor.toHexString().c_str())
        };

        // enums (fallback via default string)
        try {
            textEdgeType = enumFromString<EdgeType>(
                    ini.GetValue(S, "textEdgeType",
                                 enumToString(textEdgeType).c_str()));
        } catch (...) {
        }
        try {
            fontStyle = enumFromString<FontStyle>(
                    ini.GetValue(S, "fontStyle",
                                 enumToString(fontStyle).c_str()));
        } catch (...) {
        }
        try {
            textAlignment = enumFromString<TextAlignment>(
                    ini.GetValue(S, "textAlignment",
                                 enumToString(textAlignment).c_str()));
        } catch (...) {
        }
        fontSizePercent = static_cast<int>(
                ini.GetLongValue(S, "fontSizePercent",
                                 fontSizePercent));
        horizontalMargin = static_cast<int>(
                ini.GetLongValue(S, "horizontalMargin",
                                 horizontalMargin));
        verticalMargin = static_cast<int>(
                ini.GetLongValue(S, "verticalMargin",
                                 verticalMargin));
        verticalSpacing = static_cast<int>(
                ini.GetLongValue(S, "verticalSpacing",
                                 verticalSpacing));
        totalDisplayLines = static_cast<int>(
                ini.GetLongValue(S, "totalDisplayLines",
                                 totalDisplayLines));

        maxCharsPerLine = static_cast<int>(
                ini.GetLongValue(S, "maxCharsPerLine",
                                 maxCharsPerLine));
        usernameSeparator = ini.GetValue(S, "usernameSeparator",
                                         usernameSeparator.c_str());


        return true;
    }
};


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
                lines.emplace_back(startIt, wordIt);
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

std::vector<std::string> wrapMessage(std::string &username,
                                     const std::string &usernameSeparator,
                                     const std::string &message,
                                     int maxCharsPerLine) {
    if (utf8_length(username) > maxCharsPerLine) {
        int charsToKeep = maxCharsPerLine - utf8_length(usernameSeparator);
        if (charsToKeep < 0) charsToKeep = 0;
        username = utf8_substr(username, charsToKeep);

    }
    return wrapText(usernameSeparator + message, maxCharsPerLine, utf8_length(username));;
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
        auto wrapped = wrapMessage(username, params.usernameSeparator, msg.message, params.maxCharsPerLine);
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
        pen->SetAttribute("i", (params.textItalic ? "1" : "0"));
        pen->SetAttribute("u", (params.textUnderline ? "1" : "0"));

        // Use the friendly textForegroundColor if it differs from default white.
        pen->SetAttribute("fc", static_cast<std::string>(color).c_str());
        pen->SetAttribute("fo", std::to_string(params.textForegroundColor.a).c_str());
        pen->SetAttribute("bc", static_cast<std::string>(params.textBackgroundColor).c_str());
        pen->SetAttribute("bo", std::to_string(params.textBackgroundColor.a).c_str());

        // Set edge attributes if provided.
        std::string textEdgeType = enumToString(params.textEdgeType);
        if (!textEdgeType.empty()) {
            pen->SetAttribute("ec", static_cast<std::string>(params.textEdgeColor).c_str());
            pen->SetAttribute("et", textEdgeType.c_str());
        }

        pen->SetAttribute("fs", enumToString(params.fontStyle).c_str());
        pen->SetAttribute("sz", std::to_string(params.fontSizePercent).c_str());
        head->InsertEndChild(pen);
        kv.second = std::to_string(penIndex);
        penIndex++;
    }

    // Create workspace element for positioning.
    XMLElement *ws = doc.NewElement("ws");
    ws->SetAttribute("id", "1"); // default workspace id
    ws->SetAttribute("ju", enumToString(params.textAlignment).c_str());
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
                std::string userText = line.user->name;
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


Color getRandomColor(const std::string &username) {
    std::vector<Color> defaultColors = {"#ff0000", "#0000ff", "#008000", "#b22222", "#ff7f50",
                                        "#9acd32", "#ff4500", "#2e8b57", "#daa520", "#d2691e",
                                        "#5f9ea0", "#1e90ff", "#ff69b4", "#8a2be2", "#00ff7f"};
    std::hash<std::string> hasher;
    return defaultColors[hasher(username) % defaultColors.size()];

}



