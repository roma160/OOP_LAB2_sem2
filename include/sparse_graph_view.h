#pragma once

#include "graph.h"
#include "vec2.h"

using namespace std;

class SparseGraphView {
private:
    vector<int> current_path;
public:
    SparseGraph graph;
    vector<Vec2> coordinates;
    Vec2 bounds;

    SparseGraphView();

    bool load_graph(string data, const int first_node_index = 1);

    void show_window();

    void set_current_path(vector<int> new_path);
};