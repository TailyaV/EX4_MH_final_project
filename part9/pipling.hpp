#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <memory>
#include <vector>
#include "graph.hpp"

class Pipling {
public:
    //Stract for saving all algorithms results that activated by all threds
    struct Result {
        int mst_weight = -1;
        int num_cliques = -1;
        std::vector<std::vector<int>> sccs;
        int max_flow = -1;
    };

    Pipling(); //Constractor
    ~Pipling(); //Distractor
    void start();                  // Acuator 4 threads
    void stop();                   // Close safty all threads
    void submit(const Graph& g);   // Receives and streams a graph for processing
    Result get();                  // Wait for final result

private:
   struct Job { 
    Graph graph; // The graph to process
    Result result; // Accumulated results from pipeline stages
    // Build a Job by copying the graph
    explicit Job(const Graph& g) : graph(g) {}
    // explicit Job(Graph&& g) : graph(std::move(g)) {} // Move-construct if you prefer
    };

    // Shared handle to a Job (passed between stages)
    using JobPtr = std::shared_ptr<Job>;  

    // Thread queue
    template<typename T>
    class QueueT {
        std::queue<T> q;
        std::mutex m; // Protects access to the queue
        std::condition_variable cv; // Notifies waiting threads when items arrive
    public:
        void push(T v){
            std::unique_lock<std::mutex> lk(m);
            q.push(std::move(v)); // add item under lock  
            cv.notify_one(); // wake one waiting consumer
        }
        
        T pop(){
            // lock for waiting
            std::unique_lock<std::mutex> lk(m);  
            // block until queue not empty                         
            cv.wait(lk, [&]{ return !q.empty(); });  
            // take front item                    
            T v = std::move(q.front()); q.pop();                          
            return v;
        }
    };

    //Queues connecting the 4 pipeline stages (and final output)
    QueueT<JobPtr> q1, q2, q3, q4, qout;

    // One thread per pipeline stage
    std::thread t1, t2, t3, t4;

    void stage1();  // Computes MST weight and forwards Job to q2
    void stage2();  // Computes Cliques and forwards Job to q3
    void stage3();  // Computes SCCs and forwards Job to q4
    void stage4();  // Computes MaxFlow and pushes final Job to qout
};
