#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <vector>
#include <set>
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
    vector<int> route(num_points);

    // Initialize the route with 0, 1, 2, ..., num_points-1
    for (int i = 0; i < num_points; i++) {
        route[i] = i;
    }

    // Generate permutations using std::next_permutation
    while (next_permutation(route.begin(), route.end()))
    {
        permutations.push_back(route);
    }

    return permutations;
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
    int max_cost; // first line in the file is the max possible cost in the car
    file >> max_cost;
    map<int, int> stops = read_demands(file);                         // store the stops and their demands in the stops map
    vector<vector<int>> routes = read_routes(file, stops.size() + 1); // adding 1 to account for the origin as a vertex in the graph
    vector<vector<int>> permutation_routes = generate_permutations(routes.size());

    write_routes(permutation_routes,"debug.txt");

    file.close();

    // solving

    return 0;
}
