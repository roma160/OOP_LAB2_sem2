#pragma once

#include "vec2.h"
#include "linked_list.tpp"
#include "graph.h"

#include <stddef.h>
#include <string>
#include <ostream>
#include <cmath>
#include <algorithm>
#include <map>

using namespace std;

class Field {
private:
    const static Vec2 DEF_GRAPH_LOC;
    const static float DEF_GRAPH_R;

public:
    struct FGraph: Graph
    {
        struct Selection {
            bool is_selected;
            ImColor color;

            Selection(bool is_selected = false, ImColor color = ImColor());
        };

        vector<Vec2> points;
        vector<Vec2> speeds;

        vector<Selection> points_sel, edges_sel;
        vector<string> edges_anno;

        FGraph(const Graph& graph, Vec2 point = DEF_GRAPH_LOC, float R = DEF_GRAPH_R);

        void reset_points_pos(Vec2 point = DEF_GRAPH_LOC, float R = DEF_GRAPH_R);

        void add_edge(int from, int to, int weight = 1) override;
        void remove_edge(int edge_id) override;

        void clear_annotations();
    };

    struct FGraphLink
    {
        int graph_id;
        int index;
        FGraphLink(int graph_id, int index);
    };

    friend bool operator==(const Field::FGraphLink& a, const Field::FGraphLink& b);
    friend bool operator!=(const Field::FGraphLink& a, const Field::FGraphLink& b);

    enum ForceType { Node, ConnectedNode, UpBound, DownBound, LeftBound, RightBound };

private:
    bool use_ticks = true;
    bool show_node_ids = true;
    bool show_edge_weights = true;
    bool show_only_selected_edges = false;
    bool show_actual_distance = false;
    bool bound_forces = true;
    float scale = 1.0f;

    bool was_middle_mouse = false;
    Vec2 middle_mouse_prev;
    Vec2 middle_mouse_shift;

    vector<FGraph> graphs;
    map<int, map<int, linked_list_root<FGraphLink>>> field;

    pair<int, int> get_field_index(const Vec2& point) const;

    void remove_from_field(const pair<int, int>& field_index, FGraphLink point_index);
    void add_to_field(const pair<int, int>& field_index, FGraphLink point_index);
    void recalculate_cell_for_point(const pair<int, int>& old_field_index, FGraphLink point_index);

public:
    const float cell_size;
    Vec2 bounds;

    Field(float cell_size, Vec2 bounds = Vec2{500, 500});

    int add_graph(const Graph& graph, Vec2 point = DEF_GRAPH_LOC, float R = DEF_GRAPH_R);
    void remove_graph(int index);

    static Vec2 def_compute_force(Vec2 delta, float force_distance, ForceType type);
    void do_tick(float dt, Vec2 (*force_function)(Vec2, float, ForceType) = &def_compute_force);

    void display_window();

    void toggle_point_select(int point_id, int graph_id = 0, ImColor color = ImColor(1.0f, 0.0f, 0.0f, 1.0f));
    void select_point(int point_id, int graph_id = 0, ImColor color = ImColor(1.0f, 0.0f, 0.0f, 1.0f));
    void disselect_point(int point_id, int graph_id = 0);
    void disselect_all_points(int graph_id = 0);

    void toggle_edge_select(int edge_id, int graph_id = 0, ImColor color = ImColor(1.0f, 0.0f, 0.0f, 1.0f));
    void select_edge(int edge_id, int graph_id = 0, ImColor color = ImColor(1.0f, 0.0f, 0.0f, 1.0f));
    void disselect_edge(int edge_id, int graph_id = 0);
    void disselect_all_edges(int graph_id = 0);

    const vector<FGraph>& get_graphs() const;
    FGraph* get_graph(int graph_index);

    float get_field_distance(int graph_id, int from_id, int to_id);

    void set_show_actual_distance(bool val);
};
