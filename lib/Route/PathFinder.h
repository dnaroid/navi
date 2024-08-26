#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <iostream>
#include <vector>
#include <limits>
#include <unordered_map>
#include <queue>
#include <sqlite3.h>
#include "globals.h"


// Memory-Bounded A*
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

    using Graph = std::vector<std::vector<Edge>>;

    int MAX_NODES;
    Node* nodes;
    int* previous;
    int* distances;
    Graph graph;
    int memoryLimit = 10000;
    std::vector<Location> path = {};
    float distance = 0.0;
    sqlite3_stmt* stmt;

    static float getDistance(const float x1, const float y1, const float x2, const float y2) {
        return std::sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    }

    static float getRealDistance(const float lon1, const float lat1, const float lon2, const float lat2) {
        constexpr float R = 6371.0;
        const float dLat = (lat2 - lat1) * M_PI / 180.0;
        const float dLon = (lon2 - lon1) * M_PI / 180.0;
        const float a = std::sin(dLat / 2) * std::sin(dLat / 2) +
            std::cos(lat1 * M_PI / 180.0) * std::cos(lat2 * M_PI / 180.0) *
            std::sin(dLon / 2) * std::sin(dLon / 2);
        const float c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
        return R * c;
    }

    int init() {
        LOG("56:PathFinder.h init:");
        sqlite3_initialize();
        sqlite3* db;
        sqlite3_stmt* stmt;

        rc = sqlite3_open("/sd/graph_all.db", &db);
        if (rc != SQLITE_OK) {
            LOG("Cannot open database: ", sqlite3_errmsg(db));
            return rc;
        }
        LOG("Opened database successfully");

        const char* countQuery = "SELECT COUNT(*) FROM nodes";
        rc = sqlite3_prepare_v2(db, countQuery, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            LOG("Failed to fetch node count: ", sqlite3_errmsg(db));
            sqlite3_close(db);
            return rc;
        }

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            MAX_NODES = sqlite3_column_int(stmt, 0);
            LOG("81:PathFinder.h MAX_NODES:", MAX_NODES);
        } else {
            LOG("Failed to fetch node count.");
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return rc;
        }
        sqlite3_finalize(stmt);

        nodes = new Node[MAX_NODES];
        LOG("93:PathFinder.h nodes");
        previous = new int[MAX_NODES];
        LOG("95:PathFinder.h previous");
        distances = new int[MAX_NODES];
        LOG("97:PathFinder.h distances");
        graph.resize(MAX_NODES);
        LOG("99:PathFinder.h graph");

        const auto nodeQuery = "SELECT id, x, y FROM nodes";
        rc = sqlite3_prepare_v2(db, nodeQuery, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            LOG("Failed to fetch nodes: ", sqlite3_errmsg(db));
            sqlite3_close(db);
            return rc;
        }

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            float x = static_cast<float>(sqlite3_column_double(stmt, 1));
            float y = static_cast<float>(sqlite3_column_double(stmt, 2));
            nodes[id] = {id, x, y};
        }
        sqlite3_finalize(stmt);
        LOG("113:PathFinder.h nodes loaded");

        const auto edgeQuery = "SELECT source, target, weight FROM edges";
        rc = sqlite3_prepare_v2(db, edgeQuery, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            LOG("Failed to fetch edges: ", sqlite3_errmsg(db));
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
        LOG("130:PathFinder.h graph loaded");

        LOG("Init ok");
        return SQLITE_OK;
    }

    int findPath(const Location start, const Location end) {
        size_t freeMemory = heap_caps_get_free_size(MALLOC_CAP_8BIT);
        LOG("62:PathFinder.h free_memory:", freeMemory);
        memoryLimit = freeMemory - 100 * 1024;
        LOG("138:PathFinder.h memoryLimit:", memoryLimit);

        const int startNode = findNearestNode(start);
        const int endNode = findNearestNode(end);

        if (startNode == -1 || endNode == -1) {
            std::cerr << "Start or end node not found." << std::endl;
            return 0;
        }

        std::fill(previous, previous + MAX_NODES, -1);
        std::fill(distances, distances + MAX_NODES, std::numeric_limits<int>::max());

        using MinHeap = std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>>;
        MinHeap min_heap;

        distances[startNode] = 0;
        min_heap.push({0, startNode});

        while (!min_heap.empty()) {
            int u = min_heap.top().second;
            min_heap.pop();

            if (u == endNode) break;

            for (const Edge& edge : graph[u]) {
                int v = edge.target;
                const int weight = edge.weight;
                int new_distance = distances[u] + weight;

                if (new_distance < distances[v]) {
                    distances[v] = new_distance;
                    previous[v] = u;
                    min_heap.push({new_distance, v});
                }
            }

            // Логика для ограничения памяти
            if (min_heap.size() > memoryLimit) {
                min_heap.pop(); // Удаляем наименее перспективный узел
            }
        }

        std::vector<int> nodePath;
        for (int at = endNode; at != -1; at = previous[at]) {
            nodePath.push_back(at);
        }
        std::reverse(nodePath.begin(), nodePath.end());

        distance = 0.0;
        float px = 0;
        float py = 0;
        path.clear();
        for (int nodeId : nodePath) {
            const Node& node = nodes[nodeId];
            path.push_back({node.x, node.y});
            if (px != 0) distance += getRealDistance(px, py, node.x, node.y);
            px = node.x;
            py = node.y;
        }
        LOG("Found path, distance:", distance, "km");
        return path.size();
    }

private:
    sqlite3* db = nullptr;
    int rc = 0;
    std::vector<int> nodePath;


    int findNearestNode(const Location loc) const {
        int nearestNodeId = -1;
        float minDistance = std::numeric_limits<float>::max();

        for (int i = 0; i < MAX_NODES; ++i) {
            const Node& node = nodes[i];
            const float distance = getDistance(loc.lon, loc.lat, node.x, node.y);
            if (distance < minDistance) {
                minDistance = distance;
                nearestNodeId = node.id;
            }
        }
        return nearestNodeId;
    }
};

#endif //PATHFINDER_H
