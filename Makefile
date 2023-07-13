.PHONY: clean, mrproper
CC = gcc
CFLAGS = -g -Wall -I include
LDFLAGS = -lcurl

all: tg-cctv

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

tg-cctv: src/log.c src/tg.c src/tg-cctv.c
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

help: ## Prints help for targets with comments
	@cat $(MAKEFILE_LIST) | grep -E '^[a-zA-Z_-]+:.*?## .*$$' | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'

clean:
	rm -f *.o core.*

mrproper: clean
	rm -f tg-cctv

.DEFAULT_GOAL := all
