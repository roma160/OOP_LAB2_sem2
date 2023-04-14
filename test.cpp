#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <algorithm>

using namespace std;

struct ImVec2{
    float x, y;
    ImVec2(float x, float y): x(x), y(y) {}
    ImVec2() : x(0), y(0) {}
};

class Vec2 : public ImVec2
{
public:
    Vec2() : ImVec2() {}
    Vec2(ImVec2 vec) : ImVec2(vec) {}
    Vec2(float x, float y) : ImVec2(x, y) {}

    Vec2 operator+(const Vec2 &b) const { return {x + b.x, y + b.y}; }
    Vec2 &operator+=(const Vec2 &b)
    {
        x += b.x;
        y += b.y;
        return *this;
    }
    Vec2 operator-() const { return {-x, -y}; }
    Vec2 operator-(const Vec2 &b) const { return {x - b.x, y - b.y}; }

    Vec2 operator*(float b) const { return {x * b, y * b}; }
    Vec2 operator/(float b) const { return {x / b, y / b}; }

    float abs() const { return sqrt(x * x + y * y); }
    Vec2 norm() const { return *this / abs(); }

    string to_string() const
    {
        stringstream ss;
        ss << "v(" << x << ", " << y << ")";
        return ss.str();
    }
};

struct Field
{
    const float cell_size;
    vector<Vec2> points;
    vector<Vec2> speed;
    vector<vector<vector<int>>> field;

    int compute_cell_num(float size) { return ceil(size / cell_size); }

    Field(float cell_size, Vec2 cur_size) : cell_size(cell_size)
    {
        field = {
            (const unsigned long) compute_cell_num(cur_size.x),
            vector<vector<int>>(
                compute_cell_num(cur_size.y),
                vector<int>())};
        points = vector<Vec2>();
    }

    static float sign(float a)
    {
        if (a > 0)
            return 1;
        if (a < 0)
            return -1;
        return 0;
    }

    static int min(int a, int b) { return a > b ? b : a; }

    pair<int, int> get_field_index(const Vec2 &point) const
    {
        int x = min((int)floor(point.x / cell_size), field.size());
        int y = min((int)floor(point.y / cell_size), field[0].size());
        return {x, y};
    }

    void remove_from_field(const pair<int, int> &field_index, int point_index)
    {
        const Vec2 &point = points[point_index];
        auto &v = field[field_index.first][field_index.second];
        v.erase(find(v.begin(), v.end(), point_index));
    }

    void add_to_field(const pair<int, int> &field_index, int point_index)
    {
        auto &v = field[field_index.first][field_index.second];
        field[field_index.first][field_index.second].push_back(point_index);
    }

    void add_points(vector<Vec2> new_points)
    {
        const int prev_size = points.size();
        points.resize(points.size() + new_points.size());
        copy(new_points.begin(), new_points.end(), points.begin() + prev_size);
        for (int i = 0; i < new_points.size(); i++)
        {
            const auto field_index = get_field_index(points[i + prev_size]);
            add_to_field(field_index, i + prev_size);
        }
    }

    void do_tick(float dt)
    {
        vector<Vec2> forces(points.size());
        for (int i = 0; i < points.size(); i++)
        {
            Vec2 &point = points[i];
            const auto field_index = get_field_index(point);
            for (int dx = -1; dx <= 1; dx++)
                for (int dy = -1; dy <= 1; dy++)
                {
                    if (field_index.first + dx < 0 || field_index.second + dy < 0 ||
                        field_index.first + dx >= field.size() || field_index.second + dy >= field[0].size())
                        continue;
                    auto &v = field[field_index.first + dx][field_index.second + dy];
                    for (int j : v)
                    {
                        const Vec2 &point_b = points[j];
                        const Vec2 delta = point_b - point;
                        const float delta_abs = delta.abs() - cell_size / 2;
                        forces[i] += delta.norm() * (sign(delta_abs) * pow(delta_abs, 2));
                    }
                }
        }

        for (int i = 0; i < points.size(); i++)
        {
            Vec2 &point = points[i];
            const auto field_index = get_field_index(point);
            point += forces[i] * dt;
            const auto new_field_index = get_field_index(point);
            if (field_index != new_field_index)
            {
                remove_from_field(field_index, i);
                add_to_field(new_field_index, i);
            }
        }
    }
};

int main()
{
    Field field(100, {500, 500});
    field.add_to_field({0, 0}, 0);
    field.add_points({
        {100, 100}, {200, 100}, {300, 100},
        {200, 100}, {200, 200}, {300, 200}
    });
    field.add_to_field({0, 0}, 0);

    // vector<vector<vector<int>>> field = {
    //     10, {
    //         10, vector<int>()
    //     }
    // };
    // field[0][0].push_back(0);
    return 0;
}