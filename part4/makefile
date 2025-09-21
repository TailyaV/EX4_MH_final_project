# ./euler_test -e 5 -v 4 -s 89

# === Compiler and Flags ===
CXX = g++
CXXFLAGS = -Wall -std=c++17 -g

# === Targets and Object Files ===
TARGET = euler_test
OBJS = main.o graph.o
ARGS = -e 5 -v 4 -s 42

TEST_TARGET = tests
MAIN_TEST_TARGET = main_tests
TEST_OBJS = test.o graph.o
MAIN_TEST_OBJS = main_test.o graph.o main_ut.o

# === Build main executable ===
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# object file for main
main.o: main.cpp graph.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

# object file for graph
graph.o: graph.cpp graph.hpp
	$(CXX) $(CXXFLAGS) -c graph.cpp -o graph.o

# === Build test executable ===
$(TEST_TARGET): $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $(TEST_OBJS)

# === Build main test executable ===
$(MAIN_TEST_TARGET): $(MAIN_TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $(MAIN_TEST_TARGET) $(MAIN_TEST_OBJS)

# === Build main_test binary (uses doctest main, no real main from main.cpp) ===
main_ut.o: main.cpp graph.hpp
	$(CXX) $(CXXFLAGS) -DUNIT_TESTING -c $< -o $@

# object file for test
test.o: test.cpp graph.hpp
	$(CXX) $(CXXFLAGS) -c test.cpp -o test.o

# object file for main_test
main_test.o: main_test.cpp graph.hpp main.hpp
	$(CXX) $(CXXFLAGS) -c main_test.cpp -o main_test.o

# object file for main for coverage
main_ut_cov.o: main.cpp graph.hpp
	$(CXX) $(CXXFLAGS) -fprofile-arcs -ftest-coverage -DUNIT_TESTING -c $< -o $@

# object file for main_test for coverage
main_test_cov.o: main_test.cpp graph.hpp
	$(CXX) $(CXXFLAGS) -fprofile-arcs -ftest-coverage -c $< -o $@

# === Graph Coverage Report Target ===
graph_cov: 
	$(CXX) $(CXXFLAGS) -fprofile-arcs -ftest-coverage  graph.cpp test.cpp -o graph_cov_exec
	./graph_cov_exec
	gcov *graph*.gcda

# === Main Coverage Report Target ===
main_cov: main_ut_cov.o graph.o main_test_cov.o
	$(CXX) $(CXXFLAGS) -fprofile-arcs -ftest-coverage $^ -o main_cov_exec
	./main_cov_exec
	gcov *main*.gcda

# === Gprof Report Target ===
gprof_report:
	$(CXX) $(CXXFLAGS) -pg main.cpp graph.cpp -o euler_gprof
	./euler_gprof $(ARGS)
	gprof ./euler_gprof gmon.out > gprof_report.txt

# === Valgrind Main Target ===
valgrind_report_main: 
	$(CXX) $(CXXFLAGS) main.cpp graph.cpp -o valgrind_main
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./valgrind_main $(ARGS)

# === Valgrind Graph Test Target ===
valgrind_report_test: 
	$(CXX) $(CXXFLAGS) test.cpp graph.cpp -o valgrind_test
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./valgrind_test

# === Valgrind Test Main Target ===
valgrind_report_test_main: main_ut.o main_test.o graph.o
	$(CXX) $(CXXFLAGS) $^ -o valgrind_main_test
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./valgrind_main_test

# === Valgrind CallGraph Target ===
 valgrind_callGraph:
	$(CXX) $(CXXFLAGS) main.cpp graph.cpp -o callGraph
	valgrind --tool=callgrind ./callGraph $(ARGS)
	callgrind_annotate --auto=yes callgrind.out.* > callgrind_report.txt

# === Run test ===
test: $(TEST_TARGET)
	./$(TEST_TARGET)

# === Run main test ===
main_test: $(MAIN_TEST_TARGET)
	./$(MAIN_TEST_TARGET)

# === Clean Target ===
clean:
	del /Q *.o *.exe 2>nul || rm -f *.o *.exe $(TARGET) $(TEST_TARGET) $(MAIN_TEST_TARGET) nul *.gcda *.gcno *.gcov graph_cov_exec main_cov_exec gprof_report gmon.out \
	gprof_report.txt euler_gprof valgrind_main valgrind_test valgrind_main_test callgrind_report.txt callGraph callgrind.out.*

# === Run main program ===
run: $(TARGET)
	./$(TARGET)
