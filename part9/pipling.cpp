#include "pipling.hpp"
//Constractor
Pipling::Pipling() {}
//Distractor
Pipling::~Pipling(){ stop(); }

// Acuator 4 threads
void Pipling::start(){
    t1 = std::thread([this]{ stage1(); });
    t2 = std::thread([this]{ stage2(); });
    t3 = std::thread([this]{ stage3(); });
    t4 = std::thread([this]{ stage4(); });
}

// Close safty all threads
void Pipling::stop(){
    q1.push(nullptr);
    if (t1.joinable()) t1.join();
    if (t2.joinable()) t2.join();
    if (t3.joinable()) t3.join();
    if (t4.joinable()) t4.join();
}

// Receives and streams a graph for processing, push job to q1
void Pipling::submit(const Graph& g){
    // transfer graph to constractor
    auto job = std::make_shared<Job>(g); 
    q1.push(std::move(job));
}

// Wait for final result
Pipling::Result Pipling::get(){
    for(;;){
        JobPtr j = qout.pop();     // Wait for results
        if (j) return j->result;
    }
}

//Pop a Job from q1, compute MST weight, push to q2 graph and result (job) stop on sentinel.
void Pipling::stage1(){
    for(;;){
        // wait for next item from q1
        JobPtr j = q1.pop();
        // got sentinel: forward it to q2 and exit this stage
        if (!j){ q2.push(nullptr); break; }
        j->result.mst_weight = j->graph.mstWeight();
        q2.push(std::move(j));
    }
}

//Pop a Job from q2, count cliques, push to q3 graph and result (job) stop on sentinel.
void Pipling::stage2(){
    for(;;){
        JobPtr j = q2.pop();
        // got sentinel: forward it to q3 and exit this stage
        if (!j){ q3.push(nullptr); break; }
        j->result.num_cliques = j->graph.countCliques();
        q3.push(std::move(j));
    }
}

//Pop a Job from q3, find SCCs, push to q4 graph and result (job) stop on sentinel.
void Pipling::stage3(){
    for(;;){
        JobPtr j = q3.pop();
        // got sentinel: forward it to q4 and exit this stage
        if (!j){ q4.push(nullptr); break; }
        j->result.sccs = j->graph.findSCCs();
        q4.push(std::move(j));
    }
}

//Pop a Job from q4, computes max flow, push to qout graph and result (job) stop on sentinel.
void Pipling::stage4(){
    for(;;){
        JobPtr j = q4.pop();
        // got sentinel: forward it to qout and exit this stage
        if (!j){ qout.push(nullptr); break; }
        j->result.max_flow = j->graph.maxFlow(0, j->graph.getNumVertices() - 1);
        qout.push(std::move(j));
    }
}
