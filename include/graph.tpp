#pragma once

#include <vector>
#include <set>
#include <string>
#include <sstream>
#include <regex>

using namespace std;

struct Graph
{
    typedef int T;
    struct Connection
    {
        T weight;
        bool connected;

        Connection(): weight(0), connected(false) {}
        Connection(bool connected): weight(connected), connected(connected) {}
        Connection(T weight): weight(weight), connected(true) {}

        operator bool() const { return connected; }
    };

    struct Edge: Connection {
        int first;
        int second;

        Edge(): first(-1), second(-1), Connection() {}
        Edge(int first, int second): first(first), second(second), Connection(1) {}
        Edge(int first, int second, T weight): first(first), second(second), Connection(weight) {}
    };
    

    vector<vector<Connection>> connections;
    vector<Edge> edges;

    Graph(): connections(0), edges(0) {}
    Graph(int n): connections(n, vector<Connection>(n)), edges(0) {}
    Graph(vector<vector<Connection>>&& connections): connections(move(connections)), edges(0) {
        for(int i = 0; i < this->connections.size(); i++)
        for(int j = i + 1; j < this->connections[i].size(); j++)
            if(this->connections[i][j]) edges.push_back({i, j});
    }
    Graph(int n, vector<Edge>&& edges): connections(n, vector<Connection>(n)), edges(move(edges)) {
        for(auto& edge : this->edges) {
            connections[edge.first][edge.second] = true;
            connections[edge.second][edge.first] = true;
        }
    }

    string to_string() const {
        stringstream ss;
        for(auto edge : edges)
            ss << edge.first << " " << edge.second << " " << edge.weight << "\n";
        return ss.str();
    }

    static bool from_string(const string& s, Graph& graph) {
        int n = -1;
        set<Edge> edges;
        stringstream ss(s);
        for (string line; getline(ss, line); ) {
            if(line.empty()) continue;
            static const regex edge("(\\d+)\\s+(\\d+)\\s*(\\d*)");
            smatch match;
            if(!regex_search(line, match, edge)) continue;
            if(match.size() != 3 && match.size() != 4) continue;

            int weight;
            if(match[3].str().empty()) weight = 1;
            else weight = stoi(match[3]);
            Edge to_push = {stoi(match[1]), stoi(match[2]), weight};
            
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

// template <typename T>
// class EdgesList {
// private:
//     map<pair<int, int>, T> list;

//     pair<int, int> format_from_to(int from, int to) {
//         if (from > to) swap(from, to);
//         return {from, to};
//     }

// public:
//     EdgesList(): list() {}

//     T& get(int from, int to) {
//         return list[format_from_to(from, to)];
//     }

//     bool contains(int from, int to) {
//         return list.count(format_from_to(from, to)) > 0;
//     }
// };