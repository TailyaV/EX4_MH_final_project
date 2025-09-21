#include "factory.hpp"
#include "graph.hpp"
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <netinet/in.h>

int main() {
    int server_fd, new_socket;
    sockaddr_in address{};
    int opt = 1, addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0); //create tcp socket
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)); //allow reuse of port

    address.sin_family = AF_INET; //ipv4
    address.sin_addr.s_addr = INADDR_ANY; //listen on all interfaces
    address.sin_port = htons(8080); //set port 8080

    bind(server_fd, (struct sockaddr*)&address, sizeof(address)); //bind socket to address
    listen(server_fd, 3); //start listening for connections
    std::cout << "Server listening on port 8080...\n"; //debug message

    new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen); //accept client connection

    int algorithmChoice;
    read(new_socket, &algorithmChoice, sizeof(int)); //read selected algorithm from client

    int vertices;
    read(new_socket, &vertices, sizeof(int)); //read number of vertices
    Graph g(vertices, true); //create directed graph

    int u, v;
    while (true) {
        read(new_socket, &u, sizeof(int)); //read edge start
        read(new_socket, &v, sizeof(int)); //read edge end
        if (u == -1 && v == -1) break; //end of edges
        g.addEdge(u, v); //add edge to graph
    }

    auto strategy = GraphAlgorithmFactory::create((AlgorithmType)algorithmChoice); //create selected algorithm
    strategy->execute(g); //run algorithm on graph

    close(new_socket); //close client connection
    close(server_fd); //shut down server
    return 0;
}
