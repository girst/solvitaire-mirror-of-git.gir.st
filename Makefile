.PHONY: all clean

CFLAGS := -Wall -Wextra -pedantic -std=c99 -g3 #-Wno-unused-parameter

all: sol spider

#canfield:
sol: sol.c sol.h schemes.h
	$(CC) $(CFLAGS) -DKLONDIKE $< -o $@

spider: sol.c sol.h schemes.h
	$(CC) $(CFLAGS) -DSPIDER $< -o $@

clean:
	rm -f sol spider

.PHONY: getfuns test
getfuns: sol.c
	@grep -o '^\w.* \w.*(.*)[^/]*{\|^#if.*\|^#e[ln].*' $<|sed 's/ *{$$/;/' \
	| perl -0777 -pe 's{#if[^\n]*\n(#elif[^\n]*\n)?#endif[^\n]*\n}{}igs'

test:
	grep -n --color=always 'TODO\|XXX\|\/xxx-slashes-xxx\/[^:]' *
