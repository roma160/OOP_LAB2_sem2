#pragma once

#include <vector>

using namespace std;

struct Edge
{
    int first, second;

    Edge(): first(0), second(0) {}
    Edge(int first, int second): first(first), second(second) {
        if(first > second) swap(this->first, this->second);
    }
};

template<typename G = Edge>
struct Graph
{
    vector<vector<bool>> connections;
    vector<G> edges;

    Graph(): connections(0), edges(0) {}
    Graph(int n): connections(n, vector<bool>(n)), edges(0) {}
    Graph(vector<vector<bool>>&& connections): connections(move(connections)), edges(0) {
        for(int i = 0; i < this->connections.size(); i++)
        for(int j = i + 1; j < this->connections[i].size(); j++)
            if(this->connections[i][j]) edges.push_back({i, j});
    }
    Graph(int n, vector<G>&& edges): connections(n, vector<bool>(n)), edges(move(edges)) {
        for(auto& edge : this->edges) {
            connections[edge.first][edge.second] = true;
            connections[edge.second][edge.first] = true;
        }
    }
};
