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

        // Set insertion fix
        bool operator<(const Edge& b) const {
            if (first != b.first) return first < b.first;
            if (second != b.second) return second < b.second;
            return weight < b.weight;
        }
    };
    

    vector<vector<Connection>> connections;
    vector<Edge> edges;

    Graph(): connections(0), edges(0) {}
    Graph(int n): connections(n, vector<Connection>(n)), edges(0) {}
    Graph(vector<vector<Connection>>&& connections): connections(move(connections)), edges(0) {
        for(int i = 0; i < this->connections.size(); i++)
        for(int j = i + 1; j < this->connections[i].size(); j++)
            if(this->connections[i][j]) edges.push_back(
                {i, j, this->connections[i][j].weight});
    }
    Graph(int n, vector<Edge>&& edges): connections(n, vector<Connection>(n)), edges(move(edges)) {
        for(auto& edge : this->edges) {
            connections[edge.first][edge.second] = edge.weight;
            connections[edge.second][edge.first] = edge.weight;
        }
    }

    virtual void add_edge(int from, int to, int weight = 1) {
        if(from > to) swap(from, to);
        edges.push_back(Edge(from, to, weight));
        connections[from][to] = weight;
        connections[to][from] = weight;
    }

    virtual void remove_edge(int edge_id) {
        int from = edges[edge_id].first;
        int to = edges[edge_id].second;
        connections[from][to].connected = false;
        connections[to][from].connected = false;
        edges.erase(edges.begin() + edge_id);
    }

    int get_edge_id(int from, int to) const {
        if(from > to) swap(from, to);
        const int m = edges.size();
        for(int i = 0; i < m; i++)
            if(edges[i].first == from && edges[i].second == to)
                return i;
        return -1;
    }

    bool includes(const Graph& graph) const {
        if(graph.connections.size() > connections.size()) return false;
        set<Edge> mine(edges.begin(), edges.end());
        for(auto edge : graph.edges)
            if(!mine.count(edge)) return false;
        return true;
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