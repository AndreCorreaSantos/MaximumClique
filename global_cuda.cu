#include <fstream>
#include <iostream>
#include <limits>
#include <vector>
#include <cuda_runtime.h>

using namespace std;


class Route{
    public:
        __host__ __device__ Route() {}
        __host__ __device__ int size;
        __host__ __device__ int *stops;
        __host__ __device__ int totalCost;
        __host__ __device__ bool valid;
}



void read_demands(ifstream &file, int *demands, int num_vertices) {
    for (int i = 1; i < num_vertices; i++) {
        int stop, demand;
        file >> stop >> demand;
        demands[stop] = demand;
        cout << "stop, demand: " << stop << ", " << demand << "\n";
    }
}

void read_routes(ifstream &file, int num_vertices, int *route_matrix) {
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

void write_route_matrix(int *routes, int num_vertices, const string &filename) {
    ofstream outFile(filename);
    if (!outFile.is_open()) {
        cout << "Error opening output file." << endl;
        return;
    }

    for (int i = 0; i < num_vertices; i++) {
        for (int j = 0; j < num_vertices; j++) {
            outFile << routes[i * num_vertices + j] << " ";
        }
        outFile << "\n";
    }
    outFile.close();
}

void permute(vector<int> &route, int start, vector<vector<int>> &permutations) {
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

__global__ void filter_routes(int *permutations, int *route_matrix, int *demands, int *valid_routes, int max_weight, int max_stops, int max_route_size,int n_permutations) {
    int tId = (threadIdx.x + blockIdx.x * blockDim.x);


    if(tId>n_permutations-1){
        return;
    }
    int route_index = (tId*max_route_size) + 1;


    int cost = 0;
    int weight = 0;
    for(int i = 0; i<max_route_size-1; i++){

        int current_stop = permutations[route_index+i];    
        int next_stop = permutations[route_index+i+1];

    }
}

cpu_Route get_cheapest(vector<cpu_Route> valid_routes, vector<vector<int>> route_matrix) {
    int lowest_cost = numeric_limits<int>::max();
    cpu_Route cheapest_route;
    for (int i = 0; i < int(valid_routes.size()); i++) {
        if (valid_routes[i].totalCost < lowest_cost) {
            lowest_cost = valid_routes[i].totalCost;
            cheapest_route = valid_routes[i];
        }
    }
    return cheapest_route;
}

void debug_route(vector<int> route) {
    for (int i = 0; i < int(route.size()); i++) {
        cout << route[i] << "\n";
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }
    string file_name = argv[1];
    ifstream file(file_name);

    int num_vertices;
    file >> num_vertices;
    int *demands = (int *)malloc(num_vertices * sizeof(int));
    read_demands(file, demands, num_vertices);

    int *route_matrix = (int *)malloc(num_vertices * num_vertices * sizeof(int));
    read_routes(file, num_vertices, route_matrix);

    vector<vector<int>> permutations = generate_permutations(num_vertices);
    int num_routes = permutations.size();

    size_t total_size = 0;
    for (const auto &vec : permutations) {
        total_size += vec.size();
    }

    int *flat_permutations = new int[total_size];
    size_t index = 0;
    for (const auto &vec : permutations) {
        for (int val : vec) {
            flat_permutations[index++] = val;
        }
    }

    int max_weight = 15;
    int max_stops = 7;
    int *valid_routes = new int[max_stops * num_routes];

    int *d_valid_routes, *d_permutations, *d_route_matrix, *d_demands;

    int max_route_size = 2 * num_vertices + 1;
    cudaMalloc((void **)&d_valid_routes, sizeof(int) * max_route_size * num_routes);
    cudaMalloc((void **)&d_permutations, sizeof(int) * total_size);
    cudaMalloc((void **)&d_route_matrix, sizeof(int) * num_vertices * num_vertices);
    cudaMalloc((void **)&d_demands, sizeof(int) * num_vertices);

    cudaMemcpy(d_permutations, flat_permutations, sizeof(int) * total_size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_route_matrix, route_matrix, sizeof(int) * num_vertices * num_vertices, cudaMemcpyHostToDevice);
    cudaMemcpy(d_demands, demands, sizeof(int) * num_vertices, cudaMemcpyHostToDevice);

    int blockSize = 256;
    int numBlocks = (num_routes + blockSize - 1) / blockSize;

    filter_routes<<<numBlocks, blockSize>>>(d_permutations, d_route_matrix, d_demands, d_valid_routes, max_weight, max_stops, max_route_size);
    cudaMemcpy(valid_routes, d_valid_routes, sizeof(int) * max_route_size * num_routes, cudaMemcpyDeviceToHost);

    file.close();
    return 0;
}
