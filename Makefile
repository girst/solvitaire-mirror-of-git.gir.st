.PHONY: all clean

CFLAGS := -Wall -Wextra -pedantic -std=c99 -g3 -Wno-unused-parameter

all: sol

#canfield:
sol: sol.c sol.h schemes.h
	$(CC) $(CFLAGS) -DKLONDIKE $< -o $@

spider: sol.c sol.h schemes.h
	$(CC) $(CFLAGS) -DSPIDER $< -o $@

clean:
	rm -f sol spider

getfuns:
	grep -o '^\w.* \w.*(.*)[^/]*{' sol.c|sed 's/ *{$/;/'
