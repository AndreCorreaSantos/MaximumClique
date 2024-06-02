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

vector<Route> filter_routes(vector<vector<int>>& permutations, vector<vector<int>>& real_routes, vector<int>& demands, int max_weight, int max_stops) {
    vector<Route> valid_routes;
    #pragma omp parallel for
    for (int i = 0; i < permutations.size(); i++) {
        vector<int> route = permutations[i];
        int cost = 0;
        int weight = 0;
        bool valid = true;
        for (size_t j = 0; j < route.size() - 1; j++) {
            int current_stop = route[j];
            int next_stop = route[j + 1];
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

    // Debug print to verify broadcasting
    if (rank == 1) {
        cout << "Route matrix after broadcasting:" << endl;
        for (const auto& row : route_matrix) {
            for (const auto& element : row) {
                cout << element << " ";
            }
            cout << endl;
        }
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

    // Debug print to verify flat_permutations on rank 0 before scattering
    if (rank == 0) {
        cout << "Flat permutations before scattering:" << endl;
        for (const auto& perm : flat_permutations) {
            cout << perm << " ";
        }
        cout << endl;
    }

    // Allocate space for local permutations and scatter
    vector<int> local_flat_permutations(elements_per_process, -1); // Initialize with -1 to check if values are overwritten

    MPI_Scatter(flat_permutations.data(), elements_per_process, MPI_INT, 
                local_flat_permutations.data(), elements_per_process, MPI_INT, 0, MPI_COMM_WORLD);

    // Debug print to verify local_flat_permutations on each rank after scattering
    cout << "Local flat permutations on rank " << rank << ":" << endl;
    cout << elements_per_process << endl;
    for (const auto& perm : local_flat_permutations) {
        cout << perm << " ";
    }
    cout << endl;

    // Reshape local_flat_permutations to local_permutations
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

    MPI_Finalize();
    return 0;
}
