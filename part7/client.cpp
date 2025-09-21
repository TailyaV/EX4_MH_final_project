#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    //create TCP socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    //set up server address (localhost:8080)
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    //connect to server
    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    //choose algorithm
    std::cout << "Choose Algorithm:\n";
    std::cout << "1. MST\n2. Cliques\n3. SCC\n4. Max Flow\n> ";
    int choice;
    std::cin >> choice;
    write(sock, &choice, sizeof(int));

    //number of vertices
    std::cout << "Enter number of vertices: ";
    int V;
    std::cin >> V;
    write(sock, &V, sizeof(int));

    //send edges
    std::cout << "Enter edges (u v), -1 -1 to end:\n";
    int u, v;
    while (true) {
        std::cin >> u >> v;
        write(sock, &u, sizeof(int));
        write(sock, &v, sizeof(int));
        if (u == -1 && v == -1) break;
    }

    std::cout << "Graph sent to server.\n";
    close(sock);
    return 0;
}
