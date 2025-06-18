CC = gcc
CFLAGS = -Wall -Wextra -O0 -std=c99
LDFLAGS = -lm

# 운영체제별 설정
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    LDFLAGS += -lrt
endif
ifeq ($(UNAME_S),Darwin)
    LDFLAGS += 
endif

TARGET = speed_math_game
SOURCE = console.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE) $(LDFLAGS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

server:
	python3 game_server.py

.PHONY: all clean run server