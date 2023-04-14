// UI libs
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <SDL3/SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif

#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <map>
#include "linked_list.tpp"

using namespace std;

class Vec2: public ImVec2 {
public:
    Vec2() : ImVec2() {}
    Vec2(ImVec2 vec) : ImVec2(vec) {}
    Vec2(float x, float y): ImVec2(x, y) {}

    Vec2 operator+(const Vec2 &b) const { return {x + b.x, y + b.y}; }
    Vec2& operator+=(const Vec2 &b) {
        x += b.x; y += b.y;
        return *this;
    }
    Vec2 operator-() const { return {-x, -y}; }
    Vec2 operator-(const Vec2 &b) const { return {x - b.x, y - b.y}; }

    Vec2 operator*(const float b) const { return {x * b, y * b}; }
    Vec2 &operator*=(const float b) {
        x *= b; y *= b;
        return *this;
    }
    Vec2 operator/(float b) const { return {x / b, y / b}; }

    float abs() const { return sqrt(x * x + y * y); }
    Vec2 norm() const { return *this / abs(); }

    string to_string() const
    {
        stringstream ss;
        ss << "v(" << x << ", " << y << ")";
        return ss.str();
    }
};

struct Field {
    const float cell_size;
    vector<Vec2> points;
    vector<Vec2> speeds;
    map<int, map<int, linked_list_root<int>>> field;

    Field(float cell_size): cell_size(cell_size), points(), speeds() {}
    
    static float sign(float a){
        if(a > 0) return 1;
        if(a < 0) return -1;
        return 0;
    }

    static int min(int a, int b) { return a > b ? b : a; }

    pair<int, int> get_field_index(const Vec2& point) const {
        return {
            static_cast<int>(point.x / cell_size),
            static_cast<int>(point.y / cell_size)
        };
    }

    void remove_from_field(const pair<int, int>& field_index, int point_index) {
        const Vec2& point = points[point_index];
        field[field_index.first][field_index.second].remove_val(point_index);
    }

    void add_to_field(const pair<int, int>& field_index, int point_index) {
        auto &v = field[field_index.first][field_index.second];
        field[field_index.first][field_index.second].push_back(point_index);
    }

    void add_points(vector<Vec2> new_points) {
        const int prev_size = points.size();
        points.resize(points.size() + new_points.size());
        speeds.resize(points.size());
        copy(new_points.begin(), new_points.end(), points.begin() + prev_size);
        for(int i = 0; i < new_points.size(); i++){
            const auto field_index = get_field_index(points[i + prev_size]);
            add_to_field(field_index, i + prev_size);
        }
    }

    void do_tick(float dt){
        vector<Vec2> forces(points.size());
        for(int i = 0; i < points.size(); i++){
            Vec2 &point = points[i];
            const auto field_index = get_field_index(point);
            for (int dx = -1; dx <= 1; dx++)
            for (int dy = -1; dy <= 1; dy++) {
                const auto& list = field[field_index.first + dx][field_index.second + dy];
                if(list.size == 0) continue;
                auto buff = list.first;
                for(; buff != nullptr; buff = buff->next){
                    if (buff->val == i) continue;
                    const Vec2 &point_b = points[buff->val];
                    const Vec2 delta = point_b - point;
                    const float delta_abs = delta.abs() - cell_size / 2;
                    forces[i] += delta.norm() * (pow(delta_abs, 3)) / 1e3;
                }
            }
        }

        for(int i = 0; i < points.size(); i++){
            Vec2 &point = points[i];
            Vec2 &speed = speeds[i];
            const auto field_index = get_field_index(point);
            speed += forces[i] * dt;
            speed *= 0.95;

            const static float max_speed = 1e3;
            const float abs_speed = speed.abs();
            if (abs_speed > max_speed) speed *= max_speed / abs_speed;
            if (abs_speed < 0.5) speed = {0, 0};

            point += speed * dt;

            const auto new_field_index = get_field_index(point);
            if(field_index != new_field_index){
                remove_from_field(field_index, i);
                add_to_field(new_field_index, i);
            }
        }
    }

    void draw(){
        const float R = 18;
        static int selected = -1;

        ImGui::Begin("Field");
        ImGui::SetWindowSize({500, 500}, ImGuiCond_Appearing);
        const Vec2 p = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("canvas", ImGui::GetContentRegionAvail(), ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

        const Vec2 mouse = ImGui::GetMousePos();
        if (selected == -1 && ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            for (int i = 0; i < points.size() && selected == -1; i++)
                if ((points[i] + p - mouse).abs() <= R)
                selected = i;
        }
        if (selected != -1 && ImGui::IsMouseDown(ImGuiMouseButton_Left))
            points[selected] = mouse - p;
        else if (selected != -1 && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
            selected = -1;

        const ImU32 col = ImColor(1.0f, 1.0f, 0.4f, 1.0f);
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        for (ImVec2 point : points)
        {
            draw_list->AddNgonFilled(p + point, R, col, 36);
        }

        ImGui::End();
    }
}
;
static double time()
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() / 1000.;
}

// MSVC compiler main() error workaround
// https://stackoverflow.com/a/58819006/8302811
#ifdef _WIN32
int main(int, char **);
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine, int nCmdShow)
{
    return main(__argc, __argv);
}
#endif

// Main code
int main(int, char **)
{
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMEPAD) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char *glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char *glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char *glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Enable native IME.
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_Window *window = SDL_CreateWindow("Dear ImGui SDL3+OpenGL3 example", 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync


    ////// IMGUI PART
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    Field field(200);
    field.add_points({
        {100, 100}, {200, 100}, {300, 100},
        {100, 200}, {200, 200}, {300, 200}
    });
    double prev_time = time();

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                done = true;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        double cur_time = time();
        field.do_tick(cur_time - prev_time);
        prev_time = cur_time;
        field.draw();

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
