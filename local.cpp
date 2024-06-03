#include <fstream>
#include <iostream>
#include <limits>
#include <vector>
#include <algorithm>
#include "helper.hpp"
#include <climits>

using namespace std;

vector<int> get_stops(int num_vertices) {
    vector<int> stops;
    for (int i = 1; i < num_vertices; i++) { // Start from 1 because 0 is depot
        stops.push_back(i);
    }
    return stops;
}

// function that receives the permutations and tries to fix routes that violate constraints
Route generate_route(vector<vector<int>> route_matrix, vector<int> demands, int max_weight, int num_vertices) {
    vector<int> route_stops;
    route_stops.push_back(0); // Starting at depot (0)
    int current_stop = 0;
    vector<int> remaining_stops = get_stops(num_vertices);
    int size = remaining_stops.size();

    while (!remaining_stops.empty()) {
        int lowest_cost = INT_MAX;
        int next_stop = -1;
        int index = -1;

        for (int i = 0; i < size; i++) {
            int candidate = remaining_stops[i];
            int path_cost = route_matrix[current_stop][candidate];
            bool path_exists = path_cost != 0;
            bool not_inplace = current_stop != candidate;
            bool cheaper = path_cost < lowest_cost;

            if (path_exists && not_inplace && cheaper) {
                lowest_cost = path_cost;
                next_stop = candidate;
                index = i;
            }
        }

        if (next_stop != -1) {
            remaining_stops.erase(remaining_stops.begin() + index);
            size--;
            route_stops.push_back(next_stop);
            current_stop = next_stop;
        } else {
            // If no valid next stop is found, break the loop to avoid infinite loop
            break;
        }
    }

    route_stops.push_back(0); // returning to depot

    // Fixing overweight violations and calculating final cost
    int weight = 0;
    int cost = 0;
    for (size_t i = 0; i < route_stops.size() - 1; i++) {
        int current_stop = route_stops[i];
        int next_stop = route_stops[i + 1];

        bool overweight = !(weight + demands[next_stop] <= max_weight);
        if (overweight) { // returning to depot and resetting weight
            route_stops.insert(route_stops.begin() + i + 1, 0);
            weight = 0;
            next_stop = 0;
        } else {
            weight += demands[next_stop];
        }
        cost += route_matrix[current_stop][next_stop];
    }

    Route local_route;
    local_route.stops = route_stops;
    local_route.total_cost = cost;
    return local_route;
}

int main(int argc, char *argv[]) {
    // reading data
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }
    string file_name = argv[1];
    ifstream file(file_name);

    vector<int> demands = read_demands(file); 
    vector<vector<int>> route_matrix = read_routes(file, demands.size());
    int max_weight = 15; 
    int num_vertices = demands.size(); // Ensure num_vertices reflects the actual number of vertices

    Route local_route = generate_route(route_matrix, demands, max_weight, num_vertices);
    vector<Route> results;
    results.push_back(local_route);
    write_routes(results, "output/local.txt"); 

    file.close();

    return 0;
}
