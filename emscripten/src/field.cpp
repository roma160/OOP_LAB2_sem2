#include "imgui.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"

#include "field.h"
#include "utils.h"
#include "main.h"

#include <math.h>
#include <iostream>

using namespace std;

const Vec2 Field::DEF_GRAPH_LOC = {200, 200};
const float Field::DEF_GRAPH_R = 100;


Field::FGraph::Selection::Selection(bool is_selected, ImColor color):
    is_selected(is_selected), color(color) {}

Field::FGraph::FGraph(const Graph& graph): 
    Graph(graph), points(connections.size()), speeds(connections.size()),
    points_sel(connections.size()), edges_sel(edges.size()), edges_anno(edges.size()) {}
Field::FGraphLink::FGraphLink(int graph_id, int index): graph_id(graph_id), index(index) {}

void Field::FGraph::add_edge(int from, int to, int weight){
    if(from > to) swap(from, to);
    Graph::add_edge(from, to, weight);
    edges_sel.push_back(Selection());
    edges_anno.push_back("");
}

void Field::FGraph::remove_edge(int edge_id){
    Graph::remove_edge(edge_id);
    edges_sel.erase(edges_sel.begin() + edge_id);
    edges_anno.erase(edges_anno.begin() + edge_id);
}

void Field::FGraph::clear_annotations() {
    for(int i = 0; i < edges.size(); i++)
        edges_anno[i].clear();
}


Field::Field(float cell_size, Vec2 bounds): 
    cell_size(cell_size), bounds(bounds), graphs(),
    is_dark_theme_selected(global::isDarkTheme) {}

pair<int, int> Field::get_field_index(const Vec2& point) const {
    return {
        static_cast<int>(point.x / cell_size),
        static_cast<int>(point.y / cell_size)
    };
}

void Field::remove_from_field(const pair<int, int>& field_index, FGraphLink point_index) {
    field[field_index.first][field_index.second].remove(point_index);
}

void Field::add_to_field(const pair<int, int>& field_index, FGraphLink point_index) {
    field[field_index.first][field_index.second].push_back(point_index);
}

int Field::add_graph(const Graph& graph, Vec2 point, float R) {
    graphs.push_back(FGraph(graph));
    const int graph_id = graphs.size() - 1;
    reset_points_pos(graph_id, point, R);

    auto& fgraph = graphs[graph_id];
    for(int i = 0; i < fgraph.connections.size(); i++){
        const auto field_index = get_field_index(fgraph.points[i]);
        add_to_field(field_index, { graph_id, i });
    }
    return graph_id;
}
void Field::remove_graph(int index) {
    auto& fgraph = graphs[index];
    for(int i = 0; i < fgraph.connections.size(); i++){
        const auto field_index = get_field_index(fgraph.points[i]);
        remove_from_field(field_index, { index, i });
    }
    graphs.erase(graphs.begin() + index);
}

inline bool operator==(const Field::FGraphLink& a, const Field::FGraphLink& b)
{ return a.graph_id == b.graph_id && a.index == b.index; }
inline bool operator!=(const Field::FGraphLink& a, const Field::FGraphLink& b)
{ return !(a == b); }

Vec2 Field::def_compute_force(Vec2 delta, float force_distance, ForceType type) {
    if(type == Node){
        const float x = (delta.abs() - force_distance);
        if (x >= 0) return delta.norm() * sqrt(abs(x)) * 10;
        else return delta.norm() * pow(x / 10, 3);
    }
    else if(type == ConnectedNode){
        const float x = (delta.abs() - force_distance / 2);
        return delta.norm() * pow(x, 2) * sign(x);
    }
    else {
        float x = 0;
        if(type == UpBound) x += delta.y;
        else if (type == DownBound) x += -delta.y;
        else if (type == LeftBound) x += delta.x;
        else if (type == RightBound) x += -delta.x;
        float s = sign(x);
        x -= force_distance / 2;
        
        if (x > 0) return Vec2();
        return delta.norm() * (pow(x, 2) * s);
    }
}

