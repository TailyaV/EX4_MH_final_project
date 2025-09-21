#make 
#./graph_test


#compiler
CXX = g++
CXXFLAGS = -Wall -std=c++17 -fprofile-arcs -ftest-coverage

#target executable
TARGET = graph_test

# Source files
SRC = main.cpp graph.cpp
OBJ = $(SRC:.cpp=.o)

#build the program
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ) -lgcov

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	del /Q *.o *.gcno *.gcda *.gcov $(TARGET).exe 2>nul || rm -f *.o *.gcno *.gcda *.gcov $(TARGET) nul

run: $(TARGET)
	./$(TARGET)

coverage: run
	gcov graph.cpp
	gcov main.cpp


#run with Valgrind memory check
valgrind-mem: $(TARGET)
	valgrind --leak-check=full --track-origins=yes ./$(TARGET)

#run with Valgrind thread race detection
valgrind-helgrind: $(TARGET)
	valgrind --tool=helgrind ./$(TARGET)

#run with Valgrind performance profiling
valgrind-callgrind: $(TARGET)
	valgrind --tool=callgrind ./$(TARGET)