#include "vec2.h"

ostream& operator<<(ostream& stream, const Vec2& vec){
    stream << vec.to_string();
    return stream;
}