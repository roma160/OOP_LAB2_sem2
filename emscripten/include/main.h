#pragma once

#include "imgui.h"
#include <set>

#ifdef __EMSCRIPTEN__
#define EMSCRIPTEN_CODE
#endif

#define FIXED_WINDOW_FLAGS ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground
#define FIXED_ALGORITHM_WINDOW_WIDTH 300

#ifdef EMSCRIPTEN_CODE
#define DELTA_TICKS 25
#else
#define DELTA_TICKS 29
#endif

namespace global {
    static bool isDarkTheme;

    void setDarkTheme(bool ignore_current = false);
    void setLightTheme(bool ignore_current = false);
};

//#define EMSCRIPTEN_CODE
