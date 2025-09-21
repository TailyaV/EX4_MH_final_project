#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
#include "graph.hpp"

#define PORT 8080
#define BUFFER_SIZE 4096

using namespace std;

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE] = {0};

    //Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    //Set up address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; //Accept connections from any IP
    server_addr.sin_port = htons(PORT);       //Convert to network byte order

    //Bind the socket
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    //Start listening
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    cout << "Server is running and waiting for connections on port " << PORT << "...\n";

    //Accept a connection
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    cout << "Client connected!\n";

    //Read data from client
    read(client_fd, buffer, BUFFER_SIZE);
    cout << "Received:\n" << buffer << endl;

    //first line = number of vertices
    //Remaining lines: one edge per line: u v
    istringstream iss(buffer); //create a stream from the received buffer
    int numVertices;
    iss >> numVertices; //read number of vertices

    Graph g(numVertices, false); //create an undirected graph with that many vertices
    int u, v;
    while (iss >> u >> v) {  //read edges until input ends
        g.addEdge(u, v);
    }

    //Process the graph
    string response;
    if (g.hasEulerCircuit()) {
        vector<int> circuit = g.findEulerCircuit();
        response = "Euler Circuit: ";
        for (int node : circuit) {
            response += to_string(node) + " ";
        }
    } else {
        response = "No Euler Circuit";
    }

    // Send response to client
    send(client_fd, response.c_str(), response.length(), 0);
    cout << "Result sent to client.\n";

    // Close connection
    close(client_fd);
    close(server_fd);

    return 0;
}
