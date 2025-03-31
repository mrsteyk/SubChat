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
    float pixelsMultY = 0.96;

    float realFontSize(int height) const {  // Made const for correctness
        return (100.0f + (virtualFontSize - 100.0f) / 4.0f) / 100.0f * (height / scaleConstantPerHeight);
    }

    float realX() const {
        return ((virtualX * 0.96) + 2.5f) / 100.0f;
    }

    float realY(int N=0) const {
        return (((virtualY+N*virtualDistanceBetweenLines) * 0.96f) + 2.15f) / 100.0f;
    }


    int charsPerLine = 10;
    int LinesCount = 10;
    int virtualDistanceBetweenLines = 4;
    // Added customizable text
    std::string text = "kamikadze1:";
    std::vector<std::string> texts = {"kam1k4dze1:ABCDEFGHIJKL","MNOPQRSTUVWXYZ","kam1k4dze1:АБВГ","MNOPQRSTUVWXYZ","electricalneo:hello","afterschoolchan:Why do","slop today?",
                                      "tonitch:hello :D"};
};


// Preload the font with a large base size (to allow scaling).
inline bool PreloadFont(const char *fontPath = "../fonts/lucon.ttf", float baseSize = 256.0f) {
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
    ImVec2 textPos(imgPos.x + overlay->realX() * imageSize.x, imgPos.y + overlay->realY() * imageSize.y);
    float desiredFontSize = overlay->realFontSize(texHeight) * scale;  // Fixed: use the renamed method

    // Prepare text drawing.
    const char *text = overlay->text.c_str();  // Use the overlay's text field
    ImGui::PushFont(g_font);
    ImVec2 baseTextSize = ImGui::CalcTextSize(text);
    float textScale = desiredFontSize / g_font->FontSize;
    ImVec2 textSize(baseTextSize.x * textScale, baseTextSize.y * textScale);

    // Render the text.
    for (int i = 0; i < 8; i++) {
        ImGui::GetWindowDrawList()->AddText(g_font, desiredFontSize, {imgPos.x + overlay->realX() * imageSize.x, imgPos.y + overlay->realY(i) * imageSize.y}, IM_COL32(255, 255, 255, 255), overlay->texts[i].c_str());
    }
    ImGui::PopFont();

    ImGui::GetWindowDrawList()->AddRect(
            textPos,
            ImVec2(textPos.x + textSize.x, textPos.y + textSize.y),
            IM_COL32(255, 255, 0, 255),
            0.0f,  // rounding
            0,     // flags
            2.0f   // thickness
    );

    ImGui::End();
}