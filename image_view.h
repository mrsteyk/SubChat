#pragma once

#include "imgui.h"
#include "imgui_internal.h"
#include <cmath>

// Global font pointer.
static ImFont *g_font = nullptr;

// Structure that holds the state of the interactive text overlay.
struct InteractiveTextOverlay {
    // Relative text position, in [0, 1] relative to the image’s top-left.
    float relX = 0.5f;
    float relY = 0.5f;
    // A factor applied to the base font size.
    float fontSizeFactor = 50.0f;
};

// Preload the font with a large base size (to allow scaling).
inline void PreloadFont(const char *fontPath = "../fonts/lucon.ttf", float baseSize = 256.0f) {
    ImGuiIO &io = ImGui::GetIO();
    g_font = io.Fonts->AddFontFromFileTTF(fontPath, baseSize, nullptr, io.Fonts->GetGlyphRangesCyrillic());
}

// Renders an interactive image window that displays an image and overlays draggable,
// resizable text using relative coordinates.
inline void ShowInteractiveImage(
        ImTextureID texture, int texWidth, int texHeight,
        bool *p_open,
        InteractiveTextOverlay *overlay) {
    // Define margins as a fraction of the image dimensions and handle size.
    constexpr float marginX = 0.05f;
    constexpr float marginY = 0.05f;
    constexpr float handleSize = 8.0f; // Resize handle size.

    if (!ImGui::Begin("Preview", p_open)) {
        ImGui::End();
        return;
    }

    // Determine the available region and scale the image to fit while preserving aspect ratio.
    ImVec2 availSize = ImGui::GetContentRegionAvail();
    float scale = ImMin(availSize.x / texWidth, availSize.y / texHeight);
    ImVec2 imageSize(texWidth * scale, texHeight * scale);

    // Center the image in the available area.
    ImVec2 offset((availSize.x - imageSize.x) * 0.5f, (availSize.y - imageSize.y) * 0.5f);
    ImGui::SetCursorPos(ImGui::GetCursorPos() + offset);

    // Draw the image.
    ImGui::Image(texture, imageSize);
    // Get the screen-space top-left of the image.
    ImVec2 imgPos = ImGui::GetItemRectMin();

    // Compute the text position and desired font size.
    ImVec2 textPos(imgPos.x + overlay->relX * imageSize.x, imgPos.y + overlay->relY * imageSize.y);
    float desiredFontSize = overlay->fontSizeFactor * scale;

    // Prepare text drawing.
    const char *text = "Sample Text";
    ImGui::PushFont(g_font);
    ImVec2 baseTextSize = ImGui::CalcTextSize(text);
    float textScale = desiredFontSize / g_font->FontSize;
    ImVec2 textSize(baseTextSize.x * textScale, baseTextSize.y * textScale);

    // Render the text.
    ImGui::GetWindowDrawList()->AddText(g_font, desiredFontSize, textPos, IM_COL32(255, 255, 255, 255), text);
    ImGui::PopFont();

    // --- Handle Text Dragging ---
    ImGui::SetCursorScreenPos(textPos);
    ImGui::InvisibleButton("drag_text", ImVec2(textSize.x - handleSize, textSize.y - handleSize));
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) {
        ImVec2 delta = ImGui::GetIO().MouseDelta;
        overlay->relX += delta.x / imageSize.x;
        overlay->relY += delta.y / imageSize.y;
    }

    // --- Clamp Text Position Within Margins ---
    textPos = ImVec2(imgPos.x + overlay->relX * imageSize.x, imgPos.y + overlay->relY * imageSize.y);
    float minX = imgPos.x + marginX * imageSize.x;
    float minY = imgPos.y + marginY * imageSize.y;
    float maxX = imgPos.x + imageSize.x - marginX * imageSize.x - textSize.x;
    float maxY = imgPos.y + imageSize.y - marginY * imageSize.y - textSize.y;
    float clampedX = ImClamp(textPos.x, minX, maxX);
    float clampedY = ImClamp(textPos.y, minY, maxY);
    overlay->relX = (clampedX - imgPos.x) / imageSize.x;
    overlay->relY = (clampedY - imgPos.y) / imageSize.y;
    textPos = ImVec2(imgPos.x + overlay->relX * imageSize.x, imgPos.y + overlay->relY * imageSize.y);

    // --- Handle Text Resizing ---
    ImVec2 resizeHandlePos(textPos.x + textSize.x - handleSize, textPos.y + textSize.y - handleSize);
    ImGui::SetCursorScreenPos(resizeHandlePos);
    ImGui::InvisibleButton("resize_text", ImVec2(handleSize, handleSize));
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) {
        float delta =
                ImGui::GetIO().MouseDelta.x * 3; // Use horizontal drag to adjust size. //TODO: do something smarter
        float newFontFactor = overlay->fontSizeFactor + delta * 0.1f;
        // Calculate maximum allowed font size factor to keep the text within margins.
        float maxFontFactorX = (imageSize.x - marginX * imageSize.x - (textPos.x - imgPos.x)) * g_font->FontSize / (
                baseTextSize.x * scale);
        float maxFontFactorY = (imageSize.y - marginY * imageSize.y - (textPos.y - imgPos.y)) * g_font->FontSize / (
                baseTextSize.y * scale);
        overlay->fontSizeFactor = ImClamp(newFontFactor, 1.0f, ImMin(maxFontFactorX, maxFontFactorY));

        // Recompute desired font size and text size.
        desiredFontSize = overlay->fontSizeFactor * scale;
        textScale = desiredFontSize / g_font->FontSize;
        textSize = ImVec2(baseTextSize.x * textScale, baseTextSize.y * textScale);
    }

    // Draw visual indicators: resize handle and bounding box.
    ImGui::GetWindowDrawList()->AddRectFilled(resizeHandlePos,
                                              ImVec2(resizeHandlePos.x + handleSize, resizeHandlePos.y + handleSize),
                                              IM_COL32(0, 255, 0, 255));
    ImGui::GetWindowDrawList()->AddRect(textPos,
                                        ImVec2(textPos.x + textSize.x, textPos.y + textSize.y),
                                        IM_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);

    ImGui::End();
}
