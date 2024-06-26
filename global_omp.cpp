#include <omp.h>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <vector>
#include "helper.hpp"
using namespace std;



vector<Route> filter_routes(vector<vector<int>> permutations, vector<vector<int>> real_routes, vector<int> demands, int max_weight, int max_stops) {
    vector<Route> valid_routes;
    #pragma omp parallel for default(none) shared(valid_routes, permutations, real_routes, demands, max_weight, max_stops)
    for (int r = 0; r < int(permutations.size()); r++) {
        vector<int> route = permutations[r];
        int cost = 0;
        int weight = 0;
        int size = route.size() - 1;
        bool valid = true;
        for (int i = 0; i < size; i++) {
            int current_stop = route[i];
            int next_stop = route[i + 1];

            int current_cost = real_routes[current_stop][next_stop]; // cost of going to next stop
            int next_weight = demands[next_stop]; // Access demands directly from the vector using the stop number as index
            bool over_weight = weight + next_weight > max_weight;

            if (current_cost == 0 || over_weight) { // if any constraint is violated we try to add 0 in between the edges of the violator to try and fix, ADDING COST CALCULATION
                int return_cost = real_routes[current_stop][0]; // cost of returning to the origin

                if ((real_routes[current_stop][0] == 0) || (real_routes[0][next_stop] == 0)) { // if there isn't a route from current_stop to 0 or from 0 to next stop the route cannot be fixed.
                    valid = false;
                    break; // exiting the loop as the current route cannot be fixed
                }
                
                route.insert(route.begin() + i + 1, 0);
                weight = 0; // resetting the weight as the vehicle returned to the origin
                next_weight = 0;
                current_cost = return_cost;
                size++;
            }
            weight += next_weight;
            cost += current_cost;
        }

        if (valid) {
            Route new_route;
            new_route.stops = route;
            new_route.total_cost = cost;
            #pragma omp critical
            valid_routes.push_back(new_route);
        }
    }
    return valid_routes;
}



int main(int argc, char *argv[]) {
    // reading data
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }
    string file_name = argv[1];
    ifstream file(file_name);
    
    vector<int> demands = read_demands(file);  // Store the stops and their demands in the demands vector
    vector<vector<int>> route_matrix = read_routes(file, demands.size()); // No need to add 1 now, as the vector is already appropriately sized
    vector<vector<int>> permutations = generate_permutations(route_matrix.size());
    int max_weight = 15; // Hardcoding max weight to test
    int max_stops = 7; // Hardcoding max stops to test
    vector<Route> valid_routes = filter_routes(permutations, route_matrix, demands, max_weight, max_stops);
    Route cheapest_route = get_cheapest(valid_routes);
    vector<Route> results;
    results.push_back(cheapest_route);
    write_routes(results, "output/global_omp.txt"); // Writing function to debug

    file.close();
    return 0;
}
