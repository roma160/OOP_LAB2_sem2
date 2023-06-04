#include "main.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

#include <stdio.h>

#include "utils.tpp"
#include "field.h"
#include "algorithms_window.h"

#ifndef EMSCRIPTEN_CODE
#include "sparse_graph_view.h"
#include "control_window.h"
#endif

using namespace std;

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
// #include "../libs/emscripten/emscripten_mainloop_stub.h"
#include <emscripten.h>
#include <functional>
static std::function<void()>            MainLoopForEmscriptenP;
static void MainLoopForEmscripten()     { MainLoopForEmscriptenP(); }
#define EMSCRIPTEN_MAINLOOP_BEGIN       MainLoopForEmscriptenP = [&]()
#define EMSCRIPTEN_MAINLOOP_END         ; emscripten_set_main_loop(MainLoopForEmscripten, 0, true)
#else
#define EMSCRIPTEN_MAINLOOP_BEGIN
#define EMSCRIPTEN_MAINLOOP_END
#endif

// MSVC compiler main() error workaround
// https://stackoverflow.com/a/58819006/8302811
// #ifdef _WIN32
// int main(int, char **);
// int APIENTRY WinMain(HINSTANCE hInstance,
//                      HINSTANCE hPrevInstance,
//                      LPSTR lpCmdLine, int nCmdShow)
// {
//     return main(__argc, __argv);
// }
// #endif


bool is_any_scrollable_hovered = false;
void add_scrollable_data(bool is_scrollable_hovered) {
	is_any_scrollable_hovered |= is_any_scrollable_hovered;
}


// Main code
int main(int, char **)
{
    // Disabling keyboards events for browser
    // https://github.com/emscripten-core/emscripten/issues/3621
    // https://github.com/emscripten-ports/SDL2/pull/10#issuecomment-87466380
    // SDL_EventState(SDL_TEXTINPUT, SDL_DISABLE);
    // SDL_EventState(SDL_KEYDOWN, SDL_DISABLE);
    // SDL_EventState(SDL_KEYUP, SDL_DISABLE);

	// ImGui

    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
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

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window *window = SDL_CreateWindow("Dear ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
	// https://github.com/emscripten-ports/SDL2/issues/128
	SDL_SetEventFilter([](void* userdata, SDL_Event *event){
		// Filtering the keyboard events, and handling one only
		// when a text input is active
		const ImGuiIO* io = static_cast<ImGuiIO*>(userdata);
		switch(event->type) {
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				switch(event->key.keysym.scancode) {
					case SDL_SCANCODE_SPACE:
					case SDL_SCANCODE_BACKSPACE:
					case SDL_SCANCODE_RETURN:
					case SDL_SCANCODE_TAB:
					case SDL_SCANCODE_RIGHT:
					case SDL_SCANCODE_LEFT:
					case SDL_SCANCODE_DOWN:
					case SDL_SCANCODE_UP:
					case SDL_SCANCODE_HOME:
					case SDL_SCANCODE_END:
						return 1;
				}
				if(SDL_GetModState() & KMOD_CTRL){
					switch(event->key.keysym.sym) {
						case SDLK_c: case SDLK_v:
						case SDLK_a: case SDLK_x:
						case SDLK_z: case SDLK_y:
							return 1;
					}
				}
				return 0;
			case SDL_TEXTINPUT:
				if(!io->WantTextInput)
					return 0;
			// case SDL_MOUSEWHEEL:
			// 	#ifdef EMSCRIPTEN_CODE
			// 	if(!any_scrollbar_active)
			// 		emscripten_log(EM_LOG_CONSOLE, "dsdsd");

			// 	#endif

			// 	if(!any_scrollbar_active)
			// 		return 0;
			default:
				// This case is here just to disable compile warnings
				return 1;
			//ImGui::get
		}
		return 1;
	}, &io);

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Styling the window
    #ifndef EMSCRIPTEN_CODE
    const ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, .00f);
    #else
    const ImVec4 clear_color = ImVec4(1.f, 1.f, 1.f, 1.f);
	ImGui::StyleColorsLight();

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowBorderSize = 0;
	style.FrameBorderSize = 1.f;
    #endif

    // GRAPH DATA
    Field field(200);
    field.add_graph(Graph(
        {6, vector<Graph::Connection>(6, true)}
    ));

    #ifndef EMSCRIPTEN_CODE
    SparseGraphView sparseGraphView;
    //sparseGraphView.load_graph(read_file("sparse_graph_data.txt"));
    #endif

    double start_time = time();
    double dt = 0;

    // Main loop
#ifdef __EMSCRIPTEN__
    emscripten_log(EM_LOG_CONSOLE, "The main loop definition begin!");
#endif

    bool done = false;
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!done)
#endif
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
			// Do not handle keyboard events when it is not needed.
			// switch(event.type) {
			// 	case SDL_KEYDOWN:
			// 	case SDL_KEYUP:
			// 	case SDL_TEXTINPUT:
			// 		if(!io.WantTextInput)
			// 			continue;
			// }
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        
        start_time = time();

        // Main window rendering pipeline

        // https://stackoverflow.com/questions/2249282/c-c-portable-way-to-detect-debug-release
        #ifdef _DEBUG
        ImGui::ShowDemoWindow();
        #endif

        dt = time() - start_time;
        // But the actual dt could also be used
        field.do_tick(.02);
        start_time = time();
        field.display_window();

		ImGui::getScroll
		field.set_debug_message(is_any_scrollable_hovered ? "true" : "false");

        //display_control_window(field);
        #ifndef EMSCRIPTEN_CODE
        sparseGraphView.show_window();
        display_algorithms_window(field, &sparseGraphView);
        #else
        display_algorithms_window(field);
        #endif

		is_any_scrollable_hovered = false;

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#else
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
#endif

    return 0;
}
