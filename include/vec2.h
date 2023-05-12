#pragma once

#include "imgui.h"

#include <string>
#include <sstream>
#include <ostream>

using namespace std;

class Vec2: public ImVec2 {
public:
    Vec2() : ImVec2() {}
    Vec2(ImVec2 vec) : ImVec2(vec) {}
    Vec2(float x, float y): ImVec2(x, y) {}

    inline Vec2 operator+(const Vec2 &b) const { return {x + b.x, y + b.y}; }
    inline Vec2& operator+=(const Vec2 &b) {
        x += b.x; y += b.y;
        return *this;
    }
    inline Vec2 operator-() const { return {-x, -y}; }
    inline Vec2 operator-(const Vec2 &b) const { return {x - b.x, y - b.y}; }

    inline Vec2 operator*(const float b) const { return {x * b, y * b}; }
    inline Vec2 operator*(const Vec2 b) const { return {x * b.x, y * b.y}; }
    inline Vec2 &operator*=(const float b) {
        x *= b; y *= b;
        return *this;
    }
    inline Vec2 operator/(const Vec2 b) const { return {x / b.x, y / b.y}; }
    inline Vec2 operator/(float b) const { return {x / b, y / b}; }

    inline float abs() const { return sqrt(x * x + y * y); }
    inline Vec2 norm() const { return *this / abs(); }

    string to_string() const
    {
        stringstream ss;
        ss << "v(" << x << ", " << y << ")";
        return ss.str();
    }
};