# $Id$

# GNU Compiler
CC = gcc

# Intel Compiler
#CC = icc
#CFLAGS = -openmp
#LDFLAGS = -openmp

SRCS = $(wildcard *.c)
OBJS = $(subst .c,.o,$(SRCS))
LDLIBS = -lGL -lGLU -lglut

yaps : $(OBJS) $(LDLIBS)
	$(CC) $(LDFLAGS) $^ -o $@ 

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) yaps
