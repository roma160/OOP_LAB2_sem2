#include "algorithms_window.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"

#include <vector>
#include <string>
#include <sstream>
#include <regex>

#include "vec2.h"

using namespace std;

void display_algorithms_window(Field& field) {
    ImGui::Begin("Algorithms window", nullptr, ImGuiWindowFlags_NoCollapse);
    ImGui::SetWindowSize({300, 500}, ImGuiCond_Once);

    if (ImGui::TreeNode("Change graph"))
    {
        static string graph_data = field.get_graphs()[0].to_string();
        if (ImGui::Button("Load")) {
            Graph buff;
            if(Graph::from_string(graph_data, buff)){
                field.remove_graph(0);
                field.add_graph(buff);
            }
        }
        ImGui::InputTextMultiline(
            "##graph_data", &graph_data,
            ImVec2(-1, ImGui::GetTextLineHeight() * 16),
            ImGuiInputTextFlags_AllowTabInput
        );

        ImGui::TreePop();
    }

    ImGui::Text("Algorithms:");

    ImGui::End();
}