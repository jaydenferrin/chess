CC = gcc
CFLAGS = -Wall -Wextra
SRCDIR = ./src
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJDIR = ./obj
OBJS = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
DEPS = $(OBJS:%.o:%.d)
INCDIR = ./include
INCS = $(foreach DIR, $(INCDIR), -I$(DIR))
BIN = chess

.PHONY: all clean debug

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJS)

-include $(DEPS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) $(INCS) -MMD -c $< -o $@

$(OBJDIR):
	mkdir $@

debug: CFLAGS += -ggdb
debug: all

clean:
	rm -f $(BIN) $(OBJDIR)/*
