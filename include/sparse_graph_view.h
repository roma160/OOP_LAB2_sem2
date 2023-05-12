#pragma once

#include "graph.h"
#include "vec2.h"

using namespace std;

class SparseGraphView {
public:
    SparseGraph graph;
    vector<Vec2> coordinates;
    Vec2 bounds;

    SparseGraphView();

    bool load_graph(string data, const int first_node_index);

    void show_window();
};