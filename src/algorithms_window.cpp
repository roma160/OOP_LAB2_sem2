#include "algorithms_window.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"
#include "algorithms.tpp"

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

    ImGui::Dummy({0, 10});
    if(ImGui::Button("Reset selection")) {
        field.disselect_all_edges();
        field.disselect_all_points();
    }

    // Algortithms ComboBox
    static const vector<string> algorithms{ "1. BFS", "2. DFS" };
    static int item_current_idx = 1;
    if (ImGui::BeginCombo("Algorithm", algorithms[item_current_idx].c_str()))
    {
        for (int n = 0; n < algorithms.size(); n++)
        {
            const bool is_selected = (item_current_idx == n);
            if (ImGui::Selectable(algorithms[n].c_str(), is_selected))
                item_current_idx = n;
            if (is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    // Algortithm control
    ImGui::Dummy({0, 10});
    ImGui::Text(algorithms[item_current_idx].c_str());
    if(ImGui::Button("Execute")) {
        if(item_current_idx == 0) {
            algos::bfs(field, 0, 0);
        }
        else if(item_current_idx == 1) {
            algos::dfs(field, 0, 0);
        }
    }

    ImGui::End();
}