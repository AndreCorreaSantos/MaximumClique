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
    int total_cost;
};

// Function prototypes
vector<int> read_demands(ifstream &file);
vector<vector<int>> read_routes(ifstream &file, int num_stops);
vector<vector<int>> generate_permutations(int num_points);
Route get_cheapest(vector<Route> valid_routes);
void debug_route(vector<int> route);
void write_routes(const vector<Route> &routes, const string &filename);

#endif // HELPER_HPP
