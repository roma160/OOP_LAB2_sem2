#include "algorithms_window.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"
#include "algorithms.tpp"
#include "vec2.h"
#include "utils.tpp"
#include "main.h"

#include <vector>
#include <string>
#include <sstream>
#include <regex>
#include <fstream>
#include <istream>

using namespace std;

bool get_int(const string& str, int& ret) {
    try
    {
        int buff = stoi(str);
        ret = buff;
    }
    catch(std::invalid_argument const& ex)
    {
        return false;
    }
    catch(std::out_of_range const& ex)
    {
        return false;
    }
    return true;
}


void reset_field(Field& field) {
    field.disselect_all_edges();
    field.disselect_all_points();
    field.get_graph(0)->clear_annotations();
}

struct steps {
    bool show;
    int max;
    int cur;
    Field& field;
    bool changed;
    bool disabled;

    steps(Field& field): field(field) {
        reset();
    }

    void reset() {
        show = false;
        changed = false;
        max = 0;
        cur = -1;
        field.set_physics_ban(false);
    }

    void set_changed() {
        changed = true;
    }

    void pop_changed() {
        if(changed) changed = false;
        else cur = -1;
    }
};

void display_algorithms_window(Field& field, SparseGraphView* sparseGraphView) {
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

    #ifdef EMSCRIPTEN_CODE
    const ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse | FIXED_WINDOW_FLAGS;
    ImGui::SetNextWindowPos(ImVec2(
        main_viewport->WorkPos.x + main_viewport->Size.x - FIXED_ALGORITHM_WINDOW_WIDTH,
        main_viewport->WorkPos.y
    ));
    #else
    const ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
    #endif

    ImGui::Begin("Algorithms window", nullptr, window_flags);

    #ifdef EMSCRIPTEN_CODE
    ImGui::SetWindowSize({
        FIXED_ALGORITHM_WINDOW_WIDTH,
        main_viewport->Size.y
    });
    #else
    ImGui::SetWindowSize({300, 500}, ImGuiCond_Once);
    #endif

    static steps steps(field);

    static bool execute_requested = false;
    static string graph_data = field.get_graph(0)->to_string();
    static string graph_description = field.get_graph(0)->to_info_string();
    ImGui::Text("Currect graph: %s", graph_description.c_str());
    if (ImGui::TreeNode("Change graph"))
    {
        if (ImGui::Button("Load")) {
            Graph buff;
            if(Graph::from_string(graph_data, buff)){
                graph_description = buff.to_info_string();

                Graph& cur_graph = *field.get_graph(0);
                if(cur_graph.includes(buff) || (buff.includes(cur_graph) && 
                    buff.connections.size() == cur_graph.connections.size())
                ) {
                    for(int i = cur_graph.edges.size() - 1; i >= 0; i--)
                        cur_graph.remove_edge(i);
                    for(int i = 0; i < buff.edges.size(); i++)
                        cur_graph.add_edge(
                            buff.edges[i].first,
                            buff.edges[i].second,
                            buff.edges[i].weight
                        );
                }
                else {
                    field.remove_graph(0);
                    field.add_graph(buff);
                }

                reset_field(field);
                steps.reset();
            }
        }

        if (sparseGraphView != nullptr) {
            ImGui::SameLine();
            if (ImGui::Button("Load headless")) {
                string graph_data = read_file("sparse_graph_data.txt");
                if(sparseGraphView->load_graph(graph_data)) {
                    graph_description = sparseGraphView->graph.to_info_string();
                }
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
        field.get_graph(0)->clear_annotations();

        if(sparseGraphView != nullptr)
            sparseGraphView->clear_selection();
    }

    static bool incorrect_input = false;
    static string log = "";

    // Algortithms ComboBox
    static const vector<string> algorithms{
        "1. BFS", "2. DFS", "3. Prim's min tree",
        "4. Dijkstra's min path", "5. A* min path",
        "6. FordFulkerson max flow",
        #ifndef EMSCRIPTEN_CODE
        "7. Bidirect Dijkstra's min path",
        "8. Bidirect A* min path",
        #endif
    };
    static int item_current_idx = algorithms.size() - 1;
    static int item_prev_idx = -1;
    if (ImGui::BeginCombo("Algorithm", algorithms[item_current_idx].c_str()))
    {
        for (int n = 0; n < algorithms.size(); n++)
        {
            const bool is_selected = (item_current_idx == n);
            if (ImGui::Selectable(algorithms[n].c_str(), is_selected)){
                item_current_idx = n;
                incorrect_input = false;
                log.clear();

                switch (item_current_idx)
                {
                case 4:
                    field.set_show_actual_distance(true);
                    break;
                
                default:
                    field.set_show_actual_distance(false);
                    break;
                }
            }
            if (is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    if(item_current_idx != item_prev_idx) {
        steps.reset();
        item_prev_idx = item_current_idx;
        steps.disabled = item_current_idx > 4;
    }

    // Algortithm control
    static string from_node_str = to_string(1);
    static string to_node_str = to_string(field.get_graph(0)->connections.size() - 1);

    ImGui::Dummy({0, 10});
    if(incorrect_input) ImGui::TextColored(ImVec4(1, 0, 0, 1), "Incorrect input data!");
    if(ImGui::Button("Execute") || execute_requested || steps.changed) {
        steps.pop_changed();
        execute_requested = false;

        incorrect_input = false;
        reset_field(field);

        if(item_current_idx == 0) {
            algos::bfs(field, 0, 0, &steps.max, steps.cur);
        }
        else if(item_current_idx == 1) {
            algos::dfs(field, 0, 0, &steps.max, steps.cur);
        }
        else if(item_current_idx == 2) {
            algos::prims_min_tree(field, 0, 0, &steps.max, steps.cur);
        }
        else if(item_current_idx == 3) {
            int from = -1, to = -1;
            if(!get_int(from_node_str, from) || !get_int(to_node_str, to))
                incorrect_input = true;
            else algos::dijkstra_path(field, 0, from, to, &steps.max, steps.cur);
        }
        else if(item_current_idx == 4) {
            int from = -1, to = -1;
            if(!get_int(from_node_str, from) || !get_int(to_node_str, to))
                incorrect_input = true;
            else {
                Graph& graph = *field.get_graph(0);
                stringstream log_stream;
                log_stream<<"The checked nodes:\n";
                algos::astar_path(field, 0, from, to, &log_stream, &steps.max, steps.cur);
                log = log_stream.str();
            }
        }
        else if(item_current_idx == 5) {
            int from = -1, to = -1;
            if(!get_int(from_node_str, from) || !get_int(to_node_str, to))
                incorrect_input = true;
            else {
                auto& graph = *field.get_graph(0);
                auto res = algos::ff_max_flow(field, 0, from, to);
                stringstream log_stream;
                log_stream << "The maximum flow: " << res.maxFlow;
                log = log_stream.str();

                field.select_point(from, 0, ImColor(1.0f, .0f, .0f, 1.0f));
                field.select_point(to, 0, ImColor(.0f, 1.0f, .0f, 1.f));
                const int n = graph.connections.size();
                for(int i = 0; i < n; i++)
                for(int j = 0; j < n; j++){
                    if(i >= j || !res.flowData.connections[i][j].connected)
                        continue;
                    stringstream builder;
                    builder << graph.connections[i][j].weight << "/" <<
                        min(res.flowData.connections[j][i].weight, res.flowData.connections[i][j].weight);
                    graph.edges_anno[graph.get_edge_id(i, j)] = builder.str();
                }
            }
        }
        else if(sparseGraphView == nullptr) { 
            // empty case to not use the null variable
        }
        else if(item_current_idx == 6) {
            int from = -1, to = -1;
            if(!get_int(from_node_str, from) || !get_int(to_node_str, to))
                incorrect_input = true;
            else {
                stringstream ss;
                auto res = algos::bidirect_dijkstra_path(sparseGraphView->graph, from, to, &ss);
                log = ss.str();

                sparseGraphView->clear_selection();
                sparseGraphView->set_current_path(res.path);
                for(int node : res.checked_nodes)
                    sparseGraphView->set_node_selection(node, true);
            }
        }
        else if(item_current_idx == 7) {
            int from = -1, to = -1;
            if(!get_int(from_node_str, from) || !get_int(to_node_str, to))
                incorrect_input = true;
            else {
                stringstream ss;
                auto res = algos::bidirect_astar_path(*sparseGraphView, from, to, &ss);
                log = ss.str();

                sparseGraphView->clear_selection();
                sparseGraphView->set_current_path(res.path);
                for(int node : res.checked_nodes)
                    sparseGraphView->set_node_selection(node, true);
            }
        }

        if(steps.cur < 0) {
            steps.cur = steps.max;
        }
    }

    ImGui::SameLine();
    ImGui::BeginDisabled(steps.max <= 0 || steps.disabled);
    if(ImGui::Checkbox("Show steps", &steps.show)) {
        field.set_physics_ban(steps.show);
    }
    ImGui::EndDisabled();

    switch (item_current_idx)
    {
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        const float x = ImGui::GetContentRegionAvail().x / 2 - 10;
        ImGui::SetNextItemWidth(x - ImGui::CalcTextSize("Start point").x);
        ImGui::InputText("Start point", &from_node_str, ImGuiInputTextFlags_CharsDecimal);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(x - ImGui::CalcTextSize("End point").x);
        if(ImGui::InputText("End point", &to_node_str, 
            ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue))
        { execute_requested = true; }
        break;
    }

    if(steps.show) {
        ImGui::Spacing();
        if(ImGui::ArrowButton("##prev_step", ImGuiDir_Left) && steps.cur > 0) {
            steps.cur--;
            steps.set_changed();
        }

        ImGui::SameLine();
        ImGui::PushItemWidth(-25);
        if(ImGui::SliderInt("##", &steps.cur, 0, steps.max, "%d"))
            steps.set_changed();

        ImGui::SameLine();
        if(ImGui::ArrowButton("##next_step", ImGuiDir_Right) && steps.cur < steps.max) {
            steps.cur++;
            steps.set_changed();
        }
    }

    if(!log.empty()){
        ImGui::Text("%s", log.c_str());
    }

    ImGui::End();
}