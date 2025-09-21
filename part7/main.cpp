#include "graph.hpp"
#include "strategy.hpp"
#include "factory.hpp"
#include <iostream>
using namespace std;

int main() {
    int n, choice;
    cout << "Enter number of vertices: ";
    cin >> n;
    Graph g(n, false);

    cout << "Enter edges (u v), -1 -1 to end:\n";
    while (true) {
        int u, v;
        cin >> u >> v;
        if (u == -1 || v == -1) break;
        g.addEdge(u, v);
    }

    while (true) {
        cout << "\nChoose Algorithm:\n";
        cout << "1. MST weight\n";
        cout << "2. Count Cliques\n";
        cout << "3. SCCs\n";
        cout << "4. Max Flow (0 to n-1)\n";
        cout << "0. Exit\n> ";
        cin >> choice;

        if (choice == 0) break;

        //Create strategy using Factory
        auto strategy = GraphAlgorithmFactory::create(static_cast<AlgorithmType>(choice));  
        if (!strategy) {
            cout << "Invalid choice.\n";
            continue;
        }

        //Execute strategy
        strategy->execute(g);
    }

    return 0;
}
