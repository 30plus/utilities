CFLAGS += -DDEBUG -Wall -Wunused

splash:	splash.c libfb.c
	$(CC) -o $@ $^ -lm -lpng $(CFLAGS)

showpng: showpng.c libfb.c
	$(CC) -o $@ $^ -lm -lpng $(CFLAGS)

.PHONY: clean showpng
clean:
	@rm -f splash
