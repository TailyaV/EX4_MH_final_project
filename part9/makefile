# === Compiler & Flags ===
CXX      := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -g -pthread
COVFLAGS := -fprofile-arcs -ftest-coverage
LDFLAGS  := -pthread

# === Objects ===
COMMON_OBJS := graph.o pipling.o
SERVER_OBJS := server.o $(COMMON_OBJS)
CLIENT_OBJS := client.o graph.o

# ==== Valgrind flags ====
VAL              := valgrind
MEMCHECK_FLAGS   := --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1
HELGRIND_FLAGS   := --tool=helgrind --history-level=approx --error-exitcode=1
CALLGRIND_FLAGS  := --tool=callgrind
CACHEGRIND_FLAGS := --tool=cachegrind

# === Default: build both ===
all: server client

# --- Link ---
server: $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) $(SERVER_OBJS) -o server $(LDFLAGS)

client: $(CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) $(CLIENT_OBJS) -o client $(LDFLAGS)

server.o: server.cpp pipling.hpp graph.hpp
	$(CXX) $(CXXFLAGS) -c server.cpp -o server.o

pipling.o: pipling.cpp pipling.hpp
	$(CXX) $(CXXFLAGS) -c pipling.cpp -o pipling.o

graph.o: graph.cpp graph.hpp
	$(CXX) $(CXXFLAGS) -c graph.cpp -o graph.o

client: client.o graph.o
	$(CXX) $(CXXFLAGS) client.o graph.o -o client $(THREADS)

client.o: client.cpp graph.hpp
	$(CXX) $(CXXFLAGS) -c client.cpp -o client.o

# === Graph Coverage Report Target ===
graph_cov: 
	$(CXX) $(CXXFLAGS) $(COVFLAGS) graph.cpp test.cpp -o graph_cov_exec
	./graph_cov_exec
	gcov *graph*.gcda

# === pipling Coverage Report Target ===
pipling_cov: 
	$(CXX) $(CXXFLAGS) $(COVFLAGS)  pipling.cpp graph.cpp test_pipling.cpp -o pipling_cov_exec
	./pipling_cov_exec
	gcov *pipling*.gcda

# === Client Coverage Report Target ===
client_cov: pipling.cpp graph.cpp client.cpp test_client.cpp
	$(CXX) $(CXXFLAGS) -DUNIT_TEST $(COVFLAGS) pipling.cpp graph.cpp client.cpp test_client.cpp -o client_cov_exec
	./client_cov_exec
	gcov *client*.gcda

# === Server Coverage Report Target ===
server_cov: pipling.cpp graph.cpp client.cpp test_server.cpp
	$(CXX) $(CXXFLAGS) -DUNIT_TEST $(COVFLAGS) pipling.cpp graph.cpp server.cpp test_server.cpp -o server_cov_exec
	./server_cov_exec
	gcov *server*.gcda

# ==== Test binaries (no coverage) ====
graph_tests: graph.cpp test.cpp
	$(CXX) $(CXXFLAGS) -DUNIT_TEST graph.cpp test.cpp -o graph_tests $(LDFLAGS)

pipling_tests: pipling.cpp graph.cpp test_pipling.cpp
	$(CXX) $(CXXFLAGS) -DUNIT_TEST pipling.cpp graph.cpp test_pipling.cpp -o pipling_tests $(LDFLAGS)

client_tests: client.cpp graph.cpp test_client.cpp
	$(CXX) $(CXXFLAGS) -DUNIT_TEST client.cpp graph.cpp test_client.cpp -o client_tests $(LDFLAGS)

server_tests: server.cpp pipling.cpp graph.cpp test_server.cpp
	$(CXX) $(CXXFLAGS) -DUNIT_TEST server.cpp pipling.cpp graph.cpp test_server.cpp -o server_tests $(LDFLAGS)

# pattern rules: memcheck-<bin>, helgrind-<bin>, ...
memcheck-%: %
	$(VAL) $(MEMCHECK_FLAGS) ./$(<)

helgrind-%: %
	$(VAL) $(HELGRIND_FLAGS) ./$(<)

callgrind-%: %
	rm -f callgrind.out.*
	$(VAL) $(CALLGRIND_FLAGS) ./$(<)
	callgrind_annotate --auto=yes callgrind.out.* | head -n 80

cachegrind-%: %
	rm -f cachegrind.out.*
	$(VAL) $(CACHEGRIND_FLAGS) ./$(<)
	cg_annotate cachegrind.out.* | head -n 80

# Run all tools on all test binaries
valgrind-all: graph_tests pipling_tests client_tests server_tests \
              memcheck-graph_tests memcheck-pipling_tests memcheck-client_tests memcheck-server_tests \
              helgrind-graph_tests helgrind-pipling_tests helgrind-client_tests helgrind-server_tests \
              callgrind-graph_tests callgrind-pipling_tests callgrind-client_tests callgrind-server_tests \
              cachegrind-graph_tests cachegrind-pipling_tests cachegrind-client_tests cachegrind-server_tests


# === Convenience ===
run_server: server
	./server

run_client: client
	./client

# === Clean ===
.PHONY: all clean run_server run_client
clean:
	rm -f *.o server client *.gcno *.gcda graph_cov_exec *.gcov pipling_cov_exec client_cov_exec server_cov_exec graph_tests pipling_tests client_tests server_tests
