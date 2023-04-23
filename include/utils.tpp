#pragma once

#include "vec2.tpp"

#include <stddef.h>
#include <string>
#include <sstream>
#include <ostream>
#include <chrono>
#include <vector>

using namespace std;

static double time()
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