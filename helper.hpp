#ifndef HELPER_HPP
#define HELPER_HPP

#include <fstream>
#include <iostream>
#include <limits>
#include <vector>
#include <algorithm>
using namespace std;

class Route {
public:
    vector<int> stops;
    int totalCost;
};

// Function prototypes
vector<int> read_demands(ifstream &file);
vector<vector<int>> read_routes(ifstream &file, int num_stops);
vector<vector<int>> generate_permutations(int num_points);
Route get_cheapest(vector<Route> valid_routes);
void debug_route(vector<int> route);
vector<Route> filter_routes(vector<vector<int>> permutations, vector<vector<int>> real_routes, vector<int> demands, int max_weight, int max_stops);
void write_routes(const vector<Route> &routes, const string &filename);

#endif // HELPER_HPP
