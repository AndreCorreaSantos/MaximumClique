#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <vector>
using namespace std;

// read stops information from file
map<int, int> read_demands(ifstream &file) {

    int num_vertices; // numero de vertices contando com a origem, o vertice 0.
    file >> num_vertices;

    map<int, int> stops;
    for (int i = 1; i < num_vertices; i++) // vai de 1 a n_vertices
    {
        int stop;
        int demand;

        file >> stop >> demand;
        stops[stop] = demand;
        cout << "stop,demand:" << stop << "," << demand << "\n"; // debug
    }

    return stops;
}

vector<vector<int>> read_routes(ifstream &file, int num_stops) {
    int num_routes;
    file >> num_routes;
    vector<vector<int>> routes(num_stops, vector<int>(num_stops, 0)); // initializing all routes[u][v] = 0

    for (int i = 0; i < num_routes; i++) { // existing routes will store their respective cost
        int u, v, cost;
        file >> u >> v >> cost;
        routes[u][v] = cost;
        routes[v][u] = cost;
    }
    return routes;
}

void write_routes(const vector<vector<int>> &routes, const string &filename) { // debug function to write the cost matrix to a file
    ofstream outFile(filename);

    if (!outFile.is_open()) {
        cout << "Error opening output file." << endl;
        return;
    }

    for (const auto &row : routes) {
        for (int cost : row) {
            outFile << cost << " ";
        }
        outFile << endl;
    }

    outFile.close();
}

// function that receives the number of points and returns all of the possible permutations
vector<vector<int>> generate_permutations(int num_points) {
    vector<vector<int>> permutations;
    vector<int> route(num_points - 1); // Only permute num_points-1 elements

    // Initialize the route with 1, 2, ..., num_points-1
    for (int i = 1; i < num_points; i++) {
        route[i - 1] = i;
    }

    // Generate permutations using std::next_permutation
    do {
        vector<int> full_route;
        full_route.push_back(0); // Start with 0
        full_route.insert(full_route.end(), route.begin(), route.end());
        full_route.push_back(0); // End with 0
        permutations.push_back(full_route);
    } while (next_permutation(route.begin(), route.end()));

    return permutations;
}
// function that receives the permutations and tries to fix routes that violate constraints, 
vector<vector<int>> filter_routes(vector<vector<int>> permutations, vector<vector<int>> real_routes, int max_cost) {

    vector<vector<int>> valid_routes;

    for (int r = 0; r < int(permutations.size()); r++) {
        vector<int> route = permutations[r]; 
        int cost = 0;
 
        int size = route.size() - 1;
        bool valid = true;
        for (int i = 0; i < size; i++) { 
            int current_stop = route[i];
            int next_stop = route[i + 1];

            if ((real_routes[current_stop][next_stop] < 1)) { //  if any constraint is violated we try to add 0 in between the edges of the violator to try and fix, ADICIONAR CALCULO DO CUSTO

                if (real_routes[current_stop][0] == 0 || real_routes[0][next_stop] == 0) { // if there isnt a route from current_stop to 0 or from 0 to next stop the route cannot be fixed.
                    valid = false;
                    break; // exiting the loop as the current route cannot be fixed
                }

                route.insert(route.begin()+i+1, 0);
                size++;
            }
        }

        if (valid) {
            valid_routes.push_back(route);
        }
    }
    return valid_routes;
}

// // function that receives a graph of routes and returns all possible combinations to traverse the route and go from start, to start
// vector<vector<vector<int>>> generate_possible_combinations(vector<vector<int>> routes, int max_cost) {
// }

// vector<int> solveVRP(map<int, int> stops, vector<vector<int>> routes) // receives a list of stops and their demands, and a graph of routes and costs
// {
//     vector<vector<int>> best_route;
//     int lowest_cost = numeric_limits<int>::max();

//     vector<vector<vector<int>>> combinations = generate_possible_combinations(routes);
// }

int main(int argc, char *argv[]) {

    // reading data
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }
    string file_name = argv[1];
    ifstream file(file_name);
    // reading max cost
    // int max_cost; // first line in the file is the max possible cost in the car
    // file >> max_cost;
    map<int, int> stops = read_demands(file);                         // store the stops and their demands in the stops map
    vector<vector<int>> routes = read_routes(file, stops.size() + 1); // adding 1 to account for the origin as a vertex in the graph
    vector<vector<int>> permutations = generate_permutations(routes.size());
    vector<vector<int>> valid_routes = filter_routes(permutations,routes,0);

    write_routes(valid_routes,"debug.txt"); // writing function to debug
    // cout << permutations.size();
    // cout << "\n";
    // cout << valid_routes.size();
    // cout << "\n";

    file.close();

    // solving

    return 0;
}
