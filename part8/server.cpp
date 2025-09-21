#include "lf.hpp"
#include "graph.hpp"
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <netinet/in.h>

//Function for run and print all the results of 4 algorithms
static std::string run_all_algorithms(Graph& g){
    std::ostringstream out;
    //out << "\n{";
    out << "rum all algorithm:" << std::endl;
    out << "Graph adjacency list:\n";
    for (int u = 0; u < g.getNumVertices(); ++u) {
        out << u << ":";
        for (int v : g.getNeighbors(u)) out << " " << v;
        out << "\n";
    }
    out << "\nMST algorithm:" << std::endl;
    int mw = g.mstWeight();
    out << "MST weight:" << mw << std::endl;
    out << "\nCOUNT CLIQUES algorithm:" << std::endl;
    int c = g.countCliques();
    out << "number of cliques:" << c << std::endl;
    out << "\nFIND SCCs algorithm:" << std::endl;
    out << "Strongly Connected Components:\n";
    auto sccs = g.findSCCs();
    for (const auto& component : sccs) {
        for (int v : component)
            out << v << " ";
        out << "\n";
    }
    out << "\nMAX FLOW algorithm:" << std::endl;
    int mf = g.maxFlow(0, g.getNumVertices() - 1);
    out << "max flow:" << mf << std::endl;

    out << "}";
    return out.str();
}

//Function for reading all n byts precisely
static bool read_exact(int fd, void* buf, size_t n) {
    char* p = (char*)buf; size_t left = n;
    while (left) {
        ssize_t r = ::read(fd, p, left);
        if (r == 0) return false;            //close client
        if (r < 0) { if (errno == EINTR) continue; return false; }
        p += r; left -= (size_t)r;
    }
    return true;
}

//Function for writting all n byts precisely
static bool write_all(int fd, const void* buf, size_t n) {
    const char* p = static_cast<const char*>(buf);
    size_t left = n;
    while (left) {
        ssize_t w = ::write(fd, p, left);
        if (w < 0) { if (errno == EINTR) continue; return false; }
        p += w;
        left -= (size_t)w;
    }
    return true;
}

//Callbeck function for lf, get the client massage and sand back answer
void my_handler(int new_socket) {
    while(true){
    int choice = -1;
        if (!read_exact(new_socket, &choice, sizeof(int))) return;
        if(choice == 0) break;
        Graph g(0, true);

        if (choice == 1) { // GRAPH
            int vertices;
            if (!read_exact(new_socket, &vertices, sizeof(int))) return;
            g = Graph(vertices, true);

            int u, v;
            while (true) {
                if (!read_exact(new_socket, &u, sizeof(int))) return;
                if (!read_exact(new_socket, &v, sizeof(int))) return;
                if (u == -1 && v == -1) break;
                g.addEdge(u, v);
            }
        } else if (choice == 2) { // RANDOM GRAPH
            int vertices, edges, seed;
            if (!read_exact(new_socket, &vertices, sizeof(int))) return;
            if (!read_exact(new_socket, &edges, sizeof(int)))   return;
            if (!read_exact(new_socket, &seed, sizeof(int)))    return;
            g = Graph::buildRandGraph(edges, vertices, seed);
        }else {
            throw std::invalid_argument("error: Unknown command");
        }

        std::string out = run_all_algorithms(g);
        std::cout << "Sending graph algorithms results..." << std::endl;
        //Send algorithms result to client 
        write_all(new_socket, out.c_str(), out.size());
    }
}

#ifndef UNIT_TEST
int main() {
    const int PORT = 8080;
    const int THREADS = 4;

    LFServer::Handler handler = my_handler;
    
    //Creates an instance of lf server
    LFServer srv(PORT, THREADS, handler);
    std::cout << "Server listening on port " << PORT
              << " with " << THREADS << " threads (LF)...\n"
              << "Press ENTER to stop.\n";
    //Start lf server
    std::thread t([&](){ srv.start(); });
    std::string input; 
    std::getline(std::cin, input);
    srv.request_stop();
    t.join();
    std::cout << "Stopped.\n";
    return 0;
}
#endif
