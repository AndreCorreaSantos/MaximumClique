#include <fstream>
#include <iostream>
#include <limits>
#include <vector>
#include <map>
#include <cuda_runtime.h>

using namespace std;

class Route {
public:
    __host__ __device__ Route() : size(0), stops(nullptr), totalCost(0), valid(false) {}

    __host__ __device__ Route(int size, int* stops) : size(size), stops(stops), totalCost(0), valid(false) {}

    __host__ Route(int max_stops) : size(0), totalCost(0), valid(false) {
        cudaMallocManaged(&stops, max_stops * sizeof(int));
        cudaMemset(stops, 0, max_stops * sizeof(int));
    }

    __host__ __device__ ~Route() {
        if (stops != nullptr) {
            cudaFree(stops);
        }
    }

    int size;
    int* stops;
    int totalCost;
    bool valid;
};


void read_demands(ifstream& file, int* demands, int num_vertices) {
    for (int i = 1; i < num_vertices; i++) {
        int stop, demand;
        file >> stop >> demand;
        demands[stop] = demand;
        cout << "stop, demand: " << stop << ", " << demand << "\n";
    }
}

void read_routes(ifstream& file, int num_vertices, int* route_matrix) {
    int num_routes;
    file >> num_routes;

    for (int i = 0; i < num_vertices; i++) {
        for (int j = 0; j < num_vertices; j++) {
            route_matrix[i * num_vertices + j] = 0; // initializing all costs as 0
        }
    }

    for (int i = 0; i < num_routes; i++) {
        int u, v, cost;
        file >> u >> v >> cost;
        route_matrix[u * num_vertices + v] = cost;
        route_matrix[v * num_vertices + u] = cost;
    }
}

void write_routes(const vector<Route>& routes, const string& filename) {
    ofstream outFile(filename);

    if (!outFile.is_open()) {
        cout << "Error opening output file." << endl;
        return;
    }

    for (const Route& route : routes) {
        for (int j = 0; j < route.size; j++) {
            outFile << route.stops[j] << " ";
        }
        outFile << " | " << route.totalCost;
        outFile << endl;
    }

    outFile.close();
}

void permute(vector<int>& route, int start, vector<vector<int>>& permutations) {
    if (start == route.size()) {
        vector<int> full_route;
        full_route.push_back(0);
        full_route.insert(full_route.end(), route.begin(), route.end());
        full_route.push_back(0);
        permutations.push_back(full_route);
    } else {
        for (int i = start; i < route.size(); i++) {
            swap(route[start], route[i]);
            permute(route, start + 1, permutations);
            swap(route[start], route[i]); // backtrack
        }
    }
}

vector<vector<int>> generate_permutations(int num_points) {
    vector<vector<int>> permutations;
    vector<int> route(num_points - 1); // Only permute num_points-1 elements

    for (int i = 1; i < num_points; i++) {
        route[i - 1] = i;
    }

    permute(route, 0, permutations);
    return permutations;
}

__device__ void insert_into_route(Route& route, int capacity, int index, int value) {
    if (route.size >= capacity) {
        return;
    }

    // Shift elements to the right to make space for the new element
    for (int i = route.size; i > index; --i) {
        route.stops[i] = route.stops[i - 1];
    }

    route.stops[index] = value;

    route.size++;
}

__global__ void filter_routes_kernel(Route* d_routes, int* route_matrix, int* demands, Route* valid_routes, int max_weight, int num_permutations, int num_vertices, int max_route_size) {
    int tId = threadIdx.x + blockIdx.x * blockDim.x;
    if (tId > num_permutations-1) return;

    Route& route = d_routes[tId];
    int cost = 0;
    int weight = 0;
    bool valid = true;

    for (int i = 0; i < route.size - 1; i++) {
        int current_stop = route.stops[i];
        int next_stop = route.stops[i + 1];

        int current_cost = route_matrix[current_stop * num_vertices + next_stop];
        int next_weight = demands[next_stop];
        bool over_weight = weight + next_weight > max_weight;

        if (current_cost == 0 || over_weight) {
            // Check return path to the depot
            int return_cost = route_matrix[current_stop * num_vertices];
            int to_next_cost = route_matrix[next_stop];

            if (return_cost == 0 || to_next_cost == 0) {
                valid = false;
                break;
            }

            // Insert zero after the current stop if not back-to-back zeros
            insert_into_route(route, max_route_size, i + 1, 0);

            weight = 0; // resetting the weight as the vehicle returned to the origin
            next_weight = 0;
            current_cost = return_cost;
        }

        weight += next_weight;
        cost += current_cost;
    }

    if (valid) {
        route.totalCost = cost;
        route.valid = true;
        valid_routes[tId] = route;  // Store the valid route
    }
}

