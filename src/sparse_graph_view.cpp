#include "sparse_graph_view.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"

using Edge = Graph::Edge;

SparseGraphView::SparseGraphView(): graph(), coordinates(), bounds() {}

bool SparseGraphView::load_graph(string data, const int first_node_index = 1) {
    set<Edge> edges;
    vector<Vec2> coordinates;
    Vec2 bounds;
    stringstream ss(data);
    int n = -1, m = -1;
    int coordinate_index = first_node_index;
    for (string line; getline(ss, line); ) {
        if(line.empty()) continue;
        smatch match;
        if(n == -1) {
            static const regex r("(\\d+)\\s+(\\d+)");
            if(!regex_search(line, match, r)) return false;
            if(match.size() != 3) return false;
            n = stoi(match[1]);
            m = stoi(match[2]);
            coordinates = vector<Vec2>(n+1);
            continue;
        }
        else if(coordinate_index < n) {
            static const regex r("(\\d+)\\s+(\\d+)");
            if(!regex_search(line, match, r)) return false;
            if(match.size() != 3) return false;
            coordinates[coordinate_index].x = stoi(match[1]);
            coordinates[coordinate_index].y = stoi(match[2]);
            if(bounds.x < coordinates[coordinate_index].x)
                bounds.x = coordinates[coordinate_index].x;
            if(bounds.y < coordinates[coordinate_index].y)
                bounds.y = coordinates[coordinate_index].y;
            coordinate_index++;
            continue;
        }

        static const regex edge("(\\d+)\\s+(\\d+)\\s*(\\d*)");
        if(!regex_search(line, match, edge)) continue;
        if(match.size() != 3 && match.size() != 4) continue;

        int weight;
        if(match[3].str().empty()) weight = 1;
        else weight = stoi(match[3]);
        Edge to_push = {stoi(match[1]), stoi(match[2]), weight};
        edges.insert(to_push);
    }

    graph = SparseGraph(n+1, {edges.begin(), edges.end()});
    this->coordinates = coordinates;
    this->bounds = bounds;
    return true;
}

void SparseGraphView::show_window() {
    ImGui::Begin("Sparse graph view", nullptr, ImGuiWindowFlags_NoCollapse);
    ImGui::SetWindowSize({500, 500}, ImGuiCond_Once);

    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    for(int i = 0; i < graph.n; i++) {
        draw_list->AddCircleFilled(
            coordinates[i], 1.0, ImColor(1.0f, 1.0f, 1.0f, 1.0f)
        );
    }

    ImGui::End();
}