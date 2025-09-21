#pragma once
#include <functional>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

class LFServer {
public:
    using Handler = std::function<void(int)>;   // Defines a short name for the “Colback” (function)

    LFServer(int port, int threads, Handler on_client); //Constractor for lf with port, number of threads and function
    ~LFServer(); //Close socket if needed
    void start();  //Runs the server until stopped
    void request_stop(); //Stop all threads saftly by indicats leader_free_ true.

private:
    int port, threads;
    Handler function;

    int listen_fd_ = -1; //Maintains the fd value for listening to receive incoming connections.
    std::atomic<bool> stopping_{false}; //A stop flag that is synchronized between all threads

    std::mutex m_; //Lock to synchronize leader selection and access to the condition variable leader_free_
    std::condition_variable cv_; //Used to wait for Leader–Follower coordination, Until leader_free_ becomes true
    bool leader_free_ = true; //A flag that indicates whether there is a leader or not

    int  bind_listen(); //Open a TCP socket and return file discreptor
    void worker_loop(); //Main function of lf: make current thread to a leader, wait for client connection, transfers leadership to the next thread, and handle the connection.
    void become_leader_blocking(); //Choose leader
    void promote_new_leader(); //Transfers leadership to the next thread.
    void handle_one(int cfd); //Runs the handler that provided on the client's fd, then closes the connection.
};