Route get_cheapest(const vector<Route>& valid_routes) {
    int lowest_cost = numeric_limits<int>::max();
    Route cheapest_route;

    for (const auto& route : valid_routes) {
        if (route.valid && route.totalCost < lowest_cost) {
            lowest_cost = route.totalCost;
            cheapest_route = route;
        }
    }

    return cheapest_route;
}

void debug_route(const vector<int>& route) {
    for (int stop : route) {
        cout << stop << " ";
    }
    cout << endl;
}

void debug_route_matrix(int* route_matrix, int num_vertices) {
    cout << "Route Matrix:" << endl;
    for (int i = 0; i < num_vertices; i++) {
        for (int j = 0; j < num_vertices; j++) {
            cout << route_matrix[i * num_vertices + j] << " ";
        }
        cout << endl;
    }
}

void debug_demands(int* demands, int num_vertices) {
    cout << "Demands:" << endl;
    for (int i = 0; i < num_vertices; i++) {
        cout << "Stop " << i << ": " << demands[i] << endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    string file_name = argv[1];
    ifstream file(file_name);

    int num_vertices;
    file >> num_vertices;
    int* demands;
    cudaMallocManaged(&demands, num_vertices * sizeof(int));
    read_demands(file, demands, num_vertices);

    debug_demands(demands, num_vertices); // Debugging demands

    int* route_matrix;
    cudaMallocManaged(&route_matrix, num_vertices * num_vertices * sizeof(int));
    read_routes(file, num_vertices, route_matrix);

    debug_route_matrix(route_matrix, num_vertices); // Debugging route matrix

    vector<vector<int>> permutations = generate_permutations(num_vertices);

    int num_routes = permutations.size();
    int max_route_size = 15; // setting max_route_size to 15 to test

    Route* host_routes;
    cudaMallocManaged(&host_routes, num_routes * sizeof(Route));

    for (int i = 0; i < num_routes; i++) {
        host_routes[i] = Route(max_route_size);
        for (size_t j = 0; j < permutations[i].size(); j++) {
            host_routes[i].stops[j] = permutations[i][j];
        }
        host_routes[i].size = permutations[i].size();
    }

    Route* d_valid_routes;
    cudaMallocManaged(&d_valid_routes, num_routes * sizeof(Route));
    
    int count = 0;
    for (int i = 0; i<num_routes; i++){
        for (int j = 0; j<max_route_size-1; j++){
            count += d_valid_routes[i].stops[j];
        }
    }
    cout << count;
    // int blockSize = 256;
    // int numBlocks = (num_routes + blockSize - 1) / blockSize;

    // int max_weight = 15;
    // filter_routes_kernel<<<numBlocks, blockSize>>>(host_routes, route_matrix, demands, d_valid_routes, max_weight, num_routes, num_vertices, max_route_size);
    // cudaDeviceSynchronize();

    // vector<Route> valid_routes(num_routes);
    // cudaMemcpy(valid_routes.data(), d_valid_routes, num_routes * sizeof(Route), cudaMemcpyDeviceToHost);

    // // Write all valid routes to file
    // vector<Route> results;
    // for (const auto& route : valid_routes) {
    //     if (route.valid) {
    //         results.push_back(route);
    //     }
    // }
    // write_routes(results, "debug_cuda.txt");

    // file.close();
    // cudaFree(demands);
    // cudaFree(route_matrix);
    // cudaFree(host_routes);
    // cudaFree(d_valid_routes);

    // return 0;
}
