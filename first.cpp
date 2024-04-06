#include <iostream>
#include <vector>
#include <fstream>
using namespace std;

// lists all possible vertices
vector<int> get_all_nodes(int num_vertices){
    vector<int> nodes;
    for (int i = 0; i<num_vertices; i++){
        nodes.push_back(i);
    }
    return nodes;
}

vector<vector<int>> find_max_clique(vector<vector<int>> graph, int num_vertices){ // Returns the biggest clique found.
    vector <int> maximum_clique;
    vector <int> candidates = get_all_nodes(num_vertices); // initially all nodes are candidates

    while(candidates.size() != 0)
    {
        int v  = candidates.back();
        candidates.pop_back();

        bool valid = true;

        for (int u=0; u< maximum_clique.size(); u++)
        {
            if(graph[u][v] == 0)
            {
                valid = false;
                break;
            }
        }

        if(valid)
        {
            maximum_clique.push_back(v);
            vector <int> new_candidates; // NAO SEI SE ESTA CORRETO
            
            for (int u = 0; u < new_candidates.size(); u ++){
                int candidate = new_candidates.at(u);
                bool adjacent = true;

                for (int c = 0; c < maximum_clique.size(); c++)
                {
                    int node = maximum_clique.at(c);

                    if(graph[candidate][c] == 0)
                    {
                        adjacent = false;
                        break;
                    }
                }
                if(adjacent)
                {
                    new_candidates.push_back(candidate);
                    break;
                }
            }

            candidates = new_candidates;
        }
    
    }

    
}



// read graph from input
vector<vector<int>> read_graph(const string& nomeArquivo, int& num_vertices) {
    ifstream arquivo(nomeArquivo);
    int num_edges;
    arquivo >> num_vertices >> num_edges;

    vector<vector<int>> graph(num_vertices, vector<int>(num_vertices, 0));

    for (int i = 0; i < num_edges; ++i) {
        int u, v;
        arquivo >> u >> v;
        graph[u - 1][v - 1] = 1;
        graph[v - 1][u - 1] = 1;  // The graph is undirected
    }

    arquivo.close();

    return graph; // returns a V x V matrix, where V is the number of vertices and V[i][j] = 1 if there is an edge in between i e j.
}


int main(){
    int num_vertices;
    vector<vector<int>> graph = read_graph("graph.txt", num_vertices);

    find_max_clique();

    return 0;
}