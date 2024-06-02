#include <mpi.h>
#include <omp.h>
#include <fstream>
#include <iostream>
#include <vector>
#include "helper.hpp"  // Assuming this contains necessary definitions

using namespace std;

vector<int> serialize_routes(const vector<Route>& routes) {
    vector<int> serialized;
    for (const auto& route : routes) {
        serialized.push_back(route.stops.size());
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
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        cerr << "This program needs at least two MPI processes.\n";
        MPI_Finalize();
        return 1;
    }

    vector<int> demands;
    vector<vector<int>> route_matrix;
    vector<vector<int>> permutations;
    int max_weight = 15;
    int max_stops = 7;
    int permutations_size = 0;

    if (rank == 0) {
        ifstream file(argv[1]);
        if (!file) {
            cerr << "Failed to open file: " << argv[1] << endl;
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        demands = read_demands(file);
        route_matrix = read_routes(file, demands.size());
        permutations = generate_permutations(demands.size());
        permutations_size = permutations.size();
    }

    // Broadcast the size of permutations
    MPI_Bcast(&permutations_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Broadcast the size of demands
    int demands_size = demands.size();
    MPI_Bcast(&demands_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Resize vectors in all ranks based on the demands_size
    if (rank != 0) {
        demands.resize(demands_size);
        route_matrix.resize(demands_size, vector<int>(demands_size));
    }

    // Broadcast demands
    MPI_Bcast(demands.data(), demands_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Broadcast route matrix row by row
    for (int i = 0; i < demands_size; ++i) {
        MPI_Bcast(route_matrix[i].data(), demands_size, MPI_INT, 0, MPI_COMM_WORLD);
    }

    // Calculate the total number of elements each process will receive
    int perm_length = demands_size + 1;
    int num_permutations_per_process = (permutations_size + size - 1) / size;  // Distribute permutations evenly
    int elements_per_process = num_permutations_per_process * perm_length;

    // Flatten permutations for scattering
    vector<int> flat_permutations(elements_per_process * size, -1);  // Initialize with -1 to handle cases where there are fewer permutations than needed
    if (rank == 0) {
        for (int i = 0; i < permutations.size(); ++i) {
            copy(permutations[i].begin(), permutations[i].end(), flat_permutations.begin() + i * perm_length);
        }
    }

    vector<int> local_flat_permutations(elements_per_process, -1); // Initialize with -1 to check if values are overwritten

    MPI_Scatter(flat_permutations.data(), elements_per_process, MPI_INT, 
                local_flat_permutations.data(), elements_per_process, MPI_INT, 0, MPI_COMM_WORLD);


    vector<vector<int>> local_permutations(num_permutations_per_process, vector<int>(perm_length));
    for (int i = 0; i < num_permutations_per_process; ++i) {
        local_permutations[i].assign(local_flat_permutations.begin() + i * perm_length,
                                     local_flat_permutations.begin() + (i + 1) * perm_length);
    }

    // Debug print to verify scattering
    cout << "Local permutations after scattering on rank " << rank << ":" << endl;
    for (const auto& perm : local_permutations) {
        for (const auto& stop : perm) {
            cout << stop << " ";
        }
        cout << endl;
    }

    // Run filter_routes on each node
    vector<Route> local_valid_routes = filter_routes(local_permutations, route_matrix, demands, max_weight, max_stops);

    // Debug print to verify local_valid_routes
    cout << "Rank " << rank << " has " << local_valid_routes.size() << " valid routes." << endl;

    // Serialize the filtered routes
    vector<int> local_serialized = serialize_routes(local_valid_routes);
    int local_size = local_serialized.size();

    // Gather the sizes of serialized routes from each process
    vector<int> sizes(size);
    MPI_Gather(&local_size, 1, MPI_INT, sizes.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Calculate displacements for gathering the serialized routes
    vector<int> displacements(size);
    int total_size = 0;
    if (rank == 0) {
        for (int i = 0; i < size; ++i) {
            displacements[i] = total_size;
            total_size += sizes[i];
        }
    }

    // Allocate space for all serialized routes in rank 0
    vector<int> all_serialized_data(total_size);

    // Gather all serialized routes at rank 0
    MPI_Gatherv(local_serialized.data(), local_size, MPI_INT, all_serialized_data.data(),
                sizes.data(), displacements.data(), MPI_INT, 0, MPI_COMM_WORLD);

    // Rank 0 deserializes the routes and finds the cheapest route
    if (rank == 0) {
        vector<Route> all_valid_routes = deserialize_routes(all_serialized_data);
        if (!all_valid_routes.empty()) {
            Route cheapest_route = get_cheapest(all_valid_routes);
            cout << "Cheapest route cost: " << cheapest_route.total_cost << endl;
        } else {
            cout << "No valid routes found." << endl;
        }
    }

    MPI_Finalize();
    return 0;
}
