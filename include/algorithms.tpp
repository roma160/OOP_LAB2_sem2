#pragma once

#include "field.h"

#include <queue>
#include <stack>
#include <vector>
#include <algorithm>
#include <iostream>
#include <ostream>

using namespace std;

namespace algos {
    // TASK 1
    void bfs(Field& field, int graph_id, int start_point = 0) {
        Graph& graph = *field.get_graph(graph_id);
        const int n = graph.connections.size();
        if(n == 0) return;

        field.disselect_all_edges();
        field.disselect_all_points();
        field.select_point(start_point, graph_id);

        vector<bool> visited(n);
        queue<int> q;
        q.push(start_point);
        visited[start_point] = true;
        while (!q.empty())
        {
            int buff = q.front();
            q.pop();
            for(int i = 0; i < n; i++)
                if(graph.connections[buff][i] && !visited[i]){
                    q.push(i);
                    visited[i] = true;

                    // Do some work here
                    field.select_edge(
                        graph.get_edge_id(buff, i),
                        graph_id
                    );
                }
        }
    }

    // TASK 2
    void _dfs(Field& field, Graph& graph, const int& graph_id, const int& n, vector<bool>& visited, int from, int parent) {
        visited[from] = true;
        for(int i = 0; i < n; i++)
            if(graph.connections[from][i] && !visited[i]){
                _dfs(field, graph, graph_id, n, visited, i, from);
            }

        // Do some work here
        if(parent != -1)
            field.select_edge(
                graph.get_edge_id(from, parent),
                graph_id
            );
    }
    void dfs(Field& field, int graph_id, int start_point = 0) {
        Graph& graph = *field.get_graph(graph_id);
        const int n = graph.connections.size();
        if(n == 0) return;

        field.disselect_all_edges();
        field.disselect_all_points();
        field.select_point(start_point, graph_id);

        vector<bool> visited(n);
        _dfs(field, graph, graph_id, n, visited, start_point, -1);
    }

    // Task 3
    bool _pairs_descending(pair<int, int> a, pair<int, int> b)
    { return a.first > b.first; }
    Graph prims_min_tree(Graph& graph, int start_point = 0) {
        const int n = graph.connections.size();
        if(n == 0) return Graph();

        Graph ret(n);
        vector<bool> visited(n);
        stack<pair<int, int>> q;
        q.push({-1, start_point});
        while (!q.empty())
        {
            auto buff = q.top();
            q.pop();
            int from = buff.first;
            int to = buff.second;
            if(visited[to]) continue;
            visited[to] = true;
            if(from != -1) ret.add_edge(from, to);

            vector<pair<int, int>> possible;
            for(int i = 0; i < n; i++)
                if(graph.connections[to][i] && !visited[i]){
                    possible.push_back({graph.connections[to][i].weight, i});
                }
            sort(possible.begin(), possible.end(), _pairs_descending);
            for(auto p : possible) q.push({to, p.second});
        }

        return ret;
    }

    // Task 4
    vector<int> dijkstra_path(Graph& graph, int from = 0, int to = -1) {
        const int n = graph.connections.size();
        if(n == 0) return vector<int>();
        if(to == -1) to = n - 1;

        struct segment {
            vector<int> path;
            int length;

            segment(): path(), length(-1) {}
            segment(vector<int> path, int length): path(path), length(length) {}
        };
        vector<segment> distances(n);
        queue<int> q;
        q.push(from);
        distances[from] = {{from}, 0};
        while(!q.empty()) {
            int buff = q.front();
            q.pop();
            for(int i = 0; i < n; i++)
                if(graph.connections[i][buff] && (
                    distances[i].length == -1 || 
                    distances[i].length > distances[buff].length + 
                    graph.connections[i][buff].weight)
                ) {
                    distances[i].length = distances[buff].length + 
                        graph.connections[i][buff].weight;
                    distances[i].path = distances[buff].path;
                    distances[i].path.push_back(i);
                    q.push(i);
                }
        }

        return distances[to].path;
    }

    // Task 5
    vector<int> astar_path(Field& field, int graph_id = 0, int from = 0, int to = -1, ostream* log = nullptr) {
        const Graph& graph = *field.get_graph(graph_id);
        const int n = graph.connections.size();
        if(n == 0) return vector<int>();
        if(to == -1) to = n - 1;
        
        vector<float> distances(n, -1);
        vector<int> cameFrom(n, -1);
        auto cmp = [&](int a, int b) { 
            float ad = distances[a] + field.get_field_distance(graph_id, a, to);
            float bd = distances[b] + field.get_field_distance(graph_id, b, to);
            return ad > bd; 
        };
        priority_queue<int, vector<int>, decltype(cmp)> q(cmp);
        distances[from] = 0;
        q.push(from);
        while(!q.empty()) {
            int buff = q.top();
            if(buff == to) break;
            if(log != nullptr)
                *log<<"Node: "<<buff<<" (d="<<distances[buff]<<")\n";
            q.pop();
            for(int i = 0; i < n; i++)
                if(graph.connections[i][buff] && (
                    distances[i] == -1 || 
                    distances[i] > distances[buff] + 
                    field.get_field_distance(graph_id, i, buff))
                ) {
                    distances[i] = distances[buff] + 
                        field.get_field_distance(graph_id, i, buff);
                    cameFrom[i] = buff;
                    q.push(i);
                }
        }

        vector<int> ret;
        int last = to;
        ret.push_back(to);
        while(last != from) {
            last = cameFrom[last];
            ret.push_back(last);
        }
        return ret;
    }

    // Task 6
    bool _ff_bfs(Graph& graph, int from, int to, vector<int>& comeFrom) {
        const int n = graph.connections.size();
        vector<bool> visited(n);
        visited[from] = true;
        queue<int> q;
        q.push(from);
        while(!q.empty()) {
            int buff = q.front();
            q.pop();
            for(int i = 0; i < n; i++)
                if(graph.connections[buff][i].connected && 
                    graph.connections[buff][i].weight > 0 &&
                    !visited[i]
                ){
                    q.push(i);
                    visited[i] = true;
                    comeFrom[i] = buff;
                }
        }
        return visited[to];
    }
    struct FFMaxFlowRes {
        int maxFlow;
        Graph flowData;

        FFMaxFlowRes(Graph flowData): maxFlow(-1), flowData(move(flowData)) {}
    };
    FFMaxFlowRes ff_max_flow(Field& field, int graph_id = 0, int from = 0, int to = -1) {
        FFMaxFlowRes res(*field.get_graph(graph_id));
        Graph& graph = res.flowData;
        const int n = graph.connections.size();
        if(n == 0) return res;
        if(to == -1) to = n - 1;

        vector<int> comeFrom(n);
        res.maxFlow = 0;

        while(_ff_bfs(graph, from, to, comeFrom)) {
            int pathFlow = -1;
            int i = to;
            while(i != from) {
                if(pathFlow == -1 || pathFlow > graph.connections[comeFrom[i]][i].weight)
                    pathFlow = graph.connections[comeFrom[i]][i].weight;
                i = comeFrom[i];
            }

            res.maxFlow += pathFlow;

            i = to;
            while(i != from) {
                int buff = comeFrom[i];
                graph.connections[buff][i].weight -= pathFlow;
                graph.connections[i][buff].weight += pathFlow;
                i = buff;
            }
        }

        return res;
    }
}