CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -Iinclude
TARGET = process_analyzer
SOURCES = src/main.c src/process.c src/security.c src/scheduler.c

.PHONY: all clean
all: $(TARGET)

$(TARGET): $(SOURCES) include/process.h include/security.h include/scheduler.h
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET) *.o src/*.o
