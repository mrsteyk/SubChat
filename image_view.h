#pragma once

#include "imgui.h"
#include "imgui_internal.h"
#include <cmath>
#include <string>
#include <vector>
#include "ytt_generator.h"

// Global font pointer.
static ImFont *g_font = nullptr;


// Structure holding the calculated metrics.
struct SubtitleMetrics {
    int maxCharsInLine;            // Maximum characters per line.
    int maxLines;                  // Maximum number of lines that fit in the box.
    int virtualFontSize;           // The user-selected virtual font size (in percentage).
    int effectiveFirstLineCoordX;  // Computed effective first-line X coordinate (0 = left, 100 = right).
    int effectiveFirstLineCoordY;  // Computed effective first-line Y coordinate (0 = top, 100 = bottom).
    int relativeEffectiveDistanceBetweenLines; // Line spacing as a percent of the box height.
};

// Structure that holds the state of the interactive text overlay.
struct InteractiveTextOverlay {

    int virtualX = 0; // 0-100
    int virtualY = 0; //0-100
    // A factor applied to the base font size.
    int virtualFontSize = 0;  // 0-300
    float scaleConstantPerHeight = 22.5f;

    float realFontSize(int height) const {  // Made const for correctness
        return (100.0f + (virtualFontSize - 100.0f) / 4.0f) / 100.0f * (height / scaleConstantPerHeight);
    }

    float realX() const {
        return ((virtualX * 0.96) + 2.5f) / 100.0f;
    }

    float realY(int N = 0) const {
        return (((virtualY + N * virtualDistanceBetweenLines) * 0.96f) + 2.15f) / 100.0f;
    }


    int charsPerLine = 10;
    int LinesCount = 2;
    int virtualDistanceBetweenLines = 4;
    // Added customizable text
    std::string separator = ":";
    std::vector<std::pair<std::string, std::string>> messages = {
            {"Sirius",        "Lorem ipsum dolor sit amet, consectetur adipiscing elit."},
            {"Betelgeuse",    "Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua."},
            {"Vega",          "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris."},
            {"Rigel",         "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore."},
            {"Antares",       "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia."},
            {"Arcturus",      "Curabitur pretium tincidunt lacus. Nulla gravida orci a odio."},
            {"Aldebaran",     "Pellentesque habitant morbi tristique senectus et netus et malesuada fames."},
            {"Procyon",       "Maecenas sed diam eget risus varius blandit sit amet non magna."},
            {"Capella",       "Cras mattis consectetur purus sit amet fermentum."},
            {"Altair",        "Aenean lacinia bibendum nulla sed consectetur."},
            {"Pollux",        "Vestibulum id ligula porta felis euismod semper."},
            {"Spica",         "Praesent commodo cursus magna, vel scelerisque nisl consectetur et."},
            {"Deneb",         "Nullam quis risus eget urna mollis ornare vel eu leo."},
            {"Canopus",       "Etiam porta sem malesuada magna mollis euismod."},
            {"Fomalhaut",     "Donec ullamcorper nulla non metus auctor fringilla."},
            {"Bellatrix",     "Aenean eu leo quam. Pellentesque ornare sem lacinia quam venenatis."},
            {"Achernar",      "Integer posuere erat a ante venenatis dapibus posuere velit aliquet."},
            {"Regulus",       "Sed posuere consectetur est at lobortis."},
            {"Castor",        "Curabitur blandit tempus porttitor."},
            {"Mira",          "Morbi leo risus, porta ac consectetur ac, vestibulum at eros."},
            {"Alpheratz",     "Fusce dapibus, tellus ac cursus commodo, tortor mauris condimentum nibh."},
            {"Shaula",        "Donec id elit non mi porta gravida at eget metus."},
            {"Zubenelgenubi", "Vivamus sagittis lacus vel augue laoreet rutrum faucibus dolor auctor."},
            {"Sadr",          "Integer nec odio. Praesent libero. Sed cursus ante dapibus diam."},
            {"Nunki",         "Suspendisse potenti. Morbi fringilla convallis sapien."},
            {"Hadar",         "Curabitur tortor. Pellentesque nibh."},
            {"Mintaka",       "Aenean quam. In scelerisque sem at dolor."},
            {"Alnilam",       "Maecenas mattis. Sed convallis tristique sem."},
            {"Wezen",         "Proin ut ligula vel nunc egestas porttitor."},
            {"Naos",          "Aliquam erat volutpat. Nulla facilisi."},
            {"Rasalhague",    "Nam dui ligula, fringilla a, euismod sodales, sollicitudin vel, wisi."},
            {"Markab",        "Nulla facilisi. Aenean nec eros."},
            {"Diphda",        "Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere."},
            {"Enif",          "Duis cursus, mi quis viverra ornare, eros dolor interdum nulla."},
            {"Unukalhai",     "Fusce lacinia arcu et nulla."},
            {"Gienah",        "Suspendisse in justo eu magna luctus suscipit."},
            {"Algol",         "Curabitur at lacus ac velit ornare lobortis."},
            {"Menkar",        "Nullam nulla eros, ultricies sit amet, nonummy id, imperdiet feugiat."},
            {"Saiph",         "Phasellus viverra nulla ut metus varius laoreet."},
            {"Izar",          "Quisque rutrum. Aenean imperdiet."},
            {"Alhena",        "Etiam ultricies nisi vel augue."},
            {"Menkalinan",    "Curabitur ullamcorper ultricies nisi."},
            {"Avior",         "Donec mollis hendrerit risus."},
            {"Peacock",       "Praesent egestas tristique nibh."},
            {"Hamal",         "Curabitur blandit mollis lacus."},
            {"Eltanin",       "Nam adipiscing. Vestibulum eu odio."},
            {"Sadalmelik",    "Curabitur vestibulum aliquam leo."},
            {"Ankaa",         "Pellentesque habitant morbi tristique senectus et netus et malesuada."},
            {"Tarazed",       "Nunc nonummy metus. Vestibulum volutpat pretium libero."},
            {"Caph",          "Duis leo. Sed fringilla mauris sit amet nibh."},
            {"Alsephina",     "Donec sodales sagittis magna."},
            {"Sabik",         "Fusce fermentum odio nec arcu."}
    };
    std::vector<std::pair<std::string, std::string>> preview;
    bool revalidatePreview = true;