void Field::do_tick(float dt, Vec2 (*force_function)(Vec2, float, ForceType)){
    if(!use_ticks) return;

    vector<vector<Vec2>> forces(graphs.size());
    for(int g = 0; g < graphs.size(); g++){
        const int n = graphs[g].connections.size();
        forces[g] = vector<Vec2>(n);
        for(FGraphLink i = {0, g}; i.index < n; i.index++){
            Vec2 &point = graphs[g].points[i.index];

            // Looking over connected verticies
            for(int j = 0; j < n; j++){
                if(i.index == j || !graphs[g].connections[i.index][j]) continue;
                Vec2 &point_b = graphs[g].points[j];
                forces[i.graph_id][i.index] += force_function(
                    point_b - point, cell_size, ConnectedNode);
            }

            // Looking in the adjacent squares.
            const auto field_index = get_field_index(point);
            for (int dx = -1; dx <= 1; dx++)
            for (int dy = -1; dy <= 1; dy++) {
                const auto& list = field[field_index.first + dx][field_index.second + dy];
                if(list.size() == 0) continue;
                for(auto buff : list) {
                    if (buff == i || graphs[g].connections[i.index][buff.index]) continue;
                    const Vec2 &point_b = graphs[buff.graph_id].points[buff.index];
                    forces[i.graph_id][i.index] += force_function(
                        point_b - point, cell_size, Node);
                }
            }

            // Looking over walls
            if(bound_forces){
                forces[i.graph_id][i.index] +=
                    force_function({point.x, 0}, cell_size, LeftBound) + 
                    force_function({point.x - bounds.x, 0}, cell_size, RightBound) + 
                    force_function({0, point.y}, cell_size, UpBound) +
                    force_function({0, point.y - bounds.y}, cell_size, DownBound);
            }
        }
    }

    for(int g = 0; g < graphs.size(); g++){
        const int n = graphs[g].connections.size();
        for(FGraphLink i = {0, g}; i.index < n; i.index++){
            Vec2 &point = graphs[g].points[i.index];
            Vec2 &speed = graphs[g].speeds[i.index];
            const auto field_index = get_field_index(point);
            
            speed += forces[i.graph_id][i.index] * dt;
            speed *= 0.95;
            const static float max_speed = 1e3;
            const float abs_speed = speed.abs();
            if (abs_speed > max_speed) speed *= max_speed / abs_speed;
            if (abs_speed < 0.5) speed = {0, 0};

            point += speed * dt;

            recalculate_cell_for_point(field_index, i);
        }
    }
}

void Field::recalculate_cell_for_point(const pair<int, int>& old_field_index, FGraphLink point_index) {
    const auto new_field_index = get_field_index(graphs[point_index.graph_id].points[point_index.index]);
    if(old_field_index != new_field_index){
        remove_from_field(old_field_index, point_index);
        add_to_field(new_field_index, point_index);
    }
}

void Field::reset_points_pos(int graph_id, Vec2 point, float R) {
    FGraph& graph = graphs[graph_id];
    const int n = graph.connections.size();
    for(int i = 0; i < n; i++) {
        // Removing the old one from the field
        {
            const auto field_index = get_field_index(graph.points[i]);
            remove_from_field(field_index, { graph_id, i });
        }

        // Setting new position
        const float alpha = 2 * i * PI / n;
        graph.points[i] = Vec2{ cos(alpha), sin(alpha) } * R + point;
        graph.speeds[i] = {0, 0};
        {
            const auto field_index = get_field_index(graph.points[i]);
            add_to_field(field_index, { graph_id, i });
        }
    }
}

