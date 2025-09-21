# ./euler_test -e 5 -v 4 -s 89
# === Compiler and Flags ===
CXX = g++                            # The C++ compiler to use
CXXFLAGS = -Wall -std=c++17 -g       # Enable warnings and use C++17 standard

# === Target and Object Files ===
TARGET = euler_test                 # Name of the final executable
OBJS = main.o graph.o               # Object files to compile and link

# === Default Target: Build the Executable ===
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# === Compile main.cpp into main.o ===
main.o: main.cpp graph.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

# === Compile graph.cpp into graph.o ===
graph.o: graph.cpp graph.hpp
	$(CXX) $(CXXFLAGS) -c graph.cpp -o graph.o

# === Clean Target: Remove compiled files ===
clean:
	# This command works for both Windows and Linux
	del /Q *.o *.exe 2>nul || rm -f *.o *.exe $(TARGET) nul

# === Optional: Run the Program ===
run: $(TARGET)
	./$(TARGET)
