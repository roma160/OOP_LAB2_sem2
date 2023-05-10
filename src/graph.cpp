#include "graph.h"

#include <iostream>

using namespace std;

int min(int a, int b) { return a > b ? b : a; }
int max(int a, int b) { return a > b ? a : b; }

Graph::Connection::Connection(): weight(0), connected(false) {}
Graph::Connection::Connection(bool connected): weight(connected), connected(connected) {}
Graph::Connection::Connection(T weight): weight(weight), connected(true) {}

Graph::Connection::operator bool() const { return connected; }


Graph::Edge::Edge(): first(-1), second(-1), Connection() {}
Graph::Edge::Edge(int first, int second): Edge(first, second, 1) {}
Graph::Edge::Edge(int first, int second, T weight): first(min(first, second)), second(max(first, second)), Connection(weight) {}

// Set insertion fix
bool Graph::Edge::operator<(const Edge& b) const {
    if (first != b.first) return first < b.first;
    return second < b.second;
}

Graph::Graph(): connections(0), edges(0) {}
Graph::Graph(int n): connections(n, vector<Connection>(n)), edges(0) {}
Graph::Graph(vector<vector<Connection>>&& connections): connections(move(connections)), edges(0) {
    for(int i = 0; i < this->connections.size(); i++)
    for(int j = i + 1; j < this->connections[i].size(); j++)
        if(this->connections[i][j]) edges.push_back(
            {i, j, this->connections[i][j].weight});
}
Graph::Graph(int n, vector<Edge>&& edges): connections(n, vector<Connection>(n)), edges(move(edges)) {
    for(auto& edge : this->edges) {
        connections[edge.first][edge.second] = edge.weight;
        connections[edge.second][edge.first] = edge.weight;
    }
}

void Graph::add_edge(int from, int to, int weight) {
    if(from > to) swap(from, to);
    edges.push_back(Edge(from, to, weight));
    connections[from][to] = weight;
    connections[to][from] = weight;
}

void Graph::remove_edge(int edge_id) {
    int from = edges[edge_id].first;
    int to = edges[edge_id].second;
    connections[from][to].connected = false;
    connections[to][from].connected = false;
    edges.erase(edges.begin() + edge_id);
}

int Graph::get_edge_id(int from, int to) const {
    if(from > to) swap(from, to);
    const int m = edges.size();
    for(int i = 0; i < m; i++)
        if(edges[i].first == from && edges[i].second == to)
            return i;
    return -1;
}

bool Graph::includes(const Graph& graph) const {
    if(graph.connections.size() > connections.size()) return false;
    set<Edge> mine(edges.begin(), edges.end());
    for(auto edge : graph.edges)
        if(!mine.count(edge)) return false;
    return true;
}

string Graph::to_string() const {
    stringstream ss;
    for(auto edge : edges)
        ss << edge.first << " " << edge.second << " " << edge.weight << "\n";
    return ss.str();
}

string Graph::to_info_string() const {
    stringstream ss;
    ss<<"Graph(n="<<connections.size()<<"; m="<<edges.size()<<");";
    return ss.str();
}

bool Graph::from_string(const string& s, Graph& graph) {
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
        
        edges.insert(to_push);
        if (n < to_push.first) n = to_push.first;
        if (n < to_push.second) n = to_push.second;
    }
    if(n == -1) return false;
    graph = Graph(n + 1, {edges.begin(), edges.end()});
    return true;
}

SparseGraph::SparseGraph(): n(0), m(0), edges() {}
SparseGraph::SparseGraph(int n, int m): n(n), m(m), edges() {}
SparseGraph::SparseGraph(int n, const vector<Edge>& edges): n(n), m(edges.size()), edges() {
    for(auto edge : edges)
        this->edges[{edge.first, edge.second}] = edge;
}

bool SparseGraph::is_connected(int from, int to) {
    if(from > to) swap(from, to);
    return edges.count({from, to});
}

SparseGraph::Edge& SparseGraph::get_edge(int from, int to) {
    if(from > to) swap(from, to);
    return edges[{from, to}];
}

void SparseGraph::set_edge(Edge edge) {
    if(is_connected(edge.first, edge.second)) {
        if (!edge.connected) {
            edges.erase({edge.first, edge.second});
            m--;
        }
        else edges[{edge.first, edge.second}].weight = edge.weight;
    }
    else if(edge.connected) {
        edges[{edge.first, edge.second}] = edge;
        m++;
    }
}

string SparseGraph::to_string() const {
    stringstream ss;
    for(auto edge : edges) {
        ss << edge.second.first << " " << edge.second.second << " " << edge.second.weight << "\n";
    }
    return ss.str();
}

string SparseGraph::to_info_string() const {
    stringstream ss;
    ss<<"SparseGraph(n="<<n<<"; m="<<m<<");";
    return ss.str();
}

bool SparseGraph::from_string(const string& s, SparseGraph& graph) {
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
        
        edges.insert(to_push);
        if (n < to_push.first) n = to_push.first;
        if (n < to_push.second) n = to_push.second;
    }
    if(n == -1) return false;
    graph = SparseGraph(n + 1, {edges.begin(), edges.end()});
    return true;
}