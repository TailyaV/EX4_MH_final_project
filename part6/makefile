#make
#make run-server
#make run-client
#make valgrind-server
#make valgrind-client
#make helgrind-server
#make helgrind-client
#make callgrind-server
#make callgrind-client
#make coverage-server
#make coverage-client


CXX = g++
CXXFLAGS = -Wall -std=c++17 -fprofile-arcs -ftest-coverage

SERVER_TARGET = euler_server
CLIENT_TARGET = euler_client

SRCS = euler_server.cpp graph.cpp
HEADERS = graph.hpp

OBJS = $(SRCS:.cpp=.o)

#Build Rules

$(SERVER_TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(SERVER_TARGET) $(OBJS)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(CLIENT_TARGET): euler_client.o
	$(CXX) $(CXXFLAGS) -o $(CLIENT_TARGET) euler_client.o

euler_client.o: euler_client.cpp
	$(CXX) $(CXXFLAGS) -c euler_client.cpp -o euler_client.o

# Clean

clean:
	rm -f *.o *.gcda *.gcno *.gcov *.info $(SERVER_TARGET) $(CLIENT_TARGET)

#Run Targets

run-server: $(SERVER_TARGET)
	./$(SERVER_TARGET)

run-client: $(CLIENT_TARGET)
	./$(CLIENT_TARGET)

#Memory Checks

valgrind-server: $(SERVER_TARGET)
	valgrind --leak-check=full ./$(SERVER_TARGET)

valgrind-client: $(CLIENT_TARGET)
	valgrind --leak-check=full ./$(CLIENT_TARGET)

#Thread Race Detection

helgrind-server: $(SERVER_TARGET)
	valgrind --tool=helgrind ./$(SERVER_TARGET)

helgrind-client: $(CLIENT_TARGET)
	valgrind --tool=helgrind ./$(CLIENT_TARGET)

#Performance Profiling

callgrind-server: $(SERVER_TARGET)
	valgrind --tool=callgrind ./$(SERVER_TARGET)

callgrind-client: $(CLIENT_TARGET)
	valgrind --tool=callgrind ./$(CLIENT_TARGET)

#Code Coverage

coverage-server: $(SERVER_TARGET) $(CLIENT_TARGET)
	@echo "Starting server..."
	./$(SERVER_TARGET) &
	SERVER_PID=$$! ; \
	sleep 1 ; \
	echo -e "3\n0 1\n1 2\n2 0\nend" | ./$(CLIENT_TARGET) ; \
	wait $$SERVER_PID
	gcov euler_server.cpp graph.cpp > /dev/null
	@echo "Coverage for server and graph generated."


coverage-client: $(CLIENT_TARGET)
	echo -e "3\n0 1\n1 2\n2 0\nend" | ./$(CLIENT_TARGET)
	gcov euler_client.cpp > /dev/null
	@echo "Coverage for client generated."
