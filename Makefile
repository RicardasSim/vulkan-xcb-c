CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -Wshadow
DEBUG_FLAGS   = -O0 -DDEBUG -g
RELEASE_FLAGS = -O3 -DNDEBUG
INCLUDEDIR=-I./include
LIBS = -lxcb -lm -ldl
OBJ = main.o
TARGET_PROGRAM = vulkanxcbc

all: CFLAGS += $(RELEASE_FLAGS)
all: $(TARGET_PROGRAM)

debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(TARGET_PROGRAM)

$(TARGET_PROGRAM): $(OBJ)
	$(CC) -o $(TARGET_PROGRAM) $(OBJ) $(LIBS)

main.o: main.c
	$(CC) $(CFLAGS) $(INCLUDEDIR) -c main.c -o main.o

clean:
	@echo Cleaning up...
	@rm -f *.o
	@rm -f $(TARGET_PROGRAM)
	@echo Done.
