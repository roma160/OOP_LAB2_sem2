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
        for(int i = 0; i < this->connections.size(); i++)
        for(int j = i + 1; j < this->connections[i].size(); j++)
            if(this->connections[i][j]) edges.push_back({i, j});
    }
    Graph(int n, vector<pair<int, int>>&& edges): connections(n, vector<bool>(n)), edges(move(edges)) {
        for(auto& edge : this->edges) {
            connections[edge.first][edge.second] = true;
            connections[edge.second][edge.first] = true;
        }
    }

    void toggle_edge(int from, int to, bool to_connect) {
        if(connections[from][to] == to_connect) return;
        connections[from][to] = to_connect;

        if(!to_connect){
            for(int i = 0; i < edges.size(); i++)
                if((edges[i].first == from && edges[i].second == to) || 
                    (edges[i].second == from && edges[i].first == to)){
                    edges.emplace(edges.begin() + i);
                    break;
                }
        }
        else {

        }
    }
};
