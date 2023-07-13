.PHONY: clean, mrproper
CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lcurl

all: out

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

out: out.o
	$(CC) $(CFLAGS) -o $@ $+

help: ## Prints help for targets with comments
	@cat $(MAKEFILE_LIST) | grep -E '^[a-zA-Z_-]+:.*?## .*$$' | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'

clean:
	rm -f *.o core.*

mrproper: clean
	rm -f out

.DEFAULT_GOAL := all
