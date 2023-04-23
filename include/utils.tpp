#pragma once

#include <stddef.h>
#include <string>
#include <sstream>
#include <ostream>
#include <chrono>

#include <vector>

using namespace std;

inline double time()
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() / 1000.;
}

template<typename T>
ostream& operator<<(ostream& stream, const vector<T> v){
    stream << "vec{ ";
    for(int i = 0; i < v.size() - 1; i++){
        stream << v[i];
        stream << ", ";
    }
    stream << v[v.size() - 1] << " }";
    return stream;
}

// https://stackoverflow.com/a/5417289/8302811
inline float sign(float a){
    if(a > 0) return 1;
    if(a < 0) return -1;
    return 0;
}

inline int min(int a, int b) { return a > b ? b : a; }