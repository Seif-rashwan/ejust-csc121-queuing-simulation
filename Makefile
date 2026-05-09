CXX      = g++
CXXFLAGS = -Wall -Wextra -Wpedantic -std=c++23 -Iinclude
DEPFLAGS = -MMD -MP

# ──────────────────────────────────────────────────────────────────────────────
# Sources:
# ────────
# Core library sources — no main(), compiled once for both targets
LIB_SRC = src/CustomerType.cpp \
           src/ServerType.cpp \
           src/ServerListType.cpp \
           src/WaitingCustomerQueue.cpp

# Web simulation entry point (Node.js spawned)
WEB_SRC = src/WebSimulation.cpp

# CLI entry point (standalone terminal use / course submission)
CLI_SRC = src/Main.cpp

# ──────────────────────────────────────────────────────────────────────────────
# Object files:
# ─────────────
LIB_OBJ = $(LIB_SRC:src/%.cpp=build/obj/%.o)
WEB_OBJ = build/obj/WebSimulation.o
CLI_OBJ = build/obj/Main.o
DEP     = $(LIB_OBJ:.o=.d) $(WEB_OBJ:.o=.d) $(CLI_OBJ:.o=.d)

# ──────────────────────────────────────────────────────────────────────────────
# Output binaries:
# ────────────────
WEB_TARGET = build/bin/simulation.exe
CLI_TARGET = build/bin/simulation_cli.exe

.PHONY: all cli build build-cli build-web run run-cli debug lint format clean install

# Default: build both binaries
all: $(WEB_TARGET) $(CLI_TARGET)

# Explicit build targets
build: all
	@echo "✓ Build complete: $(WEB_TARGET) $(CLI_TARGET)"

build-web: $(WEB_TARGET)
	@echo "✓ Web binary built: $(WEB_TARGET)"

build-cli: $(CLI_TARGET)
	@echo "✓ CLI binary built: $(CLI_TARGET)"

# ──────────────────────────────────────────────────────────────────────────────
# Installation & Setup:
# ────────────────────
install:
	@echo "Installing dependencies..."
	@echo "1. Installing pre-commit framework..."
	python -m pip install pre-commit
	@echo "2. Installing Node.js dependencies..."
	cd server && npm install
	@echo "3. Setting up git hooks..."
	python -m pre_commit install
	@echo "✓ Installation complete; start by running 'make run' or 'make cli'!"

# Web binary (Node.js uses this) — links library + WebSimulation
$(WEB_TARGET): $(LIB_OBJ) $(WEB_OBJ) | build/bin
	$(CXX) $(CXXFLAGS) -o $@ $^

# CLI binary (standalone terminal) — links library + Main
$(CLI_TARGET): $(LIB_OBJ) $(CLI_OBJ) | build/bin
	$(CXX) $(CXXFLAGS) -o $@ $^

# Pattern rule: compile any src/*.cpp into build/obj/*.o
build/obj/%.o: src/%.cpp | build/obj
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

-include $(DEP)

build/obj:
	node -e "fs.mkdirSync('build/obj', {recursive:true})"

build/bin:
	node -e "fs.mkdirSync('build/bin', {recursive:true})"

# ──────────────────────────────────────────────────────────────────────────────
# Quick commands:
#────────────────
debug: CXXFLAGS += -g -O0
debug: all

run: $(WEB_TARGET)
	@echo "Starting Node.js server with backend..."
	cd server && node server.js

run-cli: $(CLI_TARGET)
	@echo "Running CLI simulator..."
	./$(CLI_TARGET)


lint:
	clang-tidy $(LIB_SRC) $(CLI_SRC) -- $(CXXFLAGS)

format:
	clang-format -i $(LIB_SRC) $(CLI_SRC) $(wildcard include/*.h)

clean:
	@echo "Cleaning build artifacts..."
	node -e "fs.rmSync('build', {recursive:true, force:true})"
	@echo "✓ Clean complete!"
