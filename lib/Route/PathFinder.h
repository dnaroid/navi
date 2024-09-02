#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <BootManager.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <sqlite3.h>
#include "globals.h"
#include "Helpers.h"


// Memory-Bounded A*
class PathFinder {
public:
    std::vector<Location> path = {};
    float distance = 0.0;
    int zoom = ZOOM_MIN;
    Location pathCenter;

    int init(Transport transport) {
        LOG("PathFinder.h init:");
        sqlite3_initialize();
        sqlite3* db;
        sqlite3_stmt* stmt;

        std::string db_path;

        switch (transport) {
        case TransportAll:
            db_path = "/sd/graph_all.db";
            break;
        case TransportWalk:
            db_path = "/sd/graph_walk.db";
            break;
        case TransportBike:
            db_path = "/sd/graph_bike.db";
            break;
        case TransportCar:
            db_path = "/sd/graph_drive.db";
            break;
        default:
            throw std::runtime_error("Unknown transport type");
        }

        rc = sqlite3_open(db_path.c_str(), &db);
        if (rc != SQLITE_OK) {
            LOG("Cannot open database: ", sqlite3_errmsg(db));
            return rc;
        }

        const char* countQuery = "SELECT COUNT(*) FROM nodes";
        rc = sqlite3_prepare_v2(db, countQuery, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            LOG("Failed to fetch node count: ", sqlite3_errmsg(db));
            sqlite3_close(db);
            return rc;
        }

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            nodesCount = sqlite3_column_int(stmt, 0);
            LOG("PathFinder.h MAX_NODES:", nodesCount);
        } else {
            LOG("Failed to fetch node count.");
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return rc;
        }
        sqlite3_finalize(stmt);

        nodes = new Node[nodesCount];
        previous = new int[nodesCount];
        distances = new int[nodesCount];
        graph.resize(nodesCount);

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
        LOG("PathFinder.h nodes loaded");

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
        LOG("PathFinder.h graph loaded");

        LOG("Init ok");
        return SQLITE_OK;
    }

    int findPath(const Location start, const Location end) {
        size_t freeMemory = heap_caps_get_free_size(MALLOC_CAP_8BIT);
        pathCenter = end;
        LOG("PathFinder.h free_memory:", freeMemory);
        memoryLimit = freeMemory - 200;
        LOG("PathFinder.h memoryLimit:", memoryLimit);

        const int startNode = findNearestNode(start);
        const int endNode = findNearestNode(end);

        if (startNode == -1 || endNode == -1) {
            std::cerr << "Start or end node not found." << std::endl;
            return 0;
        }

        std::fill(previous, previous + nodesCount, -1);
        std::fill(distances, distances + nodesCount, std::numeric_limits<int>::max());

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

            if (min_heap.size() > memoryLimit) {
                min_heap.pop();
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

    void calculateMapCenterAndZoom() {
        if (path.empty()) {
            zoom = ZOOM_MAX;
            return;
        };

        auto cz = getBBoxCenterAndZoom(getBBox(path));

        pathCenter = cz.center;
        zoom = cz.zoom;
    }

    ~PathFinder() {
        clear();
    }

private:
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

    int nodesCount;
    Node* nodes;
    int* previous;
    int* distances;
    Graph graph;
    int memoryLimit = 10000;
    sqlite3_stmt* stmt;
    sqlite3* db = nullptr;
    int rc = 0;
    std::vector<int> nodePath;

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

    int findNearestNode(const Location loc) const {
        int nearestNodeId = -1;
        float minDistance = std::numeric_limits<float>::max();

        for (int i = 0; i < nodesCount; ++i) {
            const Node& node = nodes[i];
            const float distance = getDistance(loc.lon, loc.lat, node.x, node.y);
            if (distance < minDistance) {
                minDistance = distance;
                nearestNodeId = node.id;
            }
        }
        return nearestNodeId;
    }

    void clear() {
        delete[] nodes;
        delete[] previous;
        delete[] distances;

        nodes = nullptr;
        previous = nullptr;
        distances = nullptr;

        graph.clear();
        nodePath.clear();
    }
};

#endif //PATHFINDER_H
