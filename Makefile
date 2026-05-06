CXX      = g++
CXXFLAGS = -Wall -Wextra -Wpedantic -std=c++23 -Iinclude
SRC      = $(wildcard src/*.cpp)
OBJ      = $(SRC:src/%.cpp=build/%.o)
TARGET   = simulation

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/%.o: src/%.cpp | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

build:
	mkdir -p build

run: all
	./$(TARGET)

debug: CXXFLAGS += -g -O0
debug: all

lint:
	clang-tidy $(SRC) -- $(CXXFLAGS)

format:
	clang-format -i $(SRC) $(wildcard include/*.h)

clean:
	rm -rf build $(TARGET) $(TARGET).exe
