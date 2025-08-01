#pragma once

#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <fstream>
#include <cctype>
#include <filesystem>
#include <optional>
#include <iterator>
#include <queue>
#include <ranges>

#if defined(_WIN32)
#undef assert
#endif

#include "utf8.h"
#include "tinyxml2.h"
#include "SimpleIni.h"
#include "magic_enum.hpp"
#include <format>

// Returns the number of UTF‑8 code points in s.
inline int utf8_length(const std::string &s) {
    return static_cast<int>(utf8::distance(s.begin(), s.end()));
}

// Returns the first 'count' UTF‑8 code points of s.
inline std::string utf8_substr(const std::string &s, int count) {
    auto it = s.begin();
    int i = 0;
    while (it != s.end() && i < count) {
        utf8::next(it, s.end());
        ++i;
    }
    return {s.begin(), it};
}

// Returns the remainder of s after consuming the first 'count' UTF‑8 code points.
inline std::string utf8_consume(const std::string &s, int count) {
    auto it = s.begin();
    int i = 0;
    while (it != s.end() && i < count) {
        utf8::next(it, s.end());
        ++i;
    }
    return {it, s.end()};
}

template<typename T, T Max>
struct Clamped {
    T value;

    Clamped(T v = 0) : value(clamp(v)) {
    }

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
    static constexpr int maxValue = 254;
    typedef unsigned char cType;

    Clamped<cType, maxValue> r;
    Clamped<cType, maxValue> g;
    Clamped<cType, maxValue> b;
    Clamped<cType, maxValue> a;


    static int hexToInt(char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        return 0;
    }


    void parseHex(const std::string &hex) {
        if (hex.empty()) return;

        std::string cleaned = hex;
        if (cleaned[0] == '#') {
            cleaned = cleaned.substr(1);
        }

        // Convert to uppercase for consistency
        std::ranges::transform(cleaned, cleaned.begin(), ::toupper);

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
        : r(red), g(green), b(blue), a(alpha) {
    }

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

    std::string toAssColor() const {
        std::stringstream ss;
        ss << "{\\c&H"
                << std::uppercase << std::hex << std::setfill('0')
                << std::setw(2) << static_cast<int>(b)
                << std::setw(2) << static_cast<int>(g)
                << std::setw(2) << static_cast<int>(r)
                << "&";

        // Here we check if alpha is not equal to maxValue (opaque).
        // If you want to force output alpha, simply output it.
        if (a != maxValue) {
            ss << "\\a&H"
                    << std::setw(2) << static_cast<int>(a)
                    << "&";
        }
        ss << "}";
        return ss.str();
    }
};

enum class HorizontalAlignment {
    Left, Center, Right
};

enum class FontStyle {
    Default,
    Monospaced, // Courier New
    Proportional, // Times New Roman
    MonospacedSans, // Lucida Console
    ProportionalSans, // Roboto
    Casual, // Comic Sans!
    Cursive, // Monotype Corsiva
    SmallCapitals // (Arial with font-variant: small-caps)
};

enum class EdgeType {
    None, HardShadow, Bevel, GlowOutline, SoftShadow
};

enum class TextAlignment {
    Left, Right, Center
};


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
    try {
        auto i = std::stoi(s);
        return static_cast<E>(i);
    } catch (...) {
        throw std::invalid_argument(
            "Invalid " + std::string(magic_enum::enum_type_name<E>())
            + " value: '" + s + "'");
    }
}

