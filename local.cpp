#include <fstream>
#include <iostream>
#include <limits>
#include <vector>
#include <algorithm>
#include "helper.hpp"
#include <climits>

using namespace std;

vector<int> get_stops(int num_vertices){
    vector<int> stops;
    for (int i = 0; i<num_vertices; i++){
        stops.push_back(i);
    }
    return stops;
}

// function that receives the permutations and tries to fix routes that violate constraints
Route generate_route(vector<vector<int>> route_matrix, vector<int> demands, int max_weight, int num_vertices) {
    int cost = 0;
    int weight = 0;
    vector<int> route_stops;
    route_stops.push_back(0);
    // starting at 0
    int current_stop = 0;
    int size = num_vertices;
    int route_cost = 0;
    vector<int> remaining_stops = get_stops(num_vertices);
    while(size != 1){  // stop when only 0 is left
        int lowest_cost = INT_MAX;
        for (int i = 0; i<size; i++){
            
            int next_stop = remaining_stops[i];
            int path_cost = route_matrix[current_stop][next_stop];
            bool path_exists = path_cost != 0;
            bool not_inplace = current_stop != i;
            bool cheaper = path_cost < lowest_cost;

            if(path_exists && not_inplace && cheaper){
                lowest_cost = path_cost;

                if(next_stop != 0){
                    remaining_stops.erase(remaining_stops.begin()+i); // delete the current element if its not the origin
                    size--;
                }
                route_stops.push_back(next_stop);
                route_cost += path_cost;
                break; // exit the for loop and search for the next_stop
            }
        }
    }
    Route local_route;
    local_route.stops = route_stops;
    local_route.total_cost = route_cost;
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
    int max_stops = 7;
    Route local_route = generate_route(route_matrix, demands, max_weight, max_stops);
    vector<Route> results;
    results.push_back(local_route);
    write_routes(results, "output/local_debug.txt"); 

    file.close();

    return 0;
}
