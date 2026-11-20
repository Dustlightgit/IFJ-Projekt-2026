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

.PHONY: all build package clean test format run rebuild show-docs stop-docs

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

PYTHON   ?= python3
PORT     ?= 8000
DOCS_PID := .docs-server.pid

show-docs:
	@if [ -f "$(DOCS_PID)" ] && kill -0 $$(cat "$(DOCS_PID)") >/dev/null 2>&1; then \
		echo "Documentation server already running at http://localhost:$(PORT) (PID $$(cat "$(DOCS_PID)"))."; \
		exit 0; \
	fi
	@rm -f "$(DOCS_PID)"
	@command -v $(PYTHON) >/dev/null 2>&1 || { echo "Error: '$(PYTHON)' not found in PATH."; exit 1; }
	@$(PYTHON) -c "import sys, http.server; assert sys.version_info >= (3, 6)" >/dev/null 2>&1 || { echo "Error: Python 3.6+ with http.server module is required."; exit 1; }
	@[ -d html ] || { echo "Error: 'html/' directory not found. Build the documentation first."; exit 1; }
	@$(PYTHON) -c "import socket; s = socket.socket(); s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1); s.bind(('', $(PORT))); s.close()" >/dev/null 2>&1 || { echo "Error: Port $(PORT) is already in use or requires elevated privileges."; exit 1; }
	@(cd html && nohup $(PYTHON) -m http.server $(PORT) >/dev/null 2>&1 & echo $$! > "$(CURDIR)/$(DOCS_PID)")
	@sleep 0.2
	@kill -0 $$(cat "$(DOCS_PID)") >/dev/null 2>&1 || { rm -f "$(DOCS_PID)"; echo "Error: Failed to start documentation server."; exit 1; }
	@echo "Documentation server running at http://localhost:$(PORT) (PID $$(cat "$(DOCS_PID)"))"

stop-docs:
	@if [ -f "$(DOCS_PID)" ]; then \
		PID=$$(cat "$(DOCS_PID)"); \
		if kill -0 "$$PID" >/dev/null 2>&1; then \
			kill "$$PID" >/dev/null 2>&1 && echo "Stopped documentation server (PID $$PID)."; \
		else \
			echo "Server process (PID $$PID) not running; removed stale PID file."; \
		fi; \
		rm -f "$(DOCS_PID)"; \
	else \
		echo "Documentation server is not running."; \
	fi