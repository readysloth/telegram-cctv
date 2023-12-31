.PHONY: clean, mrproper

CC ?= gcc
override CFLAGS += -g -Wall -I include
override LDFLAGS += -lcurl -lv4l2

all: tg-cctv

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

tg-cctv: src/log.c src/tg.c src/tg-cctv.c src/cam.c
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

help: ## Prints help for targets with comments
	@cat $(MAKEFILE_LIST) | grep -E '^[a-zA-Z_-]+:.*?## .*$$' | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'

clean:
	rm -f src/*.o *.o tg-cctv

.DEFAULT_GOAL := all
