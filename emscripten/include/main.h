#pragma once

#include "imgui.h"
#include <set>

#ifdef __EMSCRIPTEN__
#define EMSCRIPTEN_CODE
#endif

#define FIXED_WINDOW_FLAGS ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground
#define FIXED_ALGORITHM_WINDOW_WIDTH 300

#define DELTA_TICKS 29

//#define EMSCRIPTEN_CODE
