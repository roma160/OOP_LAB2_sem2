#pragma once

#include "imgui.h"
#include "imgui_internal.h"

#include "vec2.tpp"
#include "linked_list.tpp"

#include <stddef.h>
#include <string>
#include <ostream>
#include <cmath>
#include <algorithm>
#include <map>

using namespace std;

struct Field {
    const float cell_size;
    vector<Vec2> points;
    vector<Vec2> speeds;
    map<int, map<int, linked_list_root<int>>> field;

    Field(float cell_size): cell_size(cell_size), points(), speeds() {}
    
    static float sign(float a){
        if(a > 0) return 1;
        if(a < 0) return -1;
        return 0;
    }

    static int min(int a, int b) { return a > b ? b : a; }

    pair<int, int> get_field_index(const Vec2& point) const {
        return {
            static_cast<int>(point.x / cell_size),
            static_cast<int>(point.y / cell_size)
        };
    }

    void remove_from_field(const pair<int, int>& field_index, int point_index) {
        const Vec2& point = points[point_index];
        field[field_index.first][field_index.second].remove_val(point_index);
    }

    void add_to_field(const pair<int, int>& field_index, int point_index) {
        auto &v = field[field_index.first][field_index.second];
        field[field_index.first][field_index.second].push_back(point_index);
    }

    void add_points(vector<Vec2> new_points) {
        const int prev_size = points.size();
        points.resize(points.size() + new_points.size());
        speeds.resize(points.size());
        copy(new_points.begin(), new_points.end(), points.begin() + prev_size);
        for(int i = 0; i < new_points.size(); i++){
            const auto field_index = get_field_index(points[i + prev_size]);
            add_to_field(field_index, i + prev_size);
        }
    }

    void do_tick(float dt){
        vector<Vec2> forces(points.size());
        for(int i = 0; i < points.size(); i++){
            Vec2 &point = points[i];
            const auto field_index = get_field_index(point);
            for (int dx = -1; dx <= 1; dx++)
            for (int dy = -1; dy <= 1; dy++) {
                const auto& list = field[field_index.first + dx][field_index.second + dy];
                if(list.size == 0) continue;
                auto buff = list.first;
                for(; buff != nullptr; buff = buff->next){
                    if (buff->val == i) continue;
                    const Vec2 &point_b = points[buff->val];
                    const Vec2 delta = point_b - point;
                    const float delta_abs = (delta.abs() - cell_size / 2) / 200;
                    forces[i] += delta.norm() * 2000 * delta_abs * exp(-delta_abs);
                }
            }
        }

        for(int i = 0; i < points.size(); i++){
            Vec2 &point = points[i];
            Vec2 &speed = speeds[i];
            const auto field_index = get_field_index(point);
            speed += forces[i] * dt;
            speed *= 0.95;

            const static float max_speed = 1e3;
            const float abs_speed = speed.abs();
            if (abs_speed > max_speed) speed *= max_speed / abs_speed;
            if (abs_speed < 0.5) speed = {0, 0};

            point += speed * dt;

            recalculate_cell_for_point(field_index, i);
        }
    }

    // const int sizes_sum() const {
    //     int sum = 0;
    //     for (auto i = field.begin(); i != field.end(); i++){
    //         for(auto j = i->second.begin(); j != i->second.end(); j++){
    //             sum += j->second.size;
    //         }
    //     }
    //     return sum;
    // }

    void recalculate_cell_for_point(const pair<int, int>& old_field_index, int point_index){
        const auto new_field_index = get_field_index(points[point_index]);
        if(old_field_index != new_field_index){
            remove_from_field(old_field_index, point_index);
            add_to_field(new_field_index, point_index);
        }
    }

    void draw(){
        const float R = 18;
        static int selected = -1;

        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 20, main_viewport->WorkPos.y + 20), ImGuiCond_Appearing);
        ImGui::Begin("Field");
        ImGui::SetWindowSize({500, 500}, ImGuiCond_Appearing);
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
        if (selected == -1 && ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            for (int i = 0; i < points.size() && selected == -1; i++)
                if ((points[i] + p - mouse).abs() <= R)
                selected = i;
        }
        if (selected != -1 && ImGui::IsMouseDown(ImGuiMouseButton_Left)){
            const auto old_field_index = get_field_index(points[selected]);
            points[selected] = mouse - p;
            recalculate_cell_for_point(old_field_index, selected);
        }
        else if (selected != -1 && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
            selected = -1;

        const ImU32 col = ImColor(1.0f, 1.0f, 0.4f, 1.0f);
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        static bool debug_field = false;
        ImGui::SetCursorPos(cursor);
        ImGui::Checkbox("Enable field debugging", &debug_field);
        for (int i = 0; i < points.size(); i++)
        {
            draw_list->AddNgonFilled(p + points[i], R, col, 36);
            if(debug_field){
                string num = to_string(i);
                draw_list->AddText(p + points[i] - Vec2{5, 5}, ImColor(0, 0, 255), num.c_str());
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
};
