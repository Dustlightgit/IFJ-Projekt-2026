# ====================================================================
#   IFJ projekt 2026
#
#   Původní autor: xsantar00
#   Seznam změn:
#		- uprava testovani: xsantar00
#
#   makefile se stara a kompilaci specifickych zdrojovych kodu pro jejich nasledne otestovani v izolovanem prostredi
# ====================================================================

CC        := gcc
CFLAGS    := -Wall -Wextra
LDFLAGS   :=

SRC_DIR    := src
BUILD_DIR  := build
OUTPUT_DIR := output

PACKAGE_FILES := Makefile rozsireni

TARGET := $(BUILD_DIR)/main

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

.PHONY: all build package clean test clean-build-test clean-build build-test format build-run run

all: build-test

run: build
	@./$(TARGET)

build: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(@D)
	@$(CC) $(OBJS) $(LDFLAGS) -o $@ || { \
		@echo "ERROR: Build failed."; \
		@exit 1; \
	}

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@ || { \
		@echo "ERROR: Build failed while compiling $<."; \
		@exit 1; \
	}

package:
	@mkdir -p $(OUTPUT_DIR)
	@rm -f $(OUTPUT_DIR)/archive.zip
	@zip -r $(OUTPUT_DIR)/archive.zip $(SRC_DIR)/* $(PACKAGE_FILES)
	@./is_it_ok.sh $(OUTPUT_DIR)/archive.zip

clean:
	rm -rf $(BUILD_DIR) $(OUTPUT_DIR)

test:
	@if ! command -v valgrind >/dev/null 2>&1; then \
		@echo "ERROR: Valgrind is not installed."; \
		@exit 1; \
	fi
	valgrind --leak-check=yes $(TARGET)
	$(MAKE) -C test

format:
	@if ! command -v clang-format >/dev/null 2>&1; then \
		@echo "ERROR: clang-format is not installed."; \
		@exit 1; \
	fi
	clang-format -i $(SRCS)

clean-build-test: clean build test
clean-build: clean build
build-test: build test