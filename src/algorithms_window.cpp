#include "algorithms_window.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"

#include <vector>
#include <string>

#include "vec2.h"

void display_algorithms_window(Field& field) {
    ImGui::Begin("Algorithms window", nullptr, ImGuiWindowFlags_NoCollapse);
    ImGui::SetWindowSize({300, 500}, ImGuiCond_Once);

    if (ImGui::TreeNode("Trees gfgdfgdf"))
    {
        ImGui::Text("Hllo");
        ImGui::TreePop();
    }

    ImGui::End();
}