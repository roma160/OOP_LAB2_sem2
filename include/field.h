#pragma once

#include "vec2.h"
#include "linked_list.tpp"
#include "graph.tpp"

#include <stddef.h>
#include <string>
#include <ostream>
#include <cmath>
#include <algorithm>
#include <map>

using namespace std;

class Field {
public:
    struct FEdge: Edge {
        bool valid;
        string first_buff, second_buff;

        FEdge();
        FEdge(int first, int second);
    };

    struct FGraph: Graph<>
    {
        struct Selection {
            bool is_selected;
            ImColor color;

            Selection(bool is_selected = false, ImColor color = ImColor());
        };

        vector<Vec2> points;
        vector<Vec2> speeds;

        vector<Selection> points_sel, edges_sel;

        FGraph(const Graph<>& graph, Vec2 point, float R);
    };

    struct FGraphLink
    {
        int graph_id;
        int node_id;
        FGraphLink(int graph_id, int node_id);
    };

    friend bool operator==(const Field::FGraphLink& a, const Field::FGraphLink& b);
    friend bool operator!=(const Field::FGraphLink& a, const Field::FGraphLink& b);

private:
    bool stop_ticks = false;
    bool show_node_ids = true;

    vector<FGraph> graphs;
    map<int, map<int, linked_list_root<FGraphLink>>> field;

    pair<int, int> get_field_index(const Vec2& point) const;

    void remove_from_field(const pair<int, int>& field_index, FGraphLink point_index);
    void add_to_field(const pair<int, int>& field_index, FGraphLink point_index);
    void recalculate_cell_for_point(const pair<int, int>& old_field_index, FGraphLink point_index);

public:
    const float cell_size;

    Field(float cell_size);

    int add_graph(const Graph<>& graph, Vec2 point = {100, 100}, float R = 100);

    static Vec2 def_compute_force(Vec2 delta, float force_distance, bool connected);
    void do_tick(float dt, Vec2 (*force_function)(Vec2, float, bool) = &def_compute_force);

    void display_window();

    void toggle_point_select(int point_id, int graph_id = 0, ImColor color = ImColor(1.0f, 0.0f, 0.0f, 1.0f));
    void select_point(int point_id, int graph_id = 0, ImColor color = ImColor(1.0f, 0.0f, 0.0f, 1.0f));
    void disselect_point(int point_id, int graph_id = 0);

    const vector<FGraph>& get_graphs() const;
};
