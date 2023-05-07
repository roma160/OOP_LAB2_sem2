#include "imgui.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"

#include "field.h"
#include "utils.tpp"

#include <math.h>
#include <iostream>

using namespace std;

Field::FGraph::Selection::Selection(bool is_selected, ImColor color):
    is_selected(is_selected), color(color) {}

Field::FGraph::FGraph(const Graph& graph, Vec2 point, float R): 
    Graph(graph), points(connections.size()), speeds(connections.size()),
    points_sel(connections.size()), edges_sel(edges.size())
{
    const int n = connections.size();
    for(int i = 0; i < n; i++) {
        const float alpha = 2 * i * M_PI / n;
        points[i] = Vec2{ cos(alpha), sin(alpha) } * R + point;
    }
}
Field::FGraphLink::FGraphLink(int graph_id, int node_id): graph_id(graph_id), node_id(node_id) {}


Field::Field(float cell_size, Vec2 bounds): cell_size(cell_size), bounds(bounds), graphs() {}

pair<int, int> Field::get_field_index(const Vec2& point) const {
    return {
        static_cast<int>(point.x / cell_size),
        static_cast<int>(point.y / cell_size)
    };
}

void Field::remove_from_field(const pair<int, int>& field_index, FGraphLink point_index) {
    field[field_index.first][field_index.second].remove_val(point_index);
}

void Field::add_to_field(const pair<int, int>& field_index, FGraphLink point_index) {
    field[field_index.first][field_index.second].push_back(point_index);
}

int Field::add_graph(const Graph& graph, Vec2 point, float R) {
    graphs.push_back(FGraph(graph, point, R));
    const int graph_id = graphs.size() - 1;
    auto& fgraph = graphs[graph_id];
    for(int i = 0; i < fgraph.connections.size(); i++){
        const auto field_index = get_field_index(fgraph.points[i]);
        add_to_field(field_index, { graph_id, i });
    }
    return graph_id;
}

