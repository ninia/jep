# Makefile

# Define variables
PYTHON = python
SETUP = setup.py

# Default target
all: build

# Build target
build:
	$(PYTHON) $(SETUP) build

# Clean target to remove build artifacts
clean:
	$(PYTHON) $(SETUP) clean

# Install target (optional, if you want to include it)
install:
	$(PYTHON) $(SETUP) install

# Uninstall target (optional, if you want to include it)
uninstall:
	$(PYTHON) $(SETUP) uninstall

.PHONY: all build clean install uninstall

