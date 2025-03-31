#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif // IMGUI_DEFINE_MATH_OPERATORS

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GL/glew.h>
#include <cstdio>

#define GL_SILENCE_DEPRECATION

#include <GLFW/glfw3.h>

#include "image_view.h"
#include "misc/cpp/imgui_stdlib.h"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
#include "ytt_generator.h"

// float dpi_scale = 1;
// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromMemory(const void *data, size_t data_size, GLuint *out_texture, int *out_width, int *out_height) {
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char *image_data = stbi_load_from_memory(static_cast<const unsigned char *>(data),
                                                      static_cast<int>(data_size), &image_width,
                                                      &image_height, nullptr, 4);
    if (image_data == nullptr)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload pixels into texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}

// Open and read a file, then forward to LoadTextureFromMemory()
bool LoadTextureFromFile(const char *file_name, GLuint *out_texture, int *out_width, int *out_height) {
    FILE *f = fopen(file_name, "rb");
    if (f == nullptr)
        return false;
    fseek(f, 0, SEEK_END);
    auto file_size = static_cast<size_t>(ftell(f));
    if (file_size == -1)
        return false;
    fseek(f, 0, SEEK_SET);
    void *file_data = IM_ALLOC(file_size);
    fread(file_data, 1, file_size, f);
    fclose(f);
    bool ret = LoadTextureFromMemory(file_data, file_size, out_texture, out_width, out_height);
    IM_FREE(file_data);
    return ret;
}

static void glfw_error_callback(int error, const char *description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static float GetDPIScale(GLFWwindow *window) {
    float xscale, yscale;
    glfwGetWindowContentScale(window, &xscale, &yscale);
    return xscale; // Assuming uniform scaling for simplicity
}

void LoadFonts(float dpi_scale) {
    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->Clear();
    io.Fonts->AddFontFromFileTTF("../fonts/lucon.ttf", 16.0f * dpi_scale, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    io.Fonts->Build();

}

void ApplyDPIStyles(float dpi_scale) {
    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(dpi_scale);
}

void ShowDockSpace() {
    // Create a full-screen window for our dockspace.
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking |
                                    ImGuiWindowFlags_NoTitleBar |
                                    ImGuiWindowFlags_NoCollapse |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus |
                                    ImGuiWindowFlags_NoNavFocus;
    ImGui::Begin("MainDockSpace", nullptr, window_flags);
    // The DockSpace() call must be inside this window.
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), 0);
    ImGui::End();
}

int main(int, char **) {
    test();
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;
    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow *window = glfwCreateWindow(1280, 720, "Dear ImGui Docking Example", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    float dpi_scale = GetDPIScale(window);
    LoadFonts(dpi_scale);
    ApplyDPIStyles(dpi_scale);

    bool show_image_window = true;
    bool show_hello_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    int my_image_width = 0;
    int my_image_height = 0;
    GLuint my_image_texture = 0;
    bool ret = LoadTextureFromFile("../sample_t.png", &my_image_texture, &my_image_width, &my_image_height);
    IM_ASSERT(ret);

    // Variables to store our fixed dock node IDs.
    bool first_time_layout = true;
    static ImGuiID imageDockID = 0;
    static ImGuiID helloDockID = 0;
    PreloadFont();
    InteractiveTextOverlay text_overlay{};
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        float new_dpi_scale = GetDPIScale(window);
        if (new_dpi_scale != dpi_scale) {
            printf("DPI scale: %f\n", new_dpi_scale);
            dpi_scale = new_dpi_scale;
            LoadFonts(dpi_scale);
            ApplyDPIStyles(dpi_scale);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Force the windows to always dock in their designated nodes.
        if (show_image_window) {
            //ImGui::SetNextWindowDockID(imageDockID, ImGuiCond_Always);
            ShowInteractiveImage(my_image_texture, my_image_width, my_image_height, &show_image_window, &text_overlay);
        }

        if (show_hello_window) {
            //ImGui::SetNextWindowDockID(helloDockID, ImGuiCond_Always);
            ImGui::Begin("Привет, Мир!");
            ImGui::Text("This is some useful text.");
            ImGui::SliderInt("virtualFontSize", &text_overlay.virtualFontSize, 0, 300);
            ImGui::SliderInt("font X", &text_overlay.virtualX, 0, 100);
            ImGui::SliderInt("font Y", &text_overlay.virtualY, 0, 100);
            ImGui::InputFloat("pixelsMultY", &text_overlay.pixelsMultY,0.01);
            ImGui::InputText("Text", &text_overlay.text);
            // (Add any additional hello world UI elements here)
            if (ImGui::Button("Open")) {
                show_image_window = true;
            }
            ImGui::End();
        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w,
                     clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
