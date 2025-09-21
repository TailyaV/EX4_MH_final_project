#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 4096

using namespace std;

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    //Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    //Convert IPv4 address from text to binary
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address / Address not supported");
        return 1;
    }

    //Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return 1;
    }

    //input of user
    cout << "Enter number of vertices:" << endl;
    string input;
    getline(cin, input);
    stringstream graph_data;
    graph_data << input << "\n";

    cout << "Now enter the edges (u v), one per line. Type 'end' to finish:\n";
    while (true) {
        getline(cin, input);
        if (input == "end") break;
        graph_data << input << "\n";
    }

    string data_to_send = graph_data.str();

    //Send the graph data to the server
    send(sock, data_to_send.c_str(), data_to_send.size(), 0);

    // Read the response
    read(sock, buffer, BUFFER_SIZE);
    cout << "Server response: " << buffer << endl;

    //Close the socket
    close(sock);
    return 0;
}