    void generatePreview() {
        preview.clear();
        for (auto [name, text]: messages) {
            auto wrapped = wrapMessage(name, separator, text, charsPerLine);
            if (wrapped.empty()) {
                continue;
            }
            if (preview.size() < LinesCount) {
                preview.emplace_back(name, wrapped[0]);
            } else {
                break;
            }
            for (size_t i = 1; i < wrapped.size() && preview.size() < LinesCount; ++i) {
                preview.emplace_back("", wrapped[i]);
            }
        }
        revalidatePreview = false;
    }
};

bool PreloadFont(const char *fontPath = "../fonts/lucon.ttf", float baseSize = 256.0f) {
    ImGuiIO &io = ImGui::GetIO();

    g_font = io.Fonts->AddFontFromFileTTF(fontPath, baseSize, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    return g_font != nullptr;  // Return success/failure
}

// Renders an interactive image window that displays an image and overlays draggable,
// resizable text using relative coordinates.
inline void ShowInteractiveImage(
        ImTextureID texture, int texWidth, int texHeight,
        bool *p_open,
        InteractiveTextOverlay *overlay) {

    // Validate input parameters
    if (!texture || !overlay || texWidth <= 0 || texHeight <= 0) {
        return;
    }

    // Define margins as a fraction of the image dimensions and handle size.
    constexpr float marginX = 0.0f;
    constexpr float marginY = 0.0f;
    constexpr float handleSize = 20.0f; // Resize handle size.

    if (!ImGui::Begin("Preview", p_open)) {
        ImGui::End();
        return;
    }
    if (overlay->revalidatePreview) overlay->generatePreview();

    // Determine the available region and scale the image to fit while preserving aspect ratio.
    ImVec2 availSize = ImGui::GetContentRegionAvail();
    float scale = ImMin(availSize.x / static_cast<float>(texWidth), availSize.y / static_cast<float>(texHeight));
    ImVec2 imageSize(texWidth * scale, texHeight * scale);

    // Center the image in the available area.
    ImVec2 offset((availSize.x - imageSize.x) * 0.5f, (availSize.y - imageSize.y) * 0.5f);
    ImGui::SetCursorPos(ImGui::GetCursorPos() + offset);

    // Draw the image.
    ImGui::Image(texture, imageSize);
    // Get the screen-space top-left of the image.
    ImVec2 imgPos = ImGui::GetItemRectMin();

    // Check if font is loaded
    if (!g_font) {
        ImGui::End();
        return;
    }

    // Compute the text position and desired font size.
    float desiredFontSize = overlay->realFontSize(texHeight) * scale;  // Fixed: use the renamed method

    // Prepare text drawing.
//    const char *text = overlay->text.c_str();  // Use the overlay's text field
    ImGui::PushFont(g_font);

    float textScale = desiredFontSize / g_font->FontSize;
    ImDrawList *drawList = ImGui::GetWindowDrawList();
    // Render the text.
    ImVec2 letterSize = ImGui::CalcTextSize("M");
    float boxWidth = letterSize.x * textScale;
    ImVec2 startPos;
    ImVec2 endPos;
    for (int i = 0; i < overlay->preview.size(); i++) {
        ImVec2 textPos(imgPos.x + overlay->realX() * imageSize.x, imgPos.y + overlay->realY(i) * imageSize.y);

        if (i == 0) {
            startPos = textPos;
            endPos.x = startPos.x + overlay->charsPerLine * boxWidth;
        }

        const char *firstText = overlay->preview[i].first.c_str();
        ImVec2 firstBaseTextSize = ImGui::CalcTextSize(firstText);

        ImVec2 firstTextSize(firstBaseTextSize.x * textScale, firstBaseTextSize.y * textScale);


        const char *secondText = overlay->preview[i].second.c_str();

        ImVec2 secondBaseTextSize = ImGui::CalcTextSize(secondText);

        ImVec2 secondTextSize(secondBaseTextSize.x * textScale, secondBaseTextSize.y * textScale);


        const Color c = getRandomColor(firstText);
        drawList->AddText(g_font, desiredFontSize, textPos,
                          IM_COL32(c.r, c.g, c.b, 254),
                          firstText);


        textPos.x += firstTextSize.x; // Move position for second text
        endPos.y = textPos.y + secondTextSize.y;

        drawList->AddText(g_font, desiredFontSize, textPos,
                          IM_COL32(254, 254, 254, 254),
                          secondText);

    }
    ImGui::PopFont();

    ImGui::GetWindowDrawList()->AddRect(
            startPos,
            endPos,
            IM_COL32(255, 255, 0, 255),
            0.0f,  // rounding
            0,     // flags
            2.0f   // thickness
    );

    ImGui::End();
}