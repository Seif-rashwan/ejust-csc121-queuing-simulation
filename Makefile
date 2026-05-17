# If on Windows, ensure the compiler's bin directory is prepended to the PATH
# to prevent any "DLL Hell" conflicts with Git Bash, if outdated.

ifeq ($(OS),Windows_NT)
  COMPILER_PATH := $(shell cygpath -w $$(which g++) 2>/dev/null)
  ifneq ($(COMPILER_PATH),)
    export PATH := $(dir $(COMPILER_PATH));$(PATH)
  endif
endif

CXX      = g++
CXXFLAGS = -Wall -Wextra -Wpedantic -std=c++23 -g -Iinclude
DEPFLAGS = -MMD -MP -MF $(@:.o=.d)

# ──────────────────────────────────────────────────────────────────────────────
# Sources:
# ────────
# Core library, no main(), shared by all targets
LIB_SRC = src/CustomerType.cpp \
          src/ServerType.cpp \
          src/ServerListType.cpp \
          src/WaitingCustomerQueue.cpp

# Simulation engine used by the web mode
SIM_SRC = src/SimulationEngine.cpp

# Entry points (separate targets for different modes)
CLI_MAIN_SRC = src/CLIMain.cpp
WEB_MAIN_SRC = src/WebMain.cpp

# ──────────────────────────────────────────────────────────────────────────────
# Object files:
# ─────────────
LIB_OBJ      = $(LIB_SRC:src/%.cpp=build/obj/%.o)
SIM_OBJ      = build/obj/SimulationEngine.o
WEB_MAIN_OBJ = build/obj/WebMain.o
CLI_MAIN_OBJ = build/obj/CLIMain.o
WEB_ALL_OBJ  = $(LIB_OBJ) $(SIM_OBJ) $(WEB_MAIN_OBJ)
CLI_ALL_OBJ  = $(LIB_OBJ) $(SIM_OBJ) $(CLI_MAIN_OBJ)
DEP          = $(WEB_ALL_OBJ:.o=.d) $(CLI_MAIN_OBJ:.o=.d)

# ──────────────────────────────────────────────────────────────────────────────
# Output Binaries:
# ────────────────
ifeq ($(OS),Windows_NT)
  WEB_TARGET = build/bin/simulation.exe
  CLI_TARGET = build/bin/simulation_cli.exe
  # Detect if we are in MINGW/MSYS (Unix-like shell on Windows)
  ifneq (,$(findstring MINGW,$(shell uname -s 2>/dev/null)))
    MKDIR_P = mkdir -p
    RM_RF   = rm -rf
  else
    MKDIR_P = powershell -Command "New-Item -ItemType Directory -Force -Path"
    RM_RF   = powershell -Command "Remove-Item -Recurse -Force -ErrorAction SilentlyContinue"
  endif
else
  WEB_TARGET = build/bin/simulation
  CLI_TARGET = build/bin/simulation_cli
  MKDIR_P    = mkdir -p
  RM_RF      = rm -rf
endif

.PHONY: all help build build-web build-cli run run-cli debug lint format clean install
.DELETE_ON_ERROR:

all: $(WEB_TARGET) $(CLI_TARGET)

help:
	@echo "Available commands:"
	@echo "  make build      Build all binaries (Web and CLI)"
	@echo "  make run        Build and start the Web simulation (Node.js)"
	@echo "  make run-cli    Build and start the CLI simulator"
	@echo "  make install    Install system and project dependencies"
	@echo "  make lint       Run static analysis (cpplint, cppcheck, clang-tidy)"
	@echo "  make format     Auto-format all C++ files"
	@echo "  make clean      Remove all build artifacts"
	@echo "  make debug      Start Node.js with --inspect for Chrome DevTools"

build: all
	@echo "[+] Build complete: $(WEB_TARGET) $(CLI_TARGET)"

build-web: $(WEB_TARGET)
	@echo "[+] Web binary: $(WEB_TARGET)"

build-cli: $(CLI_TARGET)
	@echo "[+] CLI binary: $(CLI_TARGET)"

# ──────────────────────────────────────────────────────────────────────────────
# Compilation Rules:
# ──────────────────
# Build binaries
$(WEB_TARGET): $(WEB_ALL_OBJ) | build/bin
	$(CXX) $(CXXFLAGS) -o $@ $^

# CLI binary: lib + engine + CLI entry point
$(CLI_TARGET): $(CLI_ALL_OBJ) | build/bin
	$(CXX) $(CXXFLAGS) -o $@ $^

# Build objects
build/obj/%.o: src/%.cpp | build/obj
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Disabled: -MMD dependency generation causes silent Error 1 on virtual/mapped drives (like G:) or some MSYS2/Windows setups
# -include $(DEP)

# Directory creation targets
build/bin:
	@$(MKDIR_P) build/bin

build/obj:
	@$(MKDIR_P) build/obj

# ──────────────────────────────────────────────────────────────────────────────
# Installation & Commands:
# ────────────────────────
debug: $(WEB_TARGET)
	@echo "Starting Node.js server in debug mode (inspector enabled on port 9229)..."
	@echo "Open chrome://inspect in your browser to attach."
	cd server && node --inspect server.js

# Web mode — builds web binary then starts Node.js server
run: $(WEB_TARGET)
	@echo "Starting Node.js server..."
	cd server && node server.js

# CLI mode — builds CLI binary then runs it interactively
run-cli: $(CLI_TARGET)
	@echo "Running CLI simulator..."
	./$(CLI_TARGET)

lint:
	@echo "Running cpplint..."
	cpplint --recursive src/ include/
	@echo ""
	@echo "Running cppcheck..."
	cppcheck --enable=all --inconclusive --std=c++23 --language=c++ \
      --suppress=missingIncludeSystem -Iinclude --inline-suppr src/ -I include 
	@echo ""
	@echo "Running clang-format..."
	clang-format --dry-run --Werror \
      $(LIB_SRC) $(SIM_SRC) $(CLI_MAIN_SRC) $(WEB_MAIN_SRC) $(wildcard include/*.h)
	@echo ""
	@echo "Running clang-tidy..."
	clang-tidy $(LIB_SRC) $(SIM_SRC) $(CLI_MAIN_SRC) $(WEB_MAIN_SRC) -- $(CXXFLAGS)

# ──────────────────────────────────────────────────────────────────────────────
# Maintenance:
# ────────────────────────
install:
	@echo "Installing dependencies..."
	@echo "1. Installing pre-commit framework..."
	python -m pip install pre-commit
	@echo "2. Installing Node.js dependencies..."
	cd server && npm install
	@echo "3. Setting up git hooks..."
	python -m pre_commit install
	@echo "[+] Installation complete; start by running 'make run' or 'make run-cli'!"

clean:
	@echo "Cleaning build artifacts..."
	@$(RM_RF) build
	@echo "[+] Clean complete."

format:
	clang-format -i \
      $(LIB_SRC) $(SIM_SRC) $(CLI_MAIN_SRC) $(WEB_MAIN_SRC) $(wildcard include/*.h)
