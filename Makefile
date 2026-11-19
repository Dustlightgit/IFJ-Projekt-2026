# ====================================================================
#   IFJ projekt 2026
#
#   xsantar00 Radovan Santavy
#
#   Makefile
#
#   Seznam zmen:
#       - uprava testovani: xsantar00
#       - uprava package, nyni i testuje pomoci `is_it_ok.sh`: xbalaka00
#       - default je ted pouze build, opraven valgrind test a syntaxe: xsantar00
#
#   Makefile se stara o kompilaci specifickych zdrojovych kodu pro jejich
#   nasledne otestovani v izolovanem prostredi
# ====================================================================

CC        := gcc
CFLAGS    := -Wall -Wextra
LDFLAGS   :=

SRC_DIR    := src
BUILD_DIR  := build
OUTPUT_DIR := output

PACKAGE_FILES := Makefile rozsireni rozdeleni dokumentace.pdf

TARGET := $(BUILD_DIR)/main

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

.PHONY: all build package clean test format run rebuild

all: build

run: build
	@./$(TARGET)

build: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(@D)
	@$(CC) $(OBJS) $(LDFLAGS) -o $@ || { \
		echo "ERROR: Build failed."; \
		exit 1; \
	}
	@echo "Build successful."

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@ || { \
		echo "ERROR: Build failed while compiling $<."; \
		exit 1; \
	}

package:
	@mkdir -p $(OUTPUT_DIR)
	@rm -f $(OUTPUT_DIR)/archive.zip
	@zip -r $(OUTPUT_DIR)/archive.zip $(SRC_DIR)/* $(PACKAGE_FILES)
	@./is_it_ok.sh $(OUTPUT_DIR)/archive.zip

clean:
	@rm -rf $(BUILD_DIR) $(OUTPUT_DIR) *.log
	@$(MAKE) -C test clean
	@echo Clean done.

test: rebuild
	@if ! command -v valgrind >/dev/null 2>&1; then \
		echo "ERROR: Valgrind is not installed."; \
		exit 1; \
	fi
	valgrind --leak-check=yes --log-fd=9 $(TARGET) <test/scripts/pipeline/unit_tests/test_ahoj/input.txt 9>&2 >/dev/null 2>&1
	@$(MAKE) -C test

format:
	@if ! command -v clang-format >/dev/null 2>&1; then \
		echo "ERROR: clang-format is not installed."; \
		exit 1; \
	fi
	clang-format -i $(SRCS)

rebuild: clean build
