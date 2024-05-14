#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <vector>
using namespace std;

class Route{
    public:
    vector<int> stops;
    int totalCost;
};

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

void write_routes(const vector<Route> &routes, const string &filename) { // debug function to write the cost matrix to a file
    ofstream outFile(filename);

    if (!outFile.is_open()) {
        cout << "Error opening output file." << endl;
        return;
    }

    Route route;
    for (int i = 0; i<routes.size(); i++) {
        route = routes[i];
        for (int j = 0; j < route.stops.size(); j++) {
            int stop = route.stops[j];
            outFile << stop << " ";
        }
        outFile << " | ";
        outFile << route.totalCost;
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
vector<Route> filter_routes(vector<vector<int>> permutations, vector<vector<int>> real_routes, map<int, int> demands, int max_weight, int max_stops) {

    vector<Route> valid_routes;

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
            int next_weight = demands[next_stop];
            bool over_weight = weight + next_weight > max_weight;

            if (current_cost == 0 || over_weight) {               //  if any constraint is violated we try to add 0 in between the edges of the violator to try and fix, ADICIONAR CALCULO DO CUSTO
                int return_cost = real_routes[current_stop][0]; // cost of returning to the origin

                if ((real_routes[current_stop][0] == 0) || (real_routes[0][next_stop] == 0)) { // if there isnt a route from current_stop to 0 or from 0 to next stop the route cannot be fixed.
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
            new_route.totalCost = cost;
            valid_routes.push_back(new_route);           
        }
    }
    return valid_routes;
}

// function that receives the valid routes and returns the one with smallest cost
Route get_cheapest(vector<Route> valid_routes, vector<vector<int>> route_matrix) {
    int lowest_cost = numeric_limits<int>::max();
    Route cheapest_route;
    Route route;
    for (int i = 0; i < int(valid_routes.size()); i++) {
        route = valid_routes[i];
        if (route.totalCost < lowest_cost) {
            lowest_cost = route.totalCost;
            cheapest_route = route;
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
    int max_weight = 15; // hardcoding max cost to test
    int max_stops = 7; // hardcoding max size to test
    vector<Route> valid_routes = filter_routes(permutations, route_matrix, demands, max_weight, max_stops);
    Route cheapest_route = get_cheapest(valid_routes, route_matrix);
    vector<Route> results;
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