template<typename E>
std::string enumToIntString(E e) {
    return std::to_string(magic_enum::enum_integer(e));
}

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
    Color textEdgeColor = {0, 0, 0, 254};

    EdgeType textEdgeType = EdgeType::SoftShadow;
    FontStyle fontStyle = FontStyle::MonospacedSans;
    int fontSizePercent = 0;

    TextAlignment textAlignment = TextAlignment::Left;
    int horizontalMargin = 71;
    int verticalMargin = 0;
    int verticalSpacing = -1;
    int totalDisplayLines = 13;

    int maxCharsPerLine = 25;
    std::string usernameSeparator = ":";

    void saveToFile(const char *filename) const {
        CSimpleIniCaseA ini;
        ini.SetUnicode();
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
            const auto hf = textForegroundColor.toHexString();
            ini.SetValue(S, "textForegroundColor", hf.c_str(),
                         ";Hex color: #RGB, #RGBA, #RRGGBB or #RRGGBBAA");
        } {
            const auto hb = textBackgroundColor.toHexString();
            ini.SetValue(S, "textBackgroundColor", hb.c_str(),
                         ";Hex color: #RGB, #RGBA, #RRGGBB or #RRGGBBAA");
        } {
            const auto he = textEdgeColor.toHexString();
            ini.SetValue(S, "textEdgeColor", he.c_str(),
                         ";Hex color: #RGB, #RGBA, #RRGGBB or #RRGGBBAA");
        }

        // enums with generated option lists
        {
            const auto val = enumToString(textEdgeType);
            const auto cm = enumOptionsComment<EdgeType>();
            ini.SetValue(S, "textEdgeType", val.c_str(), cm.c_str());
        } {
            const auto val = enumToString(fontStyle);
            const auto cm = enumOptionsComment<FontStyle>();
            ini.SetValue(S, "fontStyle", val.c_str(), cm.c_str());
        }
        ini.SetLongValue(S, "fontSizePercent", fontSizePercent,
                         ";0–300 (virtual percent)"); {
            const auto val = enumToString(textAlignment);
            const auto cm = enumOptionsComment<TextAlignment>();
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

    template<typename T>
    bool loadFromFile(const T *filename) {
        CSimpleIniCaseA ini;
        ini.SetUnicode();
        ini.SetQuotes();
        if (ini.LoadFile(filename) < 0) return false;
        constexpr auto S = "General";

        // bools (fallback to existing defaults)
        textBold = ini.GetBoolValue(S, "bold", textBold);
        textItalic = ini.GetBoolValue(S, "italic", textItalic);
        textUnderline = ini.GetBoolValue(S, "underline", textUnderline);

        // colors
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

inline std::pair<std::string, std::vector<std::string> > wrapMessage(std::string username,
                                                                     std::string separator,
                                                                     const std::string &message,
                                                                     int maxWidth) {
    std::vector<std::string> lines;
    int availableSpace = maxWidth;
    if (utf8_length(username) > maxWidth) {
        username = utf8_substr(username, maxWidth);
        lines.push_back("");
    } else {
        availableSpace -= utf8_length(username);
    }

    if (utf8_length(separator) > availableSpace) {
        separator = utf8_substr(separator, availableSpace);
    }
    lines.push_back(separator);
    availableSpace -= utf8_length(separator);


    std::istringstream iss(message);
    std::vector<std::string> words;
    std::string word;
    bool firstWord = true;
    while (iss >> word) {
        bool bigWord = false;
        while (utf8_length(word) > maxWidth) {
            bigWord = true;
            if (availableSpace < 2) {
                availableSpace = maxWidth;
                lines.push_back(utf8_substr(word, availableSpace));
                firstWord = false;
            } else {
                if (!firstWord) {
                    lines.back() += " ";
                    availableSpace--;
                }
                lines.back() += utf8_substr(word, availableSpace);
                firstWord = false;
            }
            word = utf8_consume(word, availableSpace); // add split
            availableSpace = 0;
        }
        if (bigWord) {
            //if (utf8_length(word) < availableSpace) word += " ";
            lines.push_back(word);
            availableSpace = maxWidth - utf8_length(word);
            firstWord = false;
            continue;
        }
        if (utf8_length(word) < availableSpace) {
            if (!firstWord) {
                lines.back() += " ";
                availableSpace--;
            }
            lines.back() += word;
            availableSpace -= utf8_length(word);
        } else {
            //if (utf8_length(word) < maxWidth) word += " ";
            lines.push_back(word);
            availableSpace = maxWidth - utf8_length(word);
        }
        firstWord = false;
    }
    //    for (const auto& line :lines){
    //        assert(utf8_length(line)<=maxWidth);
    //    }
    return {username, lines};
}

inline std::vector<Batch> generateBatches(const std::vector<ChatMessage> &messages, const ChatParams &params) {
    std::vector<Batch> batches;
    std::deque<ChatLine> currentLines;
    for (const auto &msg: messages) {
        auto [username, wrapped] = wrapMessage(msg.user.name, params.usernameSeparator, msg.message,
                                               params.maxCharsPerLine);
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
    return batches;
}

inline std::string generateXML(const std::vector<Batch> &batches, const ChatParams &params) {
    using namespace tinyxml2;
    XMLDocument doc;

    std::map<Color, std::string> colors;
    colors[params.textForegroundColor] = "";
    // Not optimal, but I want to factor out messages
    for (const auto &m: batches) {
        for (const auto &l: m.lines) {
            if (l.user.has_value())colors[l.user->color] = "";
        }
    }

    XMLElement *root = doc.NewElement("timedtext");
    root->SetAttribute("format", "3");
    doc.InsertFirstChild(root);

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
        std::string textEdgeType = enumToIntString(params.textEdgeType);
        if (!textEdgeType.empty()) {
            pen->SetAttribute("ec", static_cast<std::string>(params.textEdgeColor).c_str());
            pen->SetAttribute("et", textEdgeType.c_str());
        }

        pen->SetAttribute("fs", enumToIntString(params.fontStyle).c_str());
        pen->SetAttribute("sz", std::to_string(params.fontSizePercent).c_str());
        head->InsertEndChild(pen);
        kv.second = std::to_string(penIndex);
        penIndex++;
    }

    // Create workspace element for whatever reason.
    XMLElement *ws = doc.NewElement("ws");
    ws->SetAttribute("id", "1"); // default workspace id
    ws->SetAttribute("ju", enumToIntString(params.textAlignment).c_str());
    head->InsertEndChild(ws);

    // Create window position (wp) elements.
    for (int i = 0; i < params.totalDisplayLines; ++i) {
        XMLElement *wp = doc.NewElement("wp");
        wp->SetAttribute("id", std::to_string(i).c_str());
        wp->SetAttribute("ap", "0"); // anchor point
        wp->SetAttribute("ah", std::to_string(params.horizontalMargin).c_str());
        wp->SetAttribute("av", std::to_string(i * params.verticalSpacing).c_str());
        head->InsertEndChild(wp);
    }
    // Zero-width space (ZWSP) as a UTF-8 string.
    auto defaultPen = colors[params.textForegroundColor];
    constexpr const char *ZWSP = "\xE2\x80\x8B";
    for (size_t batchIndex = 0; batchIndex + 1 < batches.size(); ++batchIndex) {
        const Batch &batch = batches[batchIndex];
        const Batch &nextBatch = batches[batchIndex + 1];
        if (params.verticalSpacing == -1) {
            XMLElement *pElem = doc.NewElement("p");
            pElem->SetAttribute("t", std::to_string(batch.time).c_str());
            int duration = nextBatch.time - batch.time;
            pElem->SetAttribute("d", std::to_string(duration).c_str());
            pElem->SetAttribute("wp", "0");
            pElem->SetAttribute("ws", "1");
            pElem->SetAttribute("p", defaultPen.c_str());
            pElem->LinkEndChild(doc.NewText(""));

            for (const auto &[idx, line]: batch.lines | std::ranges::views::enumerate) {
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
                pElem->LinkEndChild(doc.NewText("\n"));
            }
            body->InsertEndChild(pElem);
        } else {
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
    }

    XMLPrinter printer;
    doc.Print(&printer);
    return printer.CStr();
}


inline Color getRandomColor(const std::string &username) {
    std::vector<Color> defaultColors = {
        "#ff0000", "#0000ff", "#008000", "#b22222", "#ff7f50",
        "#9acd32", "#ff4500", "#2e8b57", "#daa520", "#d2691e",
        "#5f9ea0", "#1e90ff", "#ff69b4", "#8a2be2", "#00ff7f"
    };
    std::hash<std::string> hasher;
    return defaultColors[hasher(username) % defaultColors.size()];
}

// dumb and simple way to parse CSV
inline std::vector<ChatMessage> parseCSV(const std::filesystem::path &filename, int timeMultiplier) {
    std::vector<ChatMessage> messages;
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << "\n";
        std::exit(-1);
    }

    std::getline(file, line);
    if (line != "time,user_name,user_color,message") {
        std::cerr << "Error: Unexpected CSV header format.\n";
        std::exit(-1);
    }

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string field;
        ChatMessage msg;

        std::getline(ss, field, ',');
        msg.time = std::stoi(field) * timeMultiplier;

        std::getline(ss, msg.user.name, ',');

        std::getline(ss, field, ',');
        msg.user.color = field.empty() ? getRandomColor(msg.user.name) : Color(field);

        std::getline(ss, msg.message);

        if (msg.message.size() >= 2 &&
            msg.message.front() == '"' &&
            msg.message.back() == '"') {
            msg.message = msg.message.substr(1, msg.message.size() - 2);
        }

        messages.emplace_back(std::move(msg));
    }

    return messages;
}

inline float realFontScale(int yttFontSize) {
    return static_cast<float>((100.0 + (yttFontSize - 100.0) / 4.0) / 100.0);
}


inline double assFontSize(const int yttFontSize, const int assHeight) {
    return (realFontScale(yttFontSize) * 64.107) / 1440.0 * static_cast<double>(assHeight);
}

inline double assX(int yttX, const int yttFontSize, const int assWidth) {
    return (51.2821 + 24.5844 * yttX + 15.9109 * realFontScale(yttFontSize)) * assWidth / 2560.0;
}

inline double assY(const int yttY, const int yttFontSize, const int assHeight, const uint32_t line = 0) {
    //     double rFontScale = realFontScale(yttFontSize);
    //
    // //    double lC = 3.4442 * rFontScale * rFontScale * rFontScale
    // //                - 8.3489 * rFontScale * rFontScale
    // //                + 74.5018 * rFontScale - 1.6156;
    // //    double lC = 3.4441851202 * rFontScale * rFontScale * rFontScale
    // //                - 8.3488610000 * rFontScale * rFontScale
    // //                + 74.5018286601 * rFontScale - 1.6155972200;
    //     double lC = 5.1862772740 * rFontScale * rFontScale * rFontScale
    //                 - 14.1350956536 * rFontScale * rFontScale
    //                 + 80.4294668977 * rFontScale - 3.3718297322;
    const int yttY2 = yttY * yttY;
    const int yttY3 = yttY2 * yttY;
    const double fScale = realFontScale(yttFontSize);
    const double fScale2 = fScale * fScale;

    return ((0.0000036448 * fScale - 0.0000102298) * yttY3 +
            (0.0001228929 * fScale + 0.0006313481) * yttY2 +
            (-0.0299080260 * fScale + 13.8373127706) * yttY +
            (2.3156363636 * fScale2 - 2.1625454545 * fScale + 30.1938181818) +
            (2.095321207 * fScale2 + 64.752581827 * fScale + 1.158860604) * line
           ) * assHeight / 1440.0;
}


static std::string formatTime(uint64_t ms) {
    uint64_t total = ms / 10;
    uint64_t h = total / 360000;
    uint64_t m = (total / 6000) % 60;
    uint64_t s = (total / 100) % 60;
    uint64_t cs = total % 100;
    return std::format("{}:{:02}:{:02}.{:02}", h, m, s, cs);
}

static std::string escapeText(const std::string &raw) {
    std::string out;
    out.reserve(raw.size());
    for (char c: raw) {
        switch (c) {
            case '\\':
                out += "\\\\";
                break;
            case '{':
                out += "\\{";
                break;
            case '}':
                out += "\\}";
                break;
            case '\n':
                out += "\\n";
                break;
            //TODO Improve this
            default:
                out += c;
                break;
        }
    }
    return out;
}


inline std::string generateAss(const std::vector<Batch> &batches,
                               const ChatParams &chat_params,
                               int video_width, int video_height) {
    static constexpr std::string_view header =
            "\xEF\xBB\xBF" // BOM
            "[Script Info]\n";
    static constexpr std::string_view info =
            "\n; Script generated by Kam1k4dze's SubChat\n"
            "Title: SubChat preview\n"
            "ScriptType: v4.00+\n"
            "WrapStyle: 2\n"
            "ScaledBorderAndShadow: yes\n"
            "YCbCr Matrix: None\n";
    static constexpr std::string_view stylesHeader =
            "[V4+ Styles]\n"
            "Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, "
            "Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, "
            "Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding\n";
    static constexpr std::string_view eventsHeader =
            "[Events]\n"
            "Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n";

    static std::string ass;
    ass.clear();
    ass += header;
    ass += info;
    ass += std::format("PlayResX: {}\nPlayResY: {}\nLayoutResX: {}\nLayoutResY: {}\n\n",
                       video_width, video_height, video_width, video_height);
    ass += stylesHeader;
    float fontSize = assFontSize(chat_params.fontSizePercent, video_height);
    ass += std::format(
        "Style: Default,Lucida Console,{:.2f},&H00000000,&H000000FF,&H00000000,&H00000000,"
        "0,0,0,0,100,100,0,0,1,0,2.5,7,0,0,0,1\n\n",
        fontSize
    );
    ass += eventsHeader;
    double posX = assX(chat_params.horizontalMargin, chat_params.fontSizePercent, video_width);
    size_t maxLines = chat_params.totalDisplayLines;
    std::vector<double> posY(maxLines);
    for (size_t idx = 0; idx < maxLines; ++idx) {
        posY[idx] = (chat_params.verticalSpacing < 0)
                        ? assY(chat_params.verticalMargin, chat_params.fontSizePercent, video_height, idx)
                        : assY(chat_params.verticalMargin + chat_params.verticalSpacing * idx,
                               chat_params.fontSizePercent, video_height);
    }

    for (size_t i = 0; i + 1 < batches.size(); ++i) {
        const auto &curr = batches[i];
        const auto &next = batches[i + 1];
        auto start = formatTime(curr.time);
        auto end = formatTime(next.time);

        for (size_t idx = 0; idx < curr.lines.size(); ++idx) {
            const auto &line = curr.lines[idx];
            std::format_to(std::back_inserter(ass),
                           "Dialogue: 0,{},{},Default,,0,0,0,,{{\\pos({:.3f},{:.3f})}}",
                           start, end, posX, posY[idx]
            );

            if (line.user) {
                ass += line.user->color.toAssColor();
                ass += escapeText(line.user->name);
            }
            ass += chat_params.textForegroundColor.toAssColor();
            ass += escapeText(line.text);
            ass += '\n';
        }
    }

    return ass;
}
