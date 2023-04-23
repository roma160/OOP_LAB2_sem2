#pragma once

#include "vec2.tpp"
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
private:
    vector<Vec2> points;
    vector<Vec2> speeds;
    map<int, map<int, linked_list_root<int>>> field;

public:
    const float cell_size;

    Field(float cell_size): cell_size(cell_size), points(), speeds() {}

    pair<int, int> get_field_index(const Vec2& point) const;

    void remove_from_field(const pair<int, int>& field_index, int point_index);
    void add_to_field(const pair<int, int>& field_index, int point_index);

    void add_points(vector<Vec2> new_points);

    void do_tick(float dt);
    void recalculate_cell_for_point(const pair<int, int>& old_field_index, int point_index);

    void draw();
};
