# ====================================================================
#    IFJ projekt 2026
#
#    Původní autor: xsantar00
#    Seznam změn:
#			pridano mazani souboru output.txt a ifjcode.txt
#
#    makefile se stara a kompilaci specifickych zdrojovych kodu pro jejich nasledne otestovani v izolovanem prostredi
# ====================================================================

.DEFAULT_GOAL := test

CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -I../src
LDFLAGS :=

SRC_DIR := ../src
SCRIPTS_DIR := scripts

COMPONENTS := $(patsubst $(SCRIPTS_DIR)/%/,%,$(wildcard $(SCRIPTS_DIR)/*/))

.PHONY: all test component-test component-build clean

all: test

# A test cannot be selected without selecting a component
ifneq ($(strip $(TEST)),)
ifeq ($(strip $(COMPONENT)),)
$(error TEST requires COMPONENT to be specified)
endif
endif

# Validate selected component
ifneq ($(strip $(COMPONENT)),)
ifeq ($(filter $(COMPONENT),$(COMPONENTS)),)
$(error Unknown component: $(COMPONENT))
endif
endif

# Shared Python test script
TEST_SCRIPT := $(SCRIPTS_DIR)/test.py

ifeq ($(strip $(COMPONENT)),)

# Run all components
test:
	@for comp in $(COMPONENTS); do \
		$(MAKE) COMPONENT=$$comp component-build || exit 1; \
	done
	@python3 $(TEST_SCRIPT)

else

COMPONENT_DIR := $(SCRIPTS_DIR)/$(COMPONENT)
BUILD_DIR := $(COMPONENT_DIR)/build

RUNNER_SRC := $(COMPONENT_DIR)/runner.c
RUNNER_BIN := $(BUILD_DIR)/runner

ifeq ($(wildcard $(COMPONENT_DIR)/sources.mk),)
$(error Missing $(COMPONENT_DIR)/sources.mk)
endif

include $(COMPONENT_DIR)/sources.mk

SOURCE_OBJS := $(addprefix $(BUILD_DIR)/,$(addsuffix .o,$(basename $(notdir $(SOURCES)))))

.PHONY: component-build

component-build: $(RUNNER_BIN)

test: component-test

component-test: $(RUNNER_BIN)
	@python3 $(TEST_SCRIPT) $(COMPONENT) $(TEST)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/runner.o: $(RUNNER_SRC)
	@mkdir -p $(@D)
	@echo "Compiling $(RUNNER_SRC)"
	@$(CC) $(CFLAGS) -c $< -o $@

$(RUNNER_BIN): $(BUILD_DIR)/runner.o $(SOURCE_OBJS)
	@echo "Linking $(COMPONENT) runner"
	@$(CC) $^ $(LDFLAGS) -o $@

endif

clean:
	@rm -rf $(foreach component,$(COMPONENTS),$(SCRIPTS_DIR)/$(component)/build)
	$(if $(COMPONENTS),@find $(foreach component,$(COMPONENTS),$(SCRIPTS_DIR)/$(component)) -type f \( -name "output.txt" -o -name "ifjcode.txt" \) -delete 2>/dev/null || true)
	@echo "Test builds cleaned."
