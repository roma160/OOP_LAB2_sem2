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

    Vec2 operator+(const Vec2 &b) const { return {x + b.x, y + b.y}; }
    Vec2& operator+=(const Vec2 &b) {
        x += b.x; y += b.y;
        return *this;
    }
    Vec2 operator-() const { return {-x, -y}; }
    Vec2 operator-(const Vec2 &b) const { return {x - b.x, y - b.y}; }

    Vec2 operator*(const float b) const { return {x * b, y * b}; }
    Vec2 &operator*=(const float b) {
        x *= b; y *= b;
        return *this;
    }
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

ostream& operator<<(ostream& stream, const Vec2& vec){
    stream << vec.to_string();
    return stream;
}