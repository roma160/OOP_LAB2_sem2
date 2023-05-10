#pragma once

#include <vector>
#include <set>
#include <string>
#include <sstream>
#include <regex>
#include <map>

using namespace std;

struct Graph
{
    typedef int T;
    struct Connection
    {
        T weight;
        bool connected;

        Connection();
        Connection(bool connected);
        Connection(T weight);

        operator bool() const;
    };

    struct Edge: Connection {
        int first;
        int second;

        Edge();
        Edge(int first, int second);
        Edge(int first, int second, T weight);

        // Set insertion fix
        bool operator<(const Edge& b) const;
    };
    

    vector<vector<Connection>> connections;
    vector<Edge> edges;

    Graph();
    Graph(int n);
    Graph(vector<vector<Connection>>&& connections);
    Graph(int n, vector<Edge>&& edges);

    virtual void add_edge(int from, int to, int weight = 1);
    virtual void remove_edge(int edge_id);

    int get_edge_id(int from, int to) const;

    bool includes(const Graph& graph) const;

    string to_string() const;
    string to_info_string() const;
    static bool from_string(const string& s, Graph& graph);
};

class SparseGraph {
public:
    using Edge = Graph::Edge;
    int n;
    int m;

private:
    map<pair<int, int>, Edge> edges;

public:
    SparseGraph();
    SparseGraph(int n, int m = 0);
    SparseGraph(int n, const vector<Edge>& edges);

    bool is_connected(int from, int to);
    Edge& get_edge(int from, int to);

    void set_edge(Edge edge);

    string to_string() const;
    string to_info_string() const;
    static bool from_string(const string& s, SparseGraph& graph);
};