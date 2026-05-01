CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
SRC      = src/Main.cpp src/CustomerType.cpp src/ServerType.cpp src/ServerListType.cpp src/WaitingCustomerQueue.cpp
TARGET   = simulation

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)