inline bool operator==(const Field::FGraphLink& a, const Field::FGraphLink& b)
{ return a.graph_id == b.graph_id && a.node_id == b.node_id; }
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
    if(stop_ticks) return;

    vector<vector<Vec2>> forces(graphs.size());
    for(int g = 0; g < graphs.size(); g++){
        const int n = graphs[g].connections.size();
        forces[g] = vector<Vec2>(n);
        for(FGraphLink i = {0, g}; i.node_id < n; i.node_id++){
            Vec2 &point = graphs[g].points[i.node_id];

            // Looking over connected verticies
            for(int j = 0; j < n; j++){
                if(i.node_id == j || !graphs[g].connections[i.node_id][j]) continue;
                Vec2 &point_b = graphs[g].points[j];
                forces[i.graph_id][i.node_id] += force_function(
                    point_b - point, cell_size, ConnectedNode);
            }

            // Looking in the adjacent squares.
            const auto field_index = get_field_index(point);
            for (int dx = -1; dx <= 1; dx++)
            for (int dy = -1; dy <= 1; dy++) {
                const auto& list = field[field_index.first + dx][field_index.second + dy];
                if(list.size == 0) continue;
                auto buff = list.first;
                for(; buff != nullptr; buff = buff->next){
                    if (buff->val == i || graphs[g].connections[i.node_id][buff->val.node_id]) continue;
                    const Vec2 &point_b = graphs[buff->val.graph_id].points[buff->val.node_id];
                    forces[i.graph_id][i.node_id] += force_function(
                        point_b - point, cell_size, Node);
                }
            }

            // Looking over walls
            if(bound_forces){
                forces[i.graph_id][i.node_id] +=
                    force_function({point.x, 0}, cell_size, LeftBound) + 
                    force_function({point.x - bounds.x, 0}, cell_size, RightBound) + 
                    force_function({0, point.y}, cell_size, UpBound) +
                    force_function({0, point.y - bounds.y}, cell_size, DownBound);
            }
        }
    }

    for(int g = 0; g < graphs.size(); g++){
        const int n = graphs[g].connections.size();
        for(FGraphLink i = {0, g}; i.node_id < n; i.node_id++){
            Vec2 &point = graphs[g].points[i.node_id];
            Vec2 &speed = graphs[g].speeds[i.node_id];
            const auto field_index = get_field_index(point);
            
            speed += forces[i.graph_id][i.node_id] * dt;
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
    const auto new_field_index = get_field_index(graphs[point_index.graph_id].points[point_index.node_id]);
    if(old_field_index != new_field_index){
        remove_from_field(old_field_index, point_index);
        add_to_field(new_field_index, point_index);
    }
}

void Field::display_window(){
    const float R = 18;
    static FGraphLink selected = {-1, 0};

    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 20, main_viewport->WorkPos.y + 20), ImGuiCond_Appearing);
    ImGui::Begin("Field", nullptr, ImGuiWindowFlags_NoCollapse);
    ImGui::SetWindowSize(bounds, ImGuiCond_Appearing);
    bounds = ImGui::GetWindowSize();

    const Vec2 p = ImGui::GetCursorScreenPos();
    const Vec2 cursor = ImGui::GetCursorPos();
    Vec2 available_space = ImGui::GetContentRegionAvail();
    if(abs(available_space.x) <= 1) available_space.x = 1;
    if(abs(available_space.y) <= 1) available_space.y = 1;
    // https://github.com/ocornut/imgui/issues/3149
    ImGui::InvisibleButton("canvas", available_space,
        ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_AllowItemOverlap);
    ImGui::SetItemAllowOverlap();

    const Vec2 mouse = ImGui::GetMousePos();
    if (selected.graph_id == -1 && ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        auto field_index = get_field_index(mouse);
        int shift_x = 2 * fmod(mouse.x, cell_size) >= cell_size;
        int shift_y = 2 * fmod(mouse.y, cell_size) >= cell_size;
        for(int dx = -1; dx <= 0 && selected.graph_id == -1; dx++)
        for(int dy = -1; dy <= 0 && selected.graph_id == -1; dy++){
            const auto& list = field[field_index.first + dx + shift_x][field_index.second + dy + shift_y];
            if(list.size == 0) continue;
            auto buff = list.first;
            for(; buff != nullptr && selected.graph_id == -1; buff = buff->next){
                const Vec2 &point = graphs[buff->val.graph_id].points[buff->val.node_id];
                if ((point + p - mouse).abs() <= R) selected = buff->val;
            }
        }
    }
    if (selected.graph_id != -1 && ImGui::IsMouseDown(ImGuiMouseButton_Left)){
        Vec2& point = graphs[selected.graph_id].points[selected.node_id];
        const auto old_field_index = get_field_index(point);
        point = mouse - p;
        recalculate_cell_for_point(old_field_index, selected);
    }
    else if (selected.graph_id != -1 && !ImGui::IsItemClicked(ImGuiMouseButton_Left))
        selected = {-1, 0};

    static bool debug_field = false;
    ImGui::SetCursorPos(cursor);
    ImGui::Checkbox("Enable field debugging", &debug_field);
    ImGui::SameLine();
    ImGui::Checkbox("Stop ticks", &stop_ticks);
    ImGui::SameLine();
    ImGui::Checkbox("Show ids", &show_node_ids);
    ImGui::SameLine();
    {
        string text = bounds.to_string();
        ImGui::Text("Size: %s", text.c_str());
    }
    ImGui::SameLine();
    ImGui::Checkbox("Bounds", &bound_forces);

    const ImU32 col = ImColor(1.0f, 1.0f, 0.4f, 1.0f);
    const ImU32 green_col = ImColor(0.0f, 1.0f, 0.1f, 1.0f);
    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    for(int g = 0; g < graphs.size(); g++){
        const auto& graph = graphs[g];

        // Drawing edges
        const int m = graph.edges.size();
        for (int i = 0; i < m; i++){
            draw_list->AddLine(
                p + graph.points[graph.edges[i].first],
                p + graph.points[graph.edges[i].second],
                col
            );
        }

        // Drawing nodes
        const int n = graph.connections.size();
        for (int i = 0; i < n; i++)
        {
            const Vec2& point = graph.points[i];
            // TODO: implement random coloring
            draw_list->AddNgonFilled(p + point, R, col, 36);
            if(graph.points_sel[i].is_selected)
                draw_list->AddCircle(p + point, R + 1, graph.points_sel[i].color, 36, 5.0f);

            if(show_node_ids){
                string num = to_string(i);
                draw_list->AddText(
                    p + point - (Vec2) ImGui::CalcTextSize(num.c_str()) / 2,
                    ImColor(0, 0, 255), num.c_str()
                );
            }
        }
    }

    if(debug_field)
    for (auto i = field.begin(); i != field.end(); i++){
        if(i->first < 0 || i->first * cell_size > available_space.x) continue;
        for(auto j = i->second.begin(); j != i->second.end(); j++){
            if(j->first < 0 || j->first * cell_size > available_space.y) continue;
            ImGui::SetCursorPos(cursor + Vec2{i->first * cell_size, j->first * cell_size});
            {
                string num = to_string(j->second.size);
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
void Field::toggle_point_select(int point_id, int graph_id, ImColor color) {
    auto& selection = graphs[graph_id].points_sel[point_id];
    if(selection.is_selected) selection.is_selected = false;
    else {
        if(selection.color != color) selection.color = color;
        selection.is_selected = true;
    }
}

const vector<Field::FGraph>& Field::get_graphs() const { return graphs; }