#pragma once

#include <vector>

using namespace std;

struct Graph
{
    vector<vector<bool>> connections;
    vector<pair<int, int>> edges;

    Graph(): connections(0), edges(0) {}
    Graph(int n): connections(n, vector<bool>(n)), edges(0) {}
    Graph(vector<vector<bool>>&& connections): connections(move(connections)), edges(0) { 
        //this->connections = move(connections);
        for(int i = 0; i < connections.size(); i++)
        for(int j = i + 1; j < connections[i].size(); j++)
            if(connections[i][j]) edges.push_back({i, j});
    }
    Graph(int n, vector<pair<int, int>>&& edges): connections(n, vector<bool>(n)), edges(move(edges)) {
        for(auto edge : edges) {
            connections[edge.first][edge.second] = true;
            connections[edge.second][edge.first] = true;
        }
    }
};
