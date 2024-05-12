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
vector<vector<int>> filter_routes(vector<vector<int>> permutations, vector<vector<int>> real_routes, map<int, int> demands, int max_cost, int max_size) {

    vector<vector<int>> valid_routes;

    for (int r = 0; r < int(permutations.size()); r++) {
        vector<int> route = permutations[r];
        int cost = 0;

        int size = route.size() - 1;
        bool valid = true;
        for (int i = 0; i < size; i++) {
            int current_stop = route[i];
            int next_stop = route[i + 1];

            int current_cost = real_routes[current_stop][next_stop] + demands[next_stop]; // cost of going to next stop + next stop's demand cost
            bool over_cost = cost + current_cost > max_cost;

            if (current_cost == 0 || over_cost) {               //  if any constraint is violated we try to add 0 in between the edges of the violator to try and fix, ADICIONAR CALCULO DO CUSTO
                int return_cost = real_routes[current_stop][0]; // cost of returning to the origin

                if ((real_routes[current_stop][0] == 0) || (real_routes[0][next_stop] == 0) || (return_cost + cost > max_cost)) { // if there isnt a route from current_stop to 0 or from 0 to next stop the route cannot be fixed.
                    valid = false;
                    break; // exiting the loop as the current route cannot be fixed
                }

                route.insert(route.begin() + i + 1, 0);
                cost = 0; // resetting cost as the vehicle returned to the origin
                current_cost = return_cost;
                size++;
            }
            cost += current_cost;
        }

        if (valid) {
            if(route.size() <= max_size){
                valid_routes.push_back(route);
            }
            
        }
    }
    return valid_routes;
}

// function to calculate the total cost of a route
int route_cost(vector<int> route, vector<vector<int>> route_matrix) {
    int cost = 0;
    for (int i = 0; i < int(route.size()) - 1; i++) {
        cost += route_matrix[i][i + 1];
    }

    return cost;
}

// function that receives the valid routes and returns the one with smallest cost
vector<int> get_cheapest(vector<vector<int>> valid_routes, vector<vector<int>> route_matrix) {
    int lowest_cost = numeric_limits<int>::max();
    vector<int> cheapest_route;
    for (int i = 0; i < int(valid_routes.size()); i++) {
        int current_cost = route_cost(valid_routes[i], route_matrix);
        if (current_cost < lowest_cost) {
            lowest_cost = current_cost;
            cheapest_route = valid_routes[i];
        }
    }
    return cheapest_route;
}

void debug_route(vector<int> route){
    for (int i = 0; i<int(route.size()); i++){
        cout<< route[i];
        cout<< "\n";
    }
}

// vector<int> solveVRP(map<int, int> stops, vector<vector<int>> routes) // receives a list of stops and their demands, and a graph of routes and costs
// {
//     vector<vector<int>> best_route;
//

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
    map<int, int> demands = read_demands(file);                               // store the stops and their demands in the demands map
    vector<vector<int>> route_matrix = read_routes(file, demands.size() + 1); // adding 1 to account for the origin as a vertex in the graph
    vector<vector<int>> permutations = generate_permutations(route_matrix.size());
    int max_cost = 190; // hardcoding max cost to test
    int max_size = 7; // hardcoding max size to test
    vector<vector<int>> valid_routes = filter_routes(permutations, route_matrix, demands, max_cost, max_size);
    vector<int> cheapest_route = get_cheapest(valid_routes, route_matrix);
    vector<vector<int>> results;
    results.push_back(cheapest_route);
    write_routes(results, "debug.txt"); // writing function to debug
    // cout << permutations.size();
    // cout << "\n";
    // cout << valid_routes.size();
    // cout << "\n";

    file.close();

    // solving

    return 0;
}
