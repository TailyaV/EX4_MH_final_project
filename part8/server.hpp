#pragma once
#include "graph.hpp"
#include <string>

//Function for run and print all the results of 4 algorithms
static std::string run_all_algorithms(Graph& g);
//Function for reading all n byts precisely
static bool read_exact(int fd, void* buf, size_t n);
//Function for writting all n byts precisely
static bool write_all(int fd, const void* buf, size_t n);
//Callbeck function for lf, get the client massage and sand back answer
void my_handler(int new_socket);