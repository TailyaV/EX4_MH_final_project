#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include <cerrno>  

// Reads from 'sock' until it sees 'delim' (e.g., '\n').
// Returns true if a full message was extracted; false otherwise.
bool recv_until_delim(int sock, char delim, std::string& out, std::string& stash) {
    char buf[1024];
    while (true) {
        // Look for the delimiter in the bytes we already have
        size_t pos = stash.find(delim);
        if (pos != std::string::npos) {
            // take message up to the delimiter
            out = stash.substr(0, pos); 
            // remove that message + the delimiter from stash
            stash.erase(0, pos + 1); 
            return true; 
        }

        // Need more data: read from the socket (may block if socket is blocking)
        ssize_t n = read(sock, buf, sizeof(buf));
        if (n == 0) {
            //no more bytes coming
            return false;
        }
        if (n < 0) {
            // read was interrupted by a signal; try again
            if (errno == EINTR) continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                //no data available right now
                return false;
            }
            return false;
        }

        //Append newly read bytes to stash and loop to search for the delimiter again
        stash.append(buf, buf + n);
    }
}

//Function for sending request to server
bool send_request(int sock){
    std::cout << "Choose Action:\n1. Send graph\n2. Random graph\nFor ending send '0'\n";
    int choice; std::cin >> choice;
    //if(choice == 0) return false;
    while (choice < 0 || choice > 2) {
            std::cout << "Unknown option, choose new one:\n"; 
            std::cin >> choice; 
        }

    //Sending request
    write(sock, &choice, sizeof(choice));

    if (choice == 1) { //Send graph
        std::cout << "Enter number of vertices: ";
        int V; std::cin >> V;
        write(sock, &V, sizeof(V));

        std::cout << "Enter edges (u v), -1 -1 to end:\n";
        int u, v;
        for (;;) {
            std::cin >> u >> v;
            write(sock, &u, sizeof(u));
            write(sock, &v, sizeof(v));
            if (u == -1 && v == -1) break;
        }
    } else if(choice == 2){ //random graph
        std::cout << "Enter number of vertices: ";
        int V; 
        std::cin >> V; write(sock, &V, sizeof(V));

        std::cout << "Enter number of edges: ";
        int e; 
        std::cin >> e; write(sock, &e, sizeof(e));

        std::cout << "Enter seed: ";
        int s; 
        std::cin >> s; write(sock, &s, sizeof(s));
    }else{ // choice == 0, no more requests
        return false;
    }
    return true;
}

#ifndef UNIT_TEST
int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr{};

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    std::string stash; 
    while(true){
        if(!send_request(sock)) break;

        std::cout << "Graph sent to server.\n";
        //Recive respond from server- results of all algorithms
        std::string msg;
        bool ok = recv_until_delim(sock, '}', msg, stash);
        if (!ok) break;              // no more data
        std::cout << "Algorithms results: " << msg << "\n";
    }
    close(sock);
    return 0;
}
#endif