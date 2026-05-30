# Simple Makefile for the ray tracer
# Usage:
#   make            # build (release by default)
#   make run        # build then run ./raytrace
#   make clean      # remove build artifacts
#   make debug      # clean and build with debug flags
#   make release    # clean and build with release flags

APP := raytrace
BUILD_DIR := build
APP_PATH := $(BUILD_DIR)/$(APP)
SRCS := main.cpp object.cpp object_tree.cpp
OBJS := $(SRCS:%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

CXX ?= g++
PKG_CONFIG ?= pkg-config
EIGEN_INC ?= /usr/include/eigen3

OPENCV_CFLAGS := $(shell $(PKG_CONFIG) --cflags opencv4 2>/dev/null)
OPENCV_LIBS   := $(shell $(PKG_CONFIG) --libs   opencv4 2>/dev/null)

# Build type: release (default) or debug
BUILD ?= release

BASE_CXXFLAGS := -std=gnu++17 -Wall -Wextra -I$(EIGEN_INC) $(OPENCV_CFLAGS)
ifeq ($(BUILD),debug)
  CXXFLAGS := $(BASE_CXXFLAGS) -g -O0 -DDEBUG_BUILD
else
  CXXFLAGS := $(BASE_CXXFLAGS) -O2 -DNDEBUG
endif

LDFLAGS := $(OPENCV_LIBS)

.PHONY: all run run-headless clean debug release docs docs-install docs-clean docs-serve

all: $(APP_PATH)

$(APP_PATH): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# C++ compilation with automatic dependency generation
$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

run: $(APP_PATH)
	$(APP_PATH)

run-headless: $(APP_PATH)
	RT_HEADLESS=1 $(APP_PATH)

clean:
	rm -rf $(BUILD_DIR) $(APP)

# Convenience targets that rebuild with the desired build type
debug:
	$(MAKE) clean
	$(MAKE) BUILD=debug all

release:
	$(MAKE) clean
	$(MAKE) BUILD=release all

# Documentation targets
docs: docs-install
	@echo "Generating documentation with Doxygen..."
	@if command -v doxygen >/dev/null 2>&1; then \
		cd docs && doxygen Doxyfile && \
		echo "Documentation generated successfully in docs/html/"; \
		echo "Open docs/html/index.html in your browser to view it."; \
	else \
		echo "Error: Doxygen is not installed."; \
		echo "Run 'make docs-install' to install it."; \
		exit 1; \
	fi

docs-install:
	@echo "Checking for Doxygen installation..."
	@if ! command -v doxygen >/dev/null 2>&1; then \
		echo "Doxygen not found. Installing..."; \
		if command -v apt >/dev/null 2>&1; then \
			sudo apt update && sudo apt install -y doxygen graphviz; \
		elif command -v yum >/dev/null 2>&1; then \
			sudo yum install -y doxygen graphviz; \
		elif command -v brew >/dev/null 2>&1; then \
			brew install doxygen graphviz; \
		else \
			echo "Please install doxygen manually for your system."; \
			exit 1; \
		fi; \
		echo "Doxygen installed successfully!"; \
	else \
		echo "Doxygen is already installed."; \
	fi

docs-clean:
	@echo "Cleaning documentation files..."
	rm -rf docs/

docs-serve: docs
	@echo "Starting local documentation server..."
	@if command -v python3 >/dev/null 2>&1; then \
		echo "Open http://localhost:8080 in your browser"; \
		cd docs/html && python3 -m http.server 8080; \
	elif command -v python >/dev/null 2>&1; then \
		echo "Open http://localhost:8080 in your browser"; \
		cd docs/html && python -m SimpleHTTPServer 8080; \
	else \
		echo "Python not found. Please open docs/html/index.html manually."; \
	fi
