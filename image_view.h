#pragma once

#include "imgui.h"
#include "imgui_internal.h"

inline void ShowImageWindow(ImTextureID input_texture, int texture_width, int texture_height, bool *p_open) {
    IM_ASSERT(ImGui::GetCurrentContext() != nullptr && "Missing Dear ImGui context. Refer to examples app!");

    ImGuiWindowFlags window_flags = 0;
    if (!ImGui::Begin("Preview", p_open, window_flags)) {
        ImGui::End();
        return;
    }

    // Get the available content region which updates when the window is resized.
    ImVec2 availSize = ImGui::GetContentRegionAvail();

    // Calculate scale to fit the image while preserving the aspect ratio.
    float scale = ImMin(availSize.x / texture_width, availSize.y / texture_height);
    ImVec2 imageSize(texture_width * scale, texture_height * scale);

    // Calculate offset to center the image within the available region.
    ImVec2 offset((availSize.x - imageSize.x) * 0.5f, (availSize.y - imageSize.y) * 0.5f);
    ImGui::SetCursorPos(ImGui::GetCursorPos() + offset);

    // Draw the image using the computed size.
    ImGui::Image(input_texture, imageSize);

    ImGui::End();
}
