#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <iostream>
#include <vector>
#include <limits>
#include <unordered_map>
#include <queue>

class PathFinder {
public:
    struct Edge {
        int target;
        int weight;
    };

    struct Node {
        int id;
        float x;
        float y;
    };

    using Graph = std::unordered_map<int, std::vector<Edge>>;
    using NodeMap = std::unordered_map<int, Node>;
    using MinHeap = std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>>;

    int loadGraph() {
        sqlite3* db;
        sqlite3_stmt* stmt;
        nodes.clear();
        graph.clear();

        rc = sqlite3_open("/sd/graph-all.db", &db);
        if (rc != SQLITE_OK) {
            std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
            return rc;
        }

        const auto nodeQuery = "SELECT id, x, y FROM nodes";
        rc = sqlite3_prepare_v2(db, nodeQuery, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to fetch nodes: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return rc;
        }

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const auto x = static_cast<float>(sqlite3_column_double(stmt, 1));
            const auto y = static_cast<float>(sqlite3_column_double(stmt, 2));
            nodes[id] = {id, x, y};
        }
        sqlite3_finalize(stmt);
        print("->50:PathFinder.h nodes:", nodes.size());

        const auto edgeQuery = "SELECT source, target, weight FROM edges";
        rc = sqlite3_prepare_v2(db, edgeQuery, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to fetch edges: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return rc;
        }

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int source = sqlite3_column_int(stmt, 0);
            int target = sqlite3_column_int(stmt, 1);
            int weight = sqlite3_column_int(stmt, 2);
            graph[source].push_back({target, weight});
        }
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        print("->67:PathFinder.h graph:", graph.size());

        return SQLITE_OK;
    }

    int findPath(const Location start, const Location end) {
        const int startNode = findNearestNode(start);
        const int endNode = findNearestNode(end);

        if (startNode == -1 || endNode == -1) {
            std::cerr << "Start or end  node not found." << std::endl;
            return 0;
        }

        previous.clear();
        path.clear();
        distances.clear();
        nodePath.clear();
        using MinHeap = std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>>;
        MinHeap min_heap;

        for (const auto& node : graph) {
            distances[node.first] = std::numeric_limits<int>::max();
            previous[node.first] = -1;
        }
        distances[startNode] = 0;
        min_heap.push({0, startNode});

        while (!min_heap.empty()) {
            int u = min_heap.top().second;
            min_heap.pop();

            if (u == endNode) break;

            for (const Edge& edge : graph.at(u)) {
                int v = edge.target;
                const int weight = edge.weight;
                int new_distance = distances[u] + weight;

                if (new_distance < distances[v]) {
                    distances[v] = new_distance;
                    previous[v] = u;
                    min_heap.push({new_distance, v});
                }
            }
        }

        for (int at = endNode; at != -1; at = previous[at]) {
            nodePath.push_back(at);
        }
        std::reverse(nodePath.begin(), nodePath.end());

        distance = 0.0;
        float px = 0;
        float py = 0;
        for (int nodeId : nodePath) {
            const Node& node = nodes[nodeId];
            path.push_back({node.x, node.y});
            if (px != 0) distance += realDistance(px, py, node.x, node.y);
            px = node.x;
            py = node.y;
        }
        print("->126:PathFinder.h distance:", distance);
        return nodePath.size();
    }

    static float euclideanDistance(const float x1, const float y1, const float x2, const float y2) {
        return std::sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    }

    static float realDistance(const float lon1, const float lat1, const float lon2, const float lat2) {
        constexpr float R = 6371.0;
        const float dLat = (lat2 - lat1) * M_PI / 180.0;
        const float dLon = (lon2 - lon1) * M_PI / 180.0;
        const float a = std::sin(dLat / 2) * std::sin(dLat / 2) +
            std::cos(lat1 * M_PI / 180.0) * std::cos(lat2 * M_PI / 180.0) *
            std::sin(dLon / 2) * std::sin(dLon / 2);
        const float c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
        return R * c;
    }

    int findNearestNode(const Location loc) const {
        int nearestNodeId = -1;
        float minDistance = std::numeric_limits<float>::max();

        for (const auto& kv : nodes) {
            const Node& node = kv.second;
            const float distance = euclideanDistance(loc.lon, loc.lat, node.x, node.y);
            if (distance < minDistance) {
                minDistance = distance;
                nearestNodeId = node.id;
            }
        }
        return nearestNodeId;
    }

    std::vector<Location> path;
    float distance = 0.0;

private:
    int rc = 0;
    Graph graph;
    NodeMap nodes;
    std::vector<int> nodePath;
    std::unordered_map<int, int> distances;
    std::unordered_map<int, int> previous;
};

#endif //PATHFINDER_H
