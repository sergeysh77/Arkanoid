# Makefile for Arkanoid Game
# Build for MSYS2 (32-bit or 64-bit)

# Compiler and tools
CC = gcc
CFLAGS = -Wall -O2 -I$(RES_DIR)
LIBS = -lgdi32 -lm -lwinmm -mwindows

# Directories
SRC_DIR = .
RES_DIR = res
RELEASE_DIR = Release

# Sources
SRCS = arkanoid.c levels.c powerups.c menu.c sounds.c language.c
OBJS = $(addprefix $(RELEASE_DIR)/, $(SRCS:.c=.o))

# Resources 
RESOURCE_RC = $(RES_DIR)/resources.rc
RESOURCE_H = $(RES_DIR)/resources.h
RESOURCE_OBJ = $(RELEASE_DIR)/resources.o


TARGET = $(RELEASE_DIR)/arkanoid.exe
all: $(RELEASE_DIR) $(TARGET)

# Create Release directory
$(RELEASE_DIR):
	mkdir -p $(RELEASE_DIR)

$(TARGET): $(OBJS) $(RESOURCE_OBJ)
	$(CC) $(OBJS) $(RESOURCE_OBJ) -o $@ $(LIBS)

# Compile resources 
$(RESOURCE_OBJ): $(RESOURCE_RC) $(RESOURCE_H)
	windres $(RESOURCE_RC) -o $@

# Compile C files
$(RELEASE_DIR)/arkanoid.o: arkanoid.c arkanoid.h $(RESOURCE_H)
	$(CC) $(CFLAGS) -c arkanoid.c -o $@

$(RELEASE_DIR)/levels.o: levels.c arkanoid.h
	$(CC) $(CFLAGS) -c levels.c -o $@

$(RELEASE_DIR)/powerups.o: powerups.c arkanoid.h
	$(CC) $(CFLAGS) -c powerups.c -o $@

$(RELEASE_DIR)/menu.o: menu.c arkanoid.h
	$(CC) $(CFLAGS) -c menu.c -o $@

$(RELEASE_DIR)/sounds.o: sounds.c arkanoid.h $(RESOURCE_H)
	$(CC) $(CFLAGS) -c sounds.c -o $@

$(RELEASE_DIR)/language.o: language.c arkanoid.h
	$(CC) $(CFLAGS) -c language.c -o $@

# Clean build files
clean:
	rm -f $(RELEASE_DIR)/*.o $(RELEASE_DIR)/*.exe
	rmdir $(RELEASE_DIR) 2>/dev/null || true

# Help information
help:
	@echo "Arkanoid Game Makefile"
	@echo "Usage:"
	@echo "  make        - Build the game (output in Release folder)"
	@echo "  make clean  - Remove Release folder and all build files"
	@echo ""
	@echo "Copyright (c) 2026 playtester"

.PHONY: all clean help