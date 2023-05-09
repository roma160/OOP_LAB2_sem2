#pragma once

#include "field.h"

#include <queue>
#include <stack>
#include <vector>

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
}