CC        := gcc
CFLAGS    := -Wall -Wextra
LDFLAGS   :=

SRC_DIR    := src
BUILD_DIR  := build
OUTPUT_DIR := output

TARGET := $(BUILD_DIR)/main

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

.PHONY: all build package clean test clean-build-test clean-build build-test format

all: build-test

build: $(TARGET)

$(TARGET): $(OBJS)
	mkdir -p $(@D)
	$(CC) $(OBJS) $(LDFLAGS) -o $@ || { \
		echo "ERROR: Build failed."; \
		exit 1; \
	}

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@ || { \
		echo "ERROR: Build failed while compiling $<."; \
		exit 1; \
	}

package:
	mkdir -p $(OUTPUT_DIR)
	cp -r $(SRC_DIR)/* $(OUTPUT_DIR)/

clean:
	rm -rf $(BUILD_DIR) $(OUTPUT_DIR)

test:
	@if ! command -v valgrind >/dev/null 2>&1; then \
		echo "ERROR: Valgrind is not installed."; \
		exit 1; \
	fi
	valgrind --leak-check=yes $(TARGET)

format:
	@if ! command -v clang-format >/dev/null 2>&1; then \
		echo "ERROR: clang-format is not installed."; \
		exit 1; \
	fi
	clang-format -i $(SRCS)

clean-build-test: clean build test
clean-build: clean build
build-test: build test