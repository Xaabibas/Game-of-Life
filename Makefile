CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11
LDLIBS = $(shell pkg-config --libs ncurses)

TARGET = game_life
SRC = game_life.c

.PHONY: all clean

all: $(TARGET)
	
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDLIBS)

clean:
	rm -f $(TARGET)