void Field::display_window(){
    const float R = 18;
    static FGraphLink selected = {-1, 0};

    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

    #ifdef EMSCRIPTEN_CODE
    const ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | FIXED_WINDOW_FLAGS;
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y));
    #else
    const ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 20, main_viewport->WorkPos.y + 20), ImGuiCond_Once);
    #endif

    ImGui::Begin("Field", nullptr, window_flags);

    #ifdef EMSCRIPTEN_CODE
    ImGui::SetWindowSize({
        main_viewport->Size.x - FIXED_ALGORITHM_WINDOW_WIDTH,
        main_viewport->Size.y
    });
    #else
    ImGui::SetWindowSize(bounds, ImGuiCond_Once);
    #endif
    bounds = ImGui::GetWindowSize();

    const Vec2 const_p = ImGui::GetCursorScreenPos();
    const Vec2 cursor = ImGui::GetCursorPos();
    Vec2 available_space = ImGui::GetContentRegionAvail();
    if(abs(available_space.x) <= 1) available_space.x = 1;
    if(abs(available_space.y) <= 1) available_space.y = 1;
    // https://github.com/ocornut/imgui/issues/3149
    ImGui::InvisibleButton("canvas", available_space,
        ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_AllowItemOverlap);
    ImGui::SetItemAllowOverlap();

    const Vec2 mouse = ImGui::GetMousePos();
    Vec2 middle_mouse_delta;

    // Dragging feature
    if(ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
        if(!was_middle_mouse) {
            was_middle_mouse = true;
            middle_mouse_prev = mouse;
        }
        middle_mouse_delta = mouse - middle_mouse_prev;
    }
    else if(was_middle_mouse) {
        middle_mouse_shift += mouse - middle_mouse_prev;
        middle_mouse_delta = {0, 0};
        was_middle_mouse = false;
    }
    const auto p = const_p + middle_mouse_shift + middle_mouse_delta;

    if (selected.graph_id == -1 && ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        auto field_index = get_field_index(mouse - p);
        int shift_x = 2 * fmod(mouse.x - p.x, cell_size) >= cell_size;
        int shift_y = 2 * fmod(mouse.y - p.y, cell_size) >= cell_size;
        for(int dx = -1; dx <= 0 && selected.graph_id == -1; dx++)
        for(int dy = -1; dy <= 0 && selected.graph_id == -1; dy++){
            const auto& list = field[field_index.first + dx + shift_x][field_index.second + dy + shift_y];
            if(list.size() == 0) continue;
            for(auto buff : list){
                if(selected.graph_id != -1) break;
                const Vec2 &point = graphs[buff.graph_id].points[buff.index];
                if ((point + p - mouse).abs() <= R) selected = buff;
            }
        }
    }
    if (selected.graph_id != -1 && ImGui::IsMouseDown(ImGuiMouseButton_Left)){
        Vec2& point = graphs[selected.graph_id].points[selected.index];
        const auto old_field_index = get_field_index(point);
        point = mouse - p;
        recalculate_cell_for_point(old_field_index, selected);
    }
    else if (selected.graph_id != -1 && !ImGui::IsItemClicked(ImGuiMouseButton_Left))
        selected = {-1, 0};

    static bool debug_field = false;
    ImGui::SetCursorPos(cursor);

    // User controls
    if(show_options) {
        ImGui::Checkbox("FieldN", &debug_field);

        ImGui::BeginDisabled(physics_ban);
        ImGui::SameLine();
        ImGui::Checkbox("Phys", &use_ticks);
        ImGui::EndDisabled();

        ImGui::SameLine();
        {
            string text = bounds.to_string();
            ImGui::Text("Size: %s", text.c_str());
        }
        ImGui::SameLine();
        ImGui::Checkbox("Bounds", &bound_forces);

        ImGui::SameLine();
        if(ImGui::Checkbox("Use dark theme", &is_dark_theme_selected)) {
            if (is_dark_theme_selected) {
                global::setDarkTheme();
            }
            else {
                global::setLightTheme();
            }
        }
    }
    else {
        ImGui::Checkbox("show_options", &show_options);
    }

    ImGui::SameLine();
    if(ImGui::Button("Reset pos")) {
        for(int i = 0; i < graphs.size(); i++)
            reset_points_pos(i);
    }

    if(show_options) {
        ImGui::Checkbox("node_i", &show_node_ids);
        ImGui::SameLine();
        ImGui::Checkbox("edge_w", &show_edge_weights);
        ImGui::SameLine();
        ImGui::Checkbox("only_sel", &show_only_selected_edges);
        ImGui::SameLine();
        ImGui::Checkbox("real_dist", &show_actual_distance);
        ImGui::SameLine();
        ImGui::Checkbox("show_options", &show_options);
    }

    // Debug message
    if(!debug_message.empty()) {
        const auto message_size = ImGui::CalcTextSize(debug_message.c_str());
        ImGui::SetCursorPos({10, bounds.y - message_size.y});
        ImGui::Text(debug_message.c_str());
    }

    // Actual graph drawing
    const ImU32 node_color = ImGui::ColorConvertFloat4ToU32(
        ImGui::GetStyleColorVec4(ImGuiCol_PlotHistogram));
    const ImU32 edge_color = ImGui::ColorConvertFloat4ToU32(
        ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    const ImU32 text_color = ImGui::ColorConvertFloat4ToU32(
        ImGui::GetStyleColorVec4(ImGuiCol_Text));
    const ImU32 text_bg_color = ImGui::ColorConvertFloat4ToU32(
        ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
    const ImU32 green_col = ImColor(0.0f, 1.0f, 0.1f, 1.0f);
    const ImU32 blue_col = ImColor(0.0f, 0.0f, 1.0f, 1.0f);

    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    for(int g = 0; g < graphs.size(); g++){
        const auto& graph = graphs[g];

        // Drawing edges
        const int m = graph.edges.size();
        for (int i = 0; i < m; i++){
            auto a = p + graph.points[graph.edges[i].first];
            auto b = p + graph.points[graph.edges[i].second];
            if(graph.edges_sel[i].is_selected){
                draw_list->AddLine(a, b,
                    graph.edges_sel[i].color, 3.0f
                );
            }
            else if(!show_only_selected_edges)
                draw_list->AddLine(a, b, edge_color);

            if(show_edge_weights && (graph.edges_sel[i].is_selected || !show_only_selected_edges)) {
                string num;
                if(!show_actual_distance) num = to_string(graph.edges[i].weight);
                else num = to_string((int) round(get_field_distance(
                    g, graph.edges[i].first, graph.edges[i].second)));
                
                const auto calc_size = ImGui::CalcTextSize(num.c_str());
                const auto text_p1 = (a + b) / 2 - (Vec2) calc_size / 2;
                draw_list->AddRectFilled(text_p1, text_p1 + calc_size, text_bg_color);
                draw_list->AddText(text_p1,
                    text_color, num.c_str()
                );

                if(!graph.edges_anno[i].empty()) { 
                    const auto anno_size = ImGui::CalcTextSize(graph.edges_anno[i].c_str());
                    const auto anno_p1 = (a + b) / 2  - (Vec2) anno_size / 2 + Vec2{0, calc_size.y};
                    draw_list->AddRectFilled(anno_p1, anno_p1 + anno_size, text_bg_color);
                    draw_list->AddText(anno_p1,
                        text_color, graph.edges_anno[i].c_str()
                    );
                }
            }
        }

        // Drawing nodes
        const int n = graph.connections.size();
        for (int i = 0; i < n; i++)
        {
            const Vec2& point = graph.points[i];
            // TODO: implement random coloring
            draw_list->AddNgonFilled(p + point, R, node_color, 36);
            if(graph.points_sel[i].is_selected)
                draw_list->AddCircle(p + point, R + 1, graph.points_sel[i].color, 36, 5.0f);

            if(show_node_ids){
                string num = to_string(i);
                draw_list->AddText(
                    p + point - (Vec2) ImGui::CalcTextSize(num.c_str()) / 2,
                    blue_col, num.c_str()
                );
            }
        }
    }

    if(debug_field)
    for (auto i = field.begin(); i != field.end(); i++){
        if(i->first < 0 || i->first * cell_size > available_space.x) continue;
        for(auto j = i->second.begin(); j != i->second.end(); j++){
            if(j->first < 0 || j->first * cell_size > available_space.y) continue;
            ImGui::SetCursorPos(cursor + (p - const_p) + Vec2{i->first * cell_size, j->first * cell_size});
            {
                string num = to_string(j->second.size());
                ImGui::TextColored(ImVec4(0, 255, 0, 255), num.c_str());
            }
        }
    }

    ImGui::End();
}


void Field::select_point(int point_id, int graph_id, ImColor color) {
    auto& selection = graphs[graph_id].points_sel[point_id];
    if(selection.color != color) selection.color = color;
    selection.is_selected = true;
}
void Field::disselect_point(int point_id, int graph_id) {
    auto& selection = graphs[graph_id].points_sel[point_id];
    selection.is_selected = false;
}
void Field::disselect_all_points(int graph_id) {
    for(int i = 0; i < graphs[graph_id].connections.size(); i++)
        disselect_point(i, graph_id);
}
void Field::toggle_point_select(int point_id, int graph_id, ImColor color) {
    auto& selection = graphs[graph_id].points_sel[point_id];
    if(selection.is_selected) selection.is_selected = false;
    else {
        if(selection.color != color) selection.color = color;
        selection.is_selected = true;
    }
}

void Field::select_edge(int edge_id, int graph_id, ImColor color) {
    if(edge_id == -1) return;
    auto& selection = graphs[graph_id].edges_sel[edge_id];
    if(selection.color != color) selection.color = color;
    selection.is_selected = true;
}
void Field::disselect_edge(int edge_id, int graph_id) {
    auto& selection = graphs[graph_id].edges_sel[edge_id];
    selection.is_selected = false;
}
void Field::disselect_all_edges(int graph_id) {
    for(int m = 0; m < graphs[graph_id].edges.size(); m++)
        disselect_edge(m, graph_id);
}
void Field::toggle_edge_select(int edge_id, int graph_id, ImColor color) {
    auto& selection = graphs[graph_id].edges_sel[edge_id];
    if(selection.is_selected) selection.is_selected = false;
    else {
        if(selection.color != color) selection.color = color;
        selection.is_selected = true;
    }
}

const vector<Field::FGraph>& Field::get_graphs() const { return graphs; }
Field::FGraph* Field::get_graph(int graph_index) { return &graphs[graph_index]; }

float Field::get_field_distance(int graph_id, int from_id, int to_id)
{
    const auto& points = graphs[graph_id].points;
    return (points[from_id] - points[to_id]).abs();
}

void Field::set_show_actual_distance(bool val) { show_actual_distance = val; }

void Field::set_physics_ban(bool is_banned) {
    physics_ban = is_banned;
    if(is_banned)
        use_ticks = false;
}

#ifdef _DEBUG
void Field::set_debug_message(string message) {
    debug_message = message;
}

void Field::hide_debug_message() {
    debug_message = "";
}
#endif