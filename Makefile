#
# Tony Givargis
# Copyright (C), 2023
# University of California, Irvine
#
# CS 238P - Operating Systems
# Makefile
#

CC     = gcc
CFLAGS = -ansi -pedantic -Wall -Wextra -Werror -Wfatal-errors -fpic -O3
LDLIBS = -lpthread
DEST   = cs238
SRCS  := $(wildcard *.c)
OBJS  := $(SRCS:.c=.o)

all: $(OBJS)
	@echo "[LN]" $(DEST)
	@$(CC) -o $(DEST) $(OBJS) $(LDLIBS)

%.o: %.c
	@echo "[CC]" $<
	@$(CC) $(CFLAGS) -c $<
	@$(CC) $(CFLAGS) -MM $< > $*.d

clean:
	@rm -f $(DEST) *.so *.o *.d *~ *#

-include $(OBJS:.o=.d)
