#include "helper.hpp"

vector<int> read_demands(ifstream &file) {
    int num_vertices;
    file >> num_vertices;

    vector<int> demands(num_vertices, 0);
    for (int i = 1; i < num_vertices; i++) {
        int stop;
        int demand;
        file >> stop >> demand;
        demands[stop] = demand;
    }

    return demands;
}

vector<vector<int>> read_routes(ifstream &file, int num_stops) {
    int num_routes;
    file >> num_routes;
    vector<vector<int>> routes(num_stops, vector<int>(num_stops, 0));

    for (int i = 0; i < num_routes; i++) {
        int u, v, cost;
        file >> u >> v >> cost;
        routes[u][v] = cost;
        routes[v][u] = cost;
    }
    return routes;
}

vector<vector<int>> generate_permutations(int num_points) {
    vector<vector<int>> permutations;
    vector<int> route(num_points - 1);

    for (int i = 1; i < num_points; i++) {
        route[i - 1] = i;
    }

    do {
        vector<int> full_route;
        full_route.push_back(0);
        full_route.insert(full_route.end(), route.begin(), route.end());
        full_route.push_back(0);
        permutations.push_back(full_route);
    } while (std::next_permutation(route.begin(), route.end()));

    return permutations;
}

Route get_cheapest(vector<Route> valid_routes) {
    int lowest_cost = numeric_limits<int>::max();
    Route cheapest_route;

    for (auto &route : valid_routes) {
        if (route.total_cost < lowest_cost) {
            lowest_cost = route.total_cost;
            cheapest_route = route;
        }
    }
    return cheapest_route;
}

void debug_route(vector<int> route) {
    for (auto stop : route) {
        cout << stop << "\n";
    }
}

void write_routes(const vector<Route> &routes, const string &filename) {
    ofstream outFile(filename);

    if (!outFile.is_open()) {
        cout << "Error opening output file." << endl;
        return;
    }

    for (const auto &route : routes) {
        for (auto stop : route.stops) {
            outFile << stop << " ";
        }
        outFile << " | " << route.total_cost << endl;
    }

    outFile.close();
}
