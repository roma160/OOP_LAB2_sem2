#include "sparse_graph_view.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"

using Edge = Graph::Edge;

SparseGraphView::SparseGraphView(): graph(), coordinates(), bounds(),
    current_path(), selected_nodes() {}

bool SparseGraphView::load_graph(string data, const int first_node_index) {
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
        else if(coordinate_index < n + first_node_index) {
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

    selected_nodes.resize(graph.n);
    clear_selection();
    return true;
}

void SparseGraphView::show_window() {
    ImGui::Begin("Sparse graph view", nullptr, ImGuiWindowFlags_NoCollapse);
    ImGui::SetWindowSize({500, 500}, ImGuiCond_Once);

    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    const auto p = ImGui::GetCursorScreenPos();
    const auto a = (Vec2(ImGui::GetWindowSize()) - Vec2{30, 30}) / bounds;

    // Edges
    for(const auto &edge : graph.get_edges()) {
        if (!edge.second.connected) continue;
        draw_list->AddLine(
            coordinates[edge.first.first] * a + p,
            coordinates[edge.first.second] * a + p,
            ImColor(.3f, .3f, .3f, .3f),
            0.7f
        );
    }

    // Path
    if(current_path.size() > 0){
        for(int i = 1; i < current_path.size(); i++) {
            if (!graph.is_connected(current_path[i-1], current_path[i])) continue;
            draw_list->AddLine(
                coordinates[current_path[i-1]] * a + p,
                coordinates[current_path[i]] * a + p,
                ImColor(1.0f, .3f, .3f, .8f),
                1.0f
            );
        }
    }

    // Nodes
    for(int i = 0; i < graph.n; i++) {
        if(!selected_nodes[i]) {
            draw_list->AddCircleFilled(
                coordinates[i] * a + p,
                1.0, ImColor(1.0f, 1.0f, 1.0f, 1.0f)
            );
        }
        else {
            draw_list->AddCircleFilled(
                coordinates[i] * a + p,
                3, ImColor(.0f, .8f, .0f, 1.0f)
            );
        }
    }

    ImGui::End();
}

void SparseGraphView::set_current_path(vector<int> new_path)
{ current_path = new_path; }

void SparseGraphView::set_node_selection(int node_id, bool selection)
{ selected_nodes[node_id] = selection; }
void SparseGraphView::clear_selection() {
    current_path.clear();
    fill(selected_nodes.begin(), selected_nodes.end(), false);
}