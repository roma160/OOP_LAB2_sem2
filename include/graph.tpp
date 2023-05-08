#pragma once

#include <vector>
#include <set>
#include <string>
#include <sstream>
#include <regex>

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

    string to_string() const {
        stringstream ss;
        for(auto edge : edges)
            ss << edge.first << " " << edge.second << "\n";
        return ss.str();
    }

    static bool from_string(const string& s, Graph& graph) {
        int n = -1;
        set<pair<int, int>> edges;
        stringstream ss(s);
        for (string line; getline(ss, line); ) {
            if(line.empty()) continue;
            static const regex edge("(\\d+)\\s+(\\d+)");
            smatch match;
            if(!regex_search(line, match, edge)) continue;
            if(match.size() != 3) continue;
            pair<int, int> to_push(stoi(match[1]), stoi(match[2]));
            if (to_push.first > to_push.second)
                swap(to_push.first, to_push.second);
            edges.insert(to_push);
            if (n < to_push.first) n = to_push.first;
            if (n < to_push.second) n = to_push.second;
        }
        if(n == -1) return false;
        graph = Graph(n + 1, {edges.begin(), edges.end()});
        return true;
    }
};
