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
    namespace {
        ImColor GRAY_COLOR = {{0.41, 0.41, 0.41, 1}};
        ImColor BLUE_COLOR = {{0.102, 0.601, 0.850, 1}};
    }

    // TASK 1
    void bfs(
        Field& field, int graph_id, int start_point = 0,
        int* total_steps = nullptr, int step = -1
    ) {
        Graph& graph = *field.get_graph(graph_id);
        const int n = graph.connections.size();
        int steps_counter = 0;
        if(n == 0) {
            if(total_steps != nullptr)
                *total_steps = steps_counter;
            return;
        }

        field.disselect_all_edges();
        field.disselect_all_points();
        field.select_point(start_point, graph_id);

        vector<bool> visited(n);
        queue<int> q;
        q.push(start_point);
        visited[start_point] = true;
        field.select_point(
            start_point, graph_id,
            BLUE_COLOR
        );
        if(step == steps_counter)
            return;

        while (!q.empty())
        {
            int buff = q.front();
            field.select_point(
                buff, graph_id,
                GRAY_COLOR
            );

            q.pop();
            for(int i = 0; i < n; i++)
                if(graph.connections[buff][i] && !visited[i]){
                    q.push(i);
                    visited[i] = true;
                    field.select_point(
                        i, graph_id,
                        BLUE_COLOR
                    );

                    // Do some work here
                    field.select_edge(
                        graph.get_edge_id(buff, i),
                        graph_id
                    );
                }

            steps_counter++;
            if(step == steps_counter)
                return;
        }

        steps_counter++;
        field.select_point(start_point, graph_id);
        if(total_steps != nullptr)
            *total_steps = steps_counter;
    }

    // TASK 2
    void _dfs(
        Field& field, Graph& graph, const int& graph_id, const int& n,
        vector<bool>& visited, int from, int parent,
        int& steps_counter, int* total_steps = nullptr, int step = -1
    ) {
        if(step == steps_counter)
            return;

        visited[from] = true;
        field.select_point(
            from, graph_id,
            GRAY_COLOR
        );
        steps_counter++;

        for(int i = 0; i < n; i++)
            if(graph.connections[from][i] && !visited[i]){
                field.select_point(
                    i, graph_id,
                    BLUE_COLOR
                );
                _dfs(
                    field, graph, graph_id, n,
                    visited, i, from,
                    steps_counter, total_steps, step
                );
            }

        // Do some work here
        if(parent != -1)
            field.select_edge(
                graph.get_edge_id(from, parent),
                graph_id
            );
    }
    void dfs(
        Field& field, int graph_id, int start_point = 0,
        int* total_steps = nullptr, int step = -1
    ) {
        Graph& graph = *field.get_graph(graph_id);
        const int n = graph.connections.size();
        int steps_counter = 0;
        if(n == 0) {
            if(total_steps != nullptr)
                *total_steps = steps_counter;
            return;
        }

        field.disselect_all_edges();
        field.disselect_all_points();
        field.select_point(
            start_point, graph_id,
            BLUE_COLOR
        );
        if(step == steps_counter)
            return;

        vector<bool> visited(n);
        _dfs(
            field, graph, graph_id, n,
            visited, start_point, -1,
            steps_counter, total_steps, step
        );

        steps_counter++;
        if(total_steps != nullptr && *total_steps < steps_counter)
            *total_steps = steps_counter;
        if(*total_steps == step || step == -1)
            field.select_point(start_point, graph_id);
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
    namespace {
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
    };
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

    // Task 7
    namespace {
        struct _comparator {
            const vector<int> *distances;
            const SparseGraphView* graph_view;
            const int node;

            _comparator(const vector<int> *distances, const SparseGraphView* graph_view = nullptr, int node = -1):
                distances(distances), graph_view(graph_view), node(node) {}

            bool operator()(int a, const int b) const { 
                if (graph_view != nullptr) {
                    const float ad = distances->at(a) + graph_view->get_distance(a, node);
                    const float bd = distances->at(b) + graph_view->get_distance(b, node);
                    return ad > bd;
                }
                else {
                    return distances->at(a) > distances->at(b);
                }
            }
        };

        int _dijkstra_step(
            SparseGraph& graph, vector<int>& distances, vector<int>& cameFrom,
            priority_queue<int, vector<int>, _comparator>* q, ostream* log
        ) {
            int buff = q->top();
            q->pop();
            for(int i = 0; i < graph.n; i++) {
                if(!graph.is_connected(buff, i)) continue;
                const auto& edge = graph.get_edge(buff, i);
                if(distances[i] == -1 || 
                    distances[i] > distances[buff] + edge.weight
                ) {
                    distances[i] = distances[buff] + edge.weight;
                    cameFrom[i] = buff;
                    q->push(i);
                }
            }
            return buff;
        }
    }
    struct bidirect_result {
        vector<int> path;
        vector<int> checked_nodes;

        bidirect_result(): path(), checked_nodes() {}
        bidirect_result(vector<int> path, vector<int> checked_nodes):
            path(path), checked_nodes(checked_nodes) {}
    };
    bidirect_result bidirect_dijkstra_path(SparseGraph& graph, int from = 0, int to = -1, ostream* log = nullptr) {
        const int n = graph.n;
        if(n == 0) return {};
        if(to == -1) to = n - 1;
        
        set<int> checked_nodes;
        vector<int> distances_from(n, -1), distances_to(n, -1),
            parent_from(n, -1), parent_to(n, -1);
        _comparator from_cmp(&distances_from), to_cmp(&distances_to);
        priority_queue<int, vector<int>, _comparator> q_from(from_cmp), q_to(to_cmp);

        q_from.push(from);
        distances_from[from] = 0;
        q_to.push(to);
        distances_to[to] = 0;
        int middle = -1;
        int buff;
        while(!q_from.empty() && !q_to.empty()) {
            buff = _dijkstra_step(graph, distances_from, parent_from, &q_from, log);
            checked_nodes.insert(buff);
            if (distances_to[buff] != -1) {
                middle = buff;
                break;
            }

            buff = _dijkstra_step(graph, distances_to, parent_to, &q_to, log);
            checked_nodes.insert(buff);
            if (distances_from[buff] != -1) {
                middle = buff;
                break;
            }
        }

        if(middle == -1) return {};
        int prev_d = -1;
        pair<int, int> connection{-1, -1};
        for(int i = 0; i < n; i++)
        for(int j = 0; j < n; j++)
        {
            if(distances_to[j] == -1 || distances_from[i] == -1) continue;
            if(!graph.is_connected(i, j)) continue;
            int cur_d = distances_to[j] + distances_from[i] + graph.get_edge(i, j).weight;
            if(prev_d == -1 || prev_d > cur_d) {
                connection = {i, j};
                prev_d = cur_d;
            }
        }

        if(prev_d == -1) return {};

        vector<int> ret;
        int cur = connection.second;
        while(cur != to) {
            ret.push_back(cur);
            cur = parent_to[cur];
        }
        ret.push_back(to);

        reverse(ret.begin(), ret.end());
        cur = connection.first;
        while(cur != from) {
            ret.push_back(cur);
            cur = parent_from[cur];
        }
        ret.push_back(from);

        reverse(ret.begin(), ret.end());

        if(log != nullptr){
            *log << "Number of checked nodes: " << checked_nodes.size() << "\n";
            *log << "Checked nodes:\n";
            int count = 0;
            for(int i : checked_nodes){
                *log << i << " ";
                count++;
                if(count > 10) *log << "\n";
            }

            *log << "\nResulting path:\n";
            for(int i = 0; i < ret.size(); i++){
                *log << ret[i];
                if(i < ret.size() - 1)
                    *log << " - ";
            }
        }

        return {ret, {checked_nodes.begin(), checked_nodes.end()}};
    }

    // Task 8
    bidirect_result bidirect_astar_path(SparseGraphView& graph_view, int from = 0, int to = -1, ostream* log = nullptr) {
        const int n = graph_view.graph.n;
        if(n == 0) return {};
        if(to == -1) to = n - 1;
        
        set<int> checked_nodes;
        vector<int> distances_from(n, -1), distances_to(n, -1),
            parent_from(n, -1), parent_to(n, -1);
        _comparator from_cmp(&distances_from, &graph_view, from), to_cmp(&distances_to, &graph_view, to);
        priority_queue<int, vector<int>, _comparator> q_from(from_cmp), q_to(to_cmp);

        q_from.push(from);
        distances_from[from] = 0;
        q_to.push(to);
        distances_to[to] = 0;
        int middle = -1;
        int buff;
        while(!q_from.empty() && !q_to.empty()) {
            buff = _dijkstra_step(graph_view.graph, distances_from, parent_from, &q_from, log);
            checked_nodes.insert(buff);
            if (distances_to[buff] != -1) {
                middle = buff;
                break;
            }

            buff = _dijkstra_step(graph_view.graph, distances_to, parent_to, &q_to, log);
            checked_nodes.insert(buff);
            if (distances_from[buff] != -1) {
                middle = buff;
                break;
            }
        }

        if(middle == -1) return {};
        int prev_d = -1;
        pair<int, int> connection{-1, -1};
        for(int i = 0; i < n; i++)
        for(int j = 0; j < n; j++)
        {
            if(distances_to[j] == -1 || distances_from[i] == -1) continue;
            if(!graph_view.graph.is_connected(i, j)) continue;
            int cur_d = distances_to[j] + distances_from[i] + graph_view.graph.get_edge(i, j).weight;
            if(prev_d == -1 || prev_d > cur_d) {
                connection = {i, j};
                prev_d = cur_d;
            }
        }

        if(prev_d == -1) return {};

        vector<int> ret;
        int cur = connection.second;
        while(cur != to) {
            ret.push_back(cur);
            cur = parent_to[cur];
        }
        ret.push_back(to);

        reverse(ret.begin(), ret.end());
        cur = connection.first;
        while(cur != from) {
            ret.push_back(cur);
            cur = parent_from[cur];
        }
        ret.push_back(from);

        reverse(ret.begin(), ret.end());

        if(log != nullptr){
            *log << "Number of checked nodes: " << checked_nodes.size() << "\n";
            *log << "Checked nodes:\n";
            int count = 0;
            for(int i : checked_nodes){
                *log << i << " ";
                count++;
                if(count > 10) *log << "\n";
            }

            *log << "\nResulting path:\n";
            for(int i = 0; i < ret.size(); i++){
                *log << ret[i];
                if(i < ret.size() - 1)
                    *log << " - ";
            }
        }

        return {ret, {checked_nodes.begin(), checked_nodes.end()}};
    }
}