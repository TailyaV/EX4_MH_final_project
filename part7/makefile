#make
#./server
#./client
#covreage- 1.make coverage-server 2../server 3.make coverage-client  4../client
#- make valgrind-mem - make valgrind-helgrind - make valgrind-callgrind
#make valgrind-client 
#make valgrind-server


CXX = g++
CXXFLAGS = -Wall -std=c++17 -fprofile-arcs -ftest-coverage
LDFLAGS = -lgcov

#object files
OBJS = graph.o strategy.o factory.o

#default target
all: part7 client server

#cuild main executable (for normal usage)
part7: main.o $(OBJS)
	$(CXX) $(CXXFLAGS) -o part7 main.o $(OBJS)

#client and server
client: client.o $(OBJS)
	$(CXX) $(CXXFLAGS) -o client client.o $(OBJS)

server: server.o $(OBJS)
	$(CXX) $(CXXFLAGS) -o server server.o $(OBJS)

#coverage test build
test_main: test_main.o $(OBJS)
	$(CXX) $(CXXFLAGS) -o test_main test_main.o $(OBJS) $(LDFLAGS)

#compilation rules
main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp

client.o: client.cpp
	$(CXX) $(CXXFLAGS) -c client.cpp

server.o: server.cpp
	$(CXX) $(CXXFLAGS) -c server.cpp

test_main.o: test_main.cpp
	$(CXX) $(CXXFLAGS) -c test_main.cpp

graph.o: graph.cpp
	$(CXX) $(CXXFLAGS) -c graph.cpp

strategy.o: strategy.cpp
	$(CXX) $(CXXFLAGS) -c strategy.cpp

factory.o: factory.cpp
	$(CXX) $(CXXFLAGS) -c factory.cpp

#valgrind
valgrind-mem: part7
	valgrind --leak-check=full --track-origins=yes ./part7

valgrind-helgrind: part7
	valgrind --tool=helgrind ./part7

valgrind-callgrind: part7
	valgrind --tool=callgrind ./part7

coverage-client: client
	./client
	gcov client.cpp

#covrage 

coverage-server: server
	./server
	gcov server.cpp

#clean up all generated files
clean:
	rm -f *.o *.gcda *.gcno *.gcov part7 client server test_main
