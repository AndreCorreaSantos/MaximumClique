#include <iostream>
#include <vector>
#include <fstream>
#include <map>
using namespace std;

// read stops information from file
map<int,int> read_demands(ifstream& file) {
    
    int num_vertices; // numero de vertices contando com a origem, o vertice 0.
    file >> num_vertices;


    cout << "\nnvertices\n" << num_vertices << "\n";

    map<int,int> stops;
    for (int i = 1; i<num_vertices; i++) // vai de 1 a n_vertices
    {
        int stop;
        int demand;

        file >> stop >> demand;
        stops[stop] = demand;
        cout << "stop,demand:" << stop << "," << demand << "\n";
    }

    
    return stops;
}


vector< vector <int>> read_routes(ifstream& file, int num_stops) {
    int num_routes;
    file >> num_routes;
    vector<vector<int>> routes(num_stops, vector<int>(num_stops,0)); // initializing all routes[u][v] = 0

    for (int i = 0; i < num_routes; i++){ // existing routes will store their respective cost
        int u,v,cost;
        file >> u >> v >> cost;
        routes[u][v] = cost;
        routes[v][u] = cost;
    }
    return routes;
}

void write_routes(const vector<vector<int>>& routes, const string& filename) { // debug function to write the cost matrix to a file
    ofstream outFile(filename);

    if (!outFile.is_open()) {
        cout << "Error opening output file." << endl;
        return;
    }

    for (const auto& row : routes) {
        for (int cost : row) {
            outFile << cost << " ";
        }
        outFile << endl;
    }

    outFile.close();
}


int main(int argc, char *argv[]){
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }
    string file_name = argv[1];
    ifstream file(file_name); 
    map<int,int> stops = read_demands(file); // store the stops and their demands in the stops map 
    vector< vector <int>> routes = read_routes(file,stops.size()+1); // adding 1 to account for the origin as a vertex in the graph
    file.close();

    string out_file = "teste.txt";


    return 0;
}
