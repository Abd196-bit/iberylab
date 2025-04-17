CC = gcc
CFLAGS = -Wall -Wextra -g -I./include
TARGET = iberypp
SRCS = src/lexer.c src/parser.c src/vm.c src/main.c src/codegen.c src/class.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) 