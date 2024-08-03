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

    using MinHeap = std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>>;

    PathFinder() : db_path{0}, db(nullptr), edge_stmt(nullptr) {
    }

    ~PathFinder() {
        if (edge_stmt) sqlite3_finalize(edge_stmt);
        if (db) sqlite3_close(db);
    }

    int initialize(const std::string& db_path) {
        rc = sqlite3_open(db_path.c_str(), &db);
        if (rc != SQLITE_OK) {
            std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
            return rc;
        }

        const char* edgeQuery = "SELECT target, weight FROM edges WHERE source = ?";
        rc = sqlite3_prepare_v2(db, edgeQuery, -1, &edge_stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to prepare edge query: " << sqlite3_errmsg(db) << std::endl;
            return rc;
        }
        return SQLITE_OK;
    }

    int findPath(Location start, Location end) {
        int startNode = findNearestNode(start.lon, start.lat);
        print("->45:PathFinder.h start.lon, start.lat:", start.lon, start.lat);
        int endNode = findNearestNode(end.lon, end.lat);
        print("->47:PathFinder.h end.lon, end.lat:", end.lon, end.lat);

        if (startNode == -1 || endNode == -1) {
            std::cerr << "Start or end node not found." << std::endl;
            return 0;
        }

        previous.clear();
        distances.clear();
        min_heap = MinHeap(); // Создаем новую пустую очередь

        // Инициализация
        distances[startNode] = 0;
        min_heap.push({0, startNode});

        while (!min_heap.empty()) {
            int u = min_heap.top().second;
            min_heap.pop();

            if (u == endNode) break;

            sqlite3_reset(edge_stmt);
            sqlite3_bind_int(edge_stmt, 1, u);

            while (sqlite3_step(edge_stmt) == SQLITE_ROW) {
                int v = sqlite3_column_int(edge_stmt, 0);
                int weight = sqlite3_column_int(edge_stmt, 1);
                int new_distance = distances[u] + weight;

                if (distances.find(v) == distances.end() || new_distance < distances[v]) {
                    distances[v] = new_distance;
                    previous[v] = u;
                    min_heap.push({new_distance, v});
                }
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
        for (size_t i = 0; i < nodePath.size(); ++i) {
            Location loc = getLocation(nodePath[i]);
            path.push_back(loc);
            if (px != 0) distance += euclideanDistance(px, py, loc.lon, loc.lat);
            px = loc.lon;
            py = loc.lat;
        }

        return path.size();
    }

    static float getDistance(const float x1, const float y1, const float x2, const float y2) {
        return std::sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    }

    float euclideanDistance(float lon1, float lat1, float lon2, float lat2) {
        const float R = 6371.0; // Радиус Земли в километрах
        float dLat = (lat2 - lat1) * M_PI / 180.0;
        float dLon = (lon2 - lon1) * M_PI / 180.0;
        float a = std::sin(dLat / 2) * std::sin(dLat / 2) +
            std::cos(lat1 * M_PI / 180.0) * std::cos(lat2 * M_PI / 180.0) *
            std::sin(dLon / 2) * std::sin(dLon / 2);
        float c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
        float distance = R * c;
        return distance;
    }

    int findNearestNode(double x, double y) {
        int nearestNodeId = -1;
        double minDistance = std::numeric_limits<double>::max();

        sqlite3_stmt* stmtNode;
        const char* nodeQuery = "SELECT id, x, y FROM nodes";
        rc = sqlite3_prepare_v2(db, nodeQuery, -1, &stmtNode, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to prepare node query: " << sqlite3_errmsg(db) << std::endl;
            return nearestNodeId;
        }

        while (sqlite3_step(stmtNode) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmtNode, 0);
            double nodeX = sqlite3_column_double(stmtNode, 1);
            double nodeY = sqlite3_column_double(stmtNode, 2);
            double distance = getDistance(x, y, nodeX, nodeY);
            if (distance < minDistance) {
                minDistance = distance;
                nearestNodeId = id;
            }
        }
        sqlite3_finalize(stmtNode);

        return nearestNodeId;
    }

    std::vector<Location> path;
    float distance = 0;

private:
    std::string db_path;
    sqlite3* db;
    sqlite3_stmt* edge_stmt;
    int rc = 0;

    std::unordered_map<int, int> distances;
    std::unordered_map<int, int> previous;
    MinHeap min_heap;

    Location getLocation(int nodeId) {
        sqlite3_stmt* stmtNode;
        const char* nodeQuery = "SELECT x, y FROM nodes WHERE id = ?";
        rc = sqlite3_prepare_v2(db, nodeQuery, -1, &stmtNode, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to prepare node query: " << sqlite3_errmsg(db) << std::endl;
            return {0.0, 0.0};
        }
        sqlite3_bind_int(stmtNode, 1, nodeId);
        Location loc = {0.0, 0.0};

        if (sqlite3_step(stmtNode) == SQLITE_ROW) {
            loc.lon = sqlite3_column_double(stmtNode, 0);
            loc.lat = sqlite3_column_double(stmtNode, 1);
        }
        sqlite3_finalize(stmtNode);

        return loc;
    }

    int getEdgeWeight(int source, int target) {
        sqlite3_stmt* stmtEdge;
        const char* edgeQuery = "SELECT weight FROM edges WHERE source = ? AND target = ?";
        rc = sqlite3_prepare_v2(db, edgeQuery, -1, &stmtEdge, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to prepare edge query: " << sqlite3_errmsg(db) << std::endl;
            return std::numeric_limits<int>::max();
        }
        sqlite3_bind_int(stmtEdge, 1, source);
        sqlite3_bind_int(stmtEdge, 2, target);
        int weight = std::numeric_limits<int>::max();

        if (sqlite3_step(stmtEdge) == SQLITE_ROW) {
            weight = sqlite3_column_int(stmtEdge, 0);
        }
        sqlite3_finalize(stmtEdge);

        return weight;
    }
};

#endif //PATHFINDER_H
