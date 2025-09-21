#include "lf.hpp"
#include <iostream>
#include <system_error>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

//Constractor for lf with port, number of threads and function
LFServer::LFServer(int port, int threads, Handler h)
    : port(port), threads(threads), function(std::move(h)) {}

//Close socket if needed
LFServer::~LFServer() {
    int fd = -1;
    { std::lock_guard<std::mutex> lk(m_); fd = listen_fd_; listen_fd_ = -1; }
    if (fd != -1) ::close(fd);
}

//Open a TCP socket and return file discreptor
int LFServer::bind_listen() {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) throw std::system_error(errno, std::generic_category(), "socket");

    int yes = 1;
    //Allows both reuse after closing, and opening multiple sockets simultaneously on the same port.
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &yes, sizeof(yes));

    sockaddr_in a{};
    a.sin_family      = AF_INET;
    a.sin_addr.s_addr = htonl(INADDR_ANY);
    a.sin_port        = htons((uint16_t)port);

    if (bind(fd, (sockaddr*)&a, sizeof(a)) < 0) {
        int e = errno; close(fd);
        throw std::system_error(e, std::generic_category(), "bind");
    }
    if (listen(fd, SOMAXCONN) < 0) {
        int e = errno; ::close(fd);
        throw std::system_error(e, std::generic_category(), "listen");
    }
    return fd;
}

//Runs the server until stopped
void LFServer::start() {
    //open tcp socket
    listen_fd_ = bind_listen();
    //Updating the flag under lockdown for true
    { std::lock_guard<std::mutex> lk(m_); leader_free_ = true; }
    //Create threads and run worker loop for each 
    std::vector<std::thread> pool;
    pool.reserve(threads);
    for (int i = 0; i < threads; ++i)
        pool.emplace_back([this]{ worker_loop(); });

    for (auto& t : pool) t.join();
}

//Stop all threads saftly by indicats leader_free_ true.
void LFServer::request_stop() {
    stopping_.store(true, std::memory_order_relaxed);
    int fd = -1;
    {
        std::lock_guard<std::mutex> lk(m_);
        fd = listen_fd_; //safe read
        leader_free_ = true;
        cv_.notify_all();          
    }
    if (fd != -1) ::shutdown(fd, SHUT_RDWR);
}

//Choose leader
void LFServer::become_leader_blocking() {
    std::unique_lock<std::mutex> lk(m_);
    //wait for leader_free_ to be true
    cv_.wait(lk, [&]{ return leader_free_ || stopping_.load(); });
    if (stopping_.load()) return;
    leader_free_ = false; //defines a leader
}

//Transfers leadership to the next thread.
void LFServer::promote_new_leader() {
    std::lock_guard<std::mutex> lk(m_);
    leader_free_ = true;
    cv_.notify_one();
}

//Runs the handler that provided on the client's fd, then closes the connection.
void LFServer::handle_one(int cfd) {
    try { function(cfd); }
    catch (const std::exception& e) { std::cerr << "handler error: " << e.what() << "\n"; }
    close(cfd);
}

//Main function of lf: make current thread to a leader, wait for client connection, transfers leadership to the next thread, and handle the connection.
void LFServer::worker_loop() {
    while (!stopping_.load()) {
        become_leader_blocking();
        if (stopping_.load()) break;

        int cfd = accept(listen_fd_, nullptr, nullptr);

        // transfers leadership to the next thread
        promote_new_leader();

        if (cfd < 0) {
            if (stopping_.load()) break;
            if (errno == EINTR || errno == EAGAIN) continue;
            std::perror("accept");
            continue;
        }
        handle_one(cfd);
    }
}
