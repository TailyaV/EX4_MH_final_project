#include "pipling.hpp"
#include "graph.hpp"
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <netinet/in.h>

//Function for print all the results of 4 algorithms
std::string to_string(Pipling::Result res){
    std::ostringstream out;
    out << "\nMST weight:\n" << res.mst_weight << std::endl;
    out << "number of cliques:\n" << res.num_cliques << std::endl;
    out << "Strongly Connected Components:\n";
    auto sccs = res.sccs;
    for (const auto& component : sccs) {
        for (int v : component)
            out << v << " ";
        out << "\n";
    }
    out << "max flow:\n" << res.max_flow;

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
bool my_handler(int new_socket) {
    int choice = 0;
        if (!read_exact(new_socket, &choice, sizeof(int))) return false;

        Graph g(0, true);

        if (choice == 1) { // GRAPH
            int vertices;
            if (!read_exact(new_socket, &vertices, sizeof(int))) return false;
            g = Graph(vertices, true);

            int u, v;
            while (true) {
                if (!read_exact(new_socket, &u, sizeof(int))) return false;
                if (!read_exact(new_socket, &v, sizeof(int))) return false;
                if (u == -1 && v == -1) break;
                g.addEdge(u, v);
            }
        } else if (choice == 2) { // RANDOM GRAPH
            int vertices, edges, seed;
            if (!read_exact(new_socket, &vertices, sizeof(int))) return false;
            if (!read_exact(new_socket, &edges, sizeof(int)))   return false;
            if (!read_exact(new_socket, &seed, sizeof(int)))    return false;
            g = Graph::buildRandGraph(edges, vertices, seed);
        }else if(choice == 0){
            return false;
        }else{
            throw std::invalid_argument("error: Unknown command");
        }
        //Create instance of Pipling
        Pipling pipling;
        //Create threds on start function
        pipling.start();
        pipling.submit(g);
        Pipling::Result res = pipling.get();
        pipling.stop();
        std::string out = to_string(res);
        std::cout << "Sending graph algorithms results..." << std::endl;
        //Send algorithms result to client 
        write_all(new_socket, out.c_str(), out.size());
        return true;
        
}

int bind_listen(int port) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) throw std::system_error(errno, std::generic_category(), "socket");

    int yes = 1;
    //Allows both reuse after closing, and opening multiple sockets simultaneously on the same port.
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &yes, sizeof(yes));

    sockaddr_in a{};
    a.sin_family      = AF_INET;
    a.sin_addr.s_addr = htonl(INADDR_ANY);
    a.sin_port        = htons((uint16_t)port);
    size_t alen = sizeof(a);
    if (bind(fd, (sockaddr*)&a, sizeof(a)) < 0) {
        int e = errno; close(fd);
        throw std::system_error(e, std::generic_category(), "bind");
    }
    if (listen(fd, SOMAXCONN) < 0) {
        int e = errno; ::close(fd);
        throw std::system_error(e, std::generic_category(), "listen");
    }
     int new_socket = accept(fd, (struct sockaddr*)&a, (socklen_t*)&alen);
    return new_socket;
}

#ifndef UNIT_TEST
int main() {
    const int PORT = 8080;
    
    //Creates an instance of lf server
    std::cout << "Server listening on port " << PORT << "...\n"
              << "Press ENTER to stop.\n";
    int fd = bind_listen(PORT);
    //get request from client and send back respond
    while(true){
        if(!my_handler(fd)){
            break;
        }
    }
    std::cout << "Stopped.\n";
    //Close connection
    shutdown(fd, SHUT_WR);
    close(fd);
    return 0;
}
#endif
