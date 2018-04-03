#######################################################################
##                     Copyright (C) 2017 wystan
##
##       filename: makefile
##    description:
##        created: 2017-07-06 11:42:13
##         author: wystan
##
#######################################################################
.PHONY: all clean

bin := a.out enc
src := $(wildcard *.c *.cpp)
obj := $(src:.c=.o)
obj := $(obj:.cpp=.o)
ld_flags := -lopus

all: $(bin)

a.out: main.o
	@gcc $^ -o $@ $(ld_flags)
	@strip $@
	@echo "[gen] "$@
enc: enc.o
	@gcc $^ -o $@ $(ld_flags)
	@strip $@
	@echo "[gen] "$@

%.o:%.c
	@echo "[ cc] "$@
	@gcc -c -Werror -Wall $< -o $@
%.o:%.cpp
	@echo "[cpp] "$@
	@g++ -c $< -o $@

clean:
	@echo "cleaning..."
	@rm -f *.o $(bin)
	@echo "done."

#######################################################################
