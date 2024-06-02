#include <mpi.h>
#include <omp.h>
#include <fstream>
#include <iostream>
#include <vector>
#include "helper.hpp"  // Assuming this contains necessary custom type and function declarations

using namespace std;



vector<int> serialize_routes(const vector<Route>& routes) {
    vector<int> serialized;
    for (const auto& route : routes) {
        serialized.push_back(route.stops.size());  // Number of stops
        for (int stop : route.stops) {
            serialized.push_back(stop);
        }
        serialized.push_back(route.total_cost);
    }
    return serialized;
}

vector<Route> deserialize_routes(const vector<int>& serialized) {
    vector<Route> routes;
    size_t i = 0;
    while (i < serialized.size()) {
        int num_stops = serialized[i++];
        vector<int> stops(num_stops);
        for (int j = 0; j < num_stops; j++) {
            stops[j] = serialized[i++];
        }
        int total_cost = serialized[i++];
        routes.push_back({stops, total_cost});
    }
    return routes;
}

vector<Route> filter_routes(vector<vector<int>>& permutations, vector<vector<int>>& real_routes, vector<int>& demands, int max_weight, int max_stops) {
    vector<Route> valid_routes;
    #pragma omp parallel for default(none) shared(valid_routes, permutations, real_routes, demands, max_weight, max_stops)
    for (int r = 0; r < int(permutations.size()); r++) {
        vector<int> route = permutations[r];
        int cost = 0;
        int weight = 0;
        bool valid = true;
        for (size_t i = 0; i < route.size() - 1; i++) {
            int current_stop = route[i];
            int next_stop = route[i + 1];
            int current_cost = real_routes[current_stop][next_stop];
            int next_weight = demands[next_stop];
            if (current_cost == 0 || weight + next_weight > max_weight) {
                valid = false;
                break;
            }
            weight += next_weight;
            cost += current_cost;
        }
        if (valid) {
            #pragma omp critical
            valid_routes.push_back({route, cost});
        }
    }
    return valid_routes;
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        if (rank == 0) {
            cerr << "This program needs at least two MPI processes.\n";
        }
        MPI_Finalize();
        return 1;
    }

    vector<int> demands;
    vector<vector<int>> route_matrix;
    vector<vector<int>> permutations;
    int max_weight = 15;
    int max_stops = 7;

    if (rank == 0) {
        ifstream file(argv[1]);
        demands = read_demands(file);
        route_matrix = read_routes(file, demands.size());
        permutations = generate_permutations(route_matrix.size());
    }

    int num_permutations_per_process = permutations.size() / size;
    vector<vector<int>> local_permutations(num_permutations_per_process);
    MPI_Scatter(permutations.data(), num_permutations_per_process * permutations[0].size(), MPI_INT,
                local_permutations.data(), num_permutations_per_process * permutations[0].size(), MPI_INT, 0, MPI_COMM_WORLD);

    vector<Route> local_valid_routes = filter_routes(local_permutations, route_matrix, demands, max_weight, max_stops);
    vector<int> local_serialized = serialize_routes(local_valid_routes);
    int local_size = local_serialized.size();
    vector<int> sizes;

    if (rank == 0) {
        sizes.resize(size);
    }

    MPI_Gather(&local_size, 1, MPI_INT, sizes.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    vector<int> all_serialized_data;
    vector<int> displacements;

    if (rank == 0) {
        displacements.resize(size);
        int total_size = 0;
        for (int i = 0; i < size; ++i) {
            displacements[i] = total_size;
            total_size += sizes[i];
        }
        all_serialized_data.resize(total_size);
    }

    MPI_Gatherv(local_serialized.data(), local_size, MPI_INT,
                all_serialized_data.data(), sizes.data(), displacements.data(), MPI_INT,
                0, MPI_COMM_WORLD);

    if (rank == 0) {
        vector<Route> all_valid_routes = deserialize_routes(all_serialized_data);
        Route cheapest_route = get_cheapest(all_valid_routes);
        cout << "Cheapest route cost: " << cheapest_route.total_cost << endl;
    }

    MPI_Finalize();
    return 0;
}
