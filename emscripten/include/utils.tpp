#pragma once

#include <stddef.h>
#include <string>
#include <sstream>
#include <ostream>
#include <fstream>
#include <chrono>

#include <vector>


#define M_PI 3.14159265358979323846


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
inline int max(int a, int b) { return a > b ? a : b; }

inline string read_file(const string filename) {
    ifstream file(filename);
    string ret, buff;
    while(getline(file, buff))
        ret += buff + "\n";
    file.close();
    return ret;
}