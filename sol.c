#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>

#include "sol.h"
#include "schemes.h"

#ifdef KLONDIKE
#define NUM_PILES 7
#define MAX_HIDDEN 6 /*how many cards are turned over at most in a tableu pile*/
#define MAX_STOCK 24 /*how many cards can be in the stock at most (=@start)*/
#define NUM_DECKS 1
#elif defined SPIDER
#define MAX_HIDDEN 5
#define NUM_PILES 10
#define MAX_STOCK 50 /*how many cards can be dealt onto the piles*/
#define NUM_DECKS 2
#endif

#define get_suit(card) \
	((card-1) % NUM_SUITS)
#define get_rank(card) \
	((card-1) / NUM_SUITS)
#define get_color(card) \
	((get_suit(card) ^ get_suit(card)>>1) & 1)

struct playfield {
	//TODO: stock and waste are incompatible with undo{}
	card_t s[MAX_STOCK]; /* stock */
	int z; /* stock size */
	int w; /* waste; index into stock (const -1 in spider) */
#ifdef KLONDIKE
	card_t f[NUM_SUITS][MAX_HIDDEN+NUM_RANKS]; /* foundation (oversized to ease find_top()) */
	card_t t[NUM_PILES][MAX_HIDDEN+NUM_RANKS]; /* tableu piles */
#elif defined SPIDER
	card_t f[NUM_DECKS*NUM_COLORS][MAX_HIDDEN+NUM_RANKS]; //each completed set gets put on its own pile, so undo is possible
	card_t t[NUM_PILES][MAX_HIDDEN+NUM_RANKS];
#endif
	struct undo {
		int from; /* pile cards were taken from */
		int to; /* pile cards were moved to */
		int n; /* number of cards moved */
		struct undo* prev;
		struct undo* next;
	} u;
} f;
struct opts {
	const struct scheme* s;
} op;

#ifdef KLONDIKE
//declare action as array 10 of array 10 of pointer to function (int,int) returning int
// this stores a function pointer for every takeable action that is then called by execute()
//lines = from, cols = to
int (*action[10][10])(int,int) = {
	/* 0    1    2    3    4    5    6    7    8    9 */
/* 0 */	{ nop, f2t, f2t, f2t, f2t, f2t, f2t, f2t, nop, nop },
/* 1 */	{ t2f, nop, t2t, t2t, t2t, t2t, t2t, t2t, nop, nop },
/* 2 */	{ t2f, t2t, nop, t2t, t2t, t2t, t2t, t2t, nop, nop },
/* 3 */	{ t2f, t2t, t2t, nop, t2t, t2t, t2t, t2t, nop, nop },
/* 4 */	{ t2f, t2t, t2t, t2t, nop, t2t, t2t, t2t, nop, nop },
/* 5 */	{ t2f, t2t, t2t, t2t, t2t, nop, t2t, t2t, nop, nop },
/* 6 */	{ t2f, t2t, t2t, t2t, t2t, t2t, nop, t2t, nop, nop },
/* 7 */	{ t2f, t2t, t2t, t2t, t2t, t2t, t2t, nop, nop, nop },
/* 8 */	{ nop, nop, nop, nop, nop, nop, nop, nop, nop, s2w },
/* 9 */	{ w2f, w2t, w2t, w2t, w2t, w2t, w2t, w2t, w2s, nop },
};
#elif defined SPIDER
int (*action[11][10])(int,int) = {
	//piles are zero-indexed in spider; stk=stock
	/* 0    1    2    3    4    5    6    7    8    9 */
/* 0 */	{ nop, t2t, t2t, t2t, t2t, t2t, t2t, t2t, t2t, t2t },
/* 1 */	{ t2t, nop, t2t, t2t, t2t, t2t, t2t, t2t, t2t, t2t },
/* 2 */	{ t2t, t2t, nop, t2t, t2t, t2t, t2t, t2t, t2t, t2t },
/* 3 */	{ t2t, t2t, t2t, nop, t2t, t2t, t2t, t2t, t2t, t2t },
/* 4 */	{ t2t, t2t, t2t, t2t, nop, t2t, t2t, t2t, t2t, t2t },
/* 5 */	{ t2t, t2t, t2t, t2t, t2t, nop, t2t, t2t, t2t, t2t },
/* 6 */	{ t2t, t2t, t2t, t2t, t2t, t2t, nop, t2t, t2t, t2t },
/* 7 */	{ t2t, t2t, t2t, t2t, t2t, t2t, t2t, nop, t2t, t2t },
/* 8 */	{ t2t, t2t, t2t, t2t, t2t, t2t, t2t, t2t, nop, t2t },
/* 9 */	{ t2t, t2t, t2t, t2t, t2t, t2t, t2t, t2t, t2t, nop },
/*stk*/	{ s2t, s2t, s2t, s2t, s2t, s2t, s2t, s2t, s2t, s2t },
};
#endif

int main(int argc, char** argv) {
	//op.s = &unicode_small_mono;
	//op.s = &unicode_large_mono;
	op.s = &unicode_large_color;
	raw_mode(1); //TODO: alt.screen, etc.
	sol(); //TODO: restart, etc.
	raw_mode(0);
}

void sol(void) {
	deal();

	int from, to;
	print_table();
	while (!get_cmd(&from, &to)) {
		if (action[from][to](from,to)) {
			printf ("\033[?5h"); fflush (stdout);
			usleep (100000);
			printf ("\033[?5l"); fflush (stdout);
		}
		print_table();
	}
}

int find_top(card_t* pile) {
	int i;
	for(i=MAX_HIDDEN+NUM_RANKS-1; i>=0 && !pile[i]; i--);
	return i;
}
void turn_over(card_t* pile) {
	int top = find_top(pile);
	if (pile[top] < 0) pile[top] *= -1;
}
#ifdef KLONDIKE
card_t stack_take(void) { /*NOTE: assert(f.w >= 0) */
	card_t card = f.s[f.w];
	/* move stack one over, so there are no gaps in it: */
	for (int i = f.w; i < f.z-1; i++)
		f.s[i] = f.s[i+1];
	f.z--;
	f.w--; /* make previous card visible again */
	return card;
}
int t2f(int from, int to) { /* tableu to foundation */
	from--; //remove off-by-one
	int top_from = find_top(f.t[from]);
	to = get_suit(f.t[from][top_from]);
	int top_to   = find_top(f.f[to]);
	if ((top_to < 0 && get_rank(f.t[from][top_from]) == RANK_A)
	|| (get_rank(f.f[to][top_to]) == get_rank(f.t[from][top_from])-1)) {
		f.f[to][top_to+1] = f.t[from][top_from];
		f.t[from][top_from] = NO_CARD;
		turn_over(f.t[from]);
		return 0;
	} else return 1;
}
int w2f(int from, int to) { /* waste to foundation */
	if (f.w < 0) return 1;
	to = get_suit(f.s[f.w]);
	int top_to = find_top(f.f[to]);
	if ((top_to < 0 && get_rank(f.s[f.w]) == RANK_A)
	|| (get_rank(f.f[to][top_to]) == get_rank(f.s[f.w])-1)) {
		f.f[to][top_to+1] = stack_take();
		return 0;
	} else return 1;
	
}
int s2w(int from, int to) { /* stock to waste */
	if (f.z == 0) return 1;
	f.w++;
	if (f.w == f.z) f.w = -1;
	return 0;
}
int w2s(int from, int to) { /* waste to stock (undoes stock to waste) */
	if (f.z == 0) return 1;
	f.w--;
	if (f.w < -1) f.w = f.z-1;
	return 0;
}
int f2t(int from, int to) { /* foundation to tableu */
	//TODO: there are two possible cards one can take. choosing one isn't implemented yet!
	to--; //remove off-by-one
	int top_to = find_top(f.t[to]);
	from = get_color(f.t[to][top_to]);
	int top_from = find_top(f.f[from]);
	if ((get_color(f.t[to][top_to]) != get_color(f.f[from][top_from]))
	&& (get_rank(f.t[to][top_to]) == get_rank(f.f[from][top_from])+1)) {
		f.t[to][top_to+1] = stack_take(); //XXX: not stack_take!
		return 0;
	} else return 1;
}
int w2t(int from, int to) { //waste to tableu
	to--; //remove off-by-one
	int top_to = find_top(f.t[to]);
	if (((get_color(f.t[to][top_to]) != get_color(f.s[f.w]))
	   && (get_rank(f.t[to][top_to]) == get_rank(f.s[f.w])+1))
	|| (top_to < 0 && get_rank(f.s[f.w]) == RANK_K)) {
		f.t[to][top_to+1] = stack_take();
		return 0;
	} else return 1;
}
int t2t(int from, int to) {
	from--; to--; //remove off-by-one
	int top_to = find_top(f.t[to]);
	int top_from = find_top(f.t[from]);
	for (int i = top_from; i >=0; i--) {
		//TODO: check that we aren't moving facedown cards!
		if (((get_color(f.t[to][top_to]) != get_color(f.t[from][i]))
		   && (get_rank(f.t[to][top_to]) == get_rank(f.t[from][i])+1))
		|| (top_to < 0 && get_rank(f.t[from][i]) == RANK_K)) {
			/* move cards [i..top_from] to their destination */
			for (;i <= top_from; i++) {
				top_to++;
				f.t[to][top_to] = f.t[from][i];
				f.t[from][i] = NO_CARD;
			}
			turn_over(f.t[from]);
			return 0;
		}
	}
	return 1; /* no such move possible */
}
#elif defined SPIDER
int t2t(int from, int to) { //TODO: in dire need of cleanup
	from--; to--; //remove off-by-one
	(from < 0) && (from = 9); // '0' is tenth ([9]) pile
	(to < 0) && (to = 9); // ditto

	int top_from = find_top(f.t[from]);
	int top_to = find_top(f.t[to]);
	for (int i = top_from; i >= 0; i--) {
		if ((f.t[from][i+1] != NO_CARD) // if there is a card below (TODO: check maximum)
		    && (get_rank(f.t[from][i+1]) != get_rank(f.t[from][i])-1) //and cards not consecutive?
		   ) {
			break;
		}
		if ((f.t[from][i+1] != NO_CARD) // if there is a card below (TODO: check maximum)
		    && (get_suit(f.t[from][i+1]) != get_suit(f.t[from][i])) //and cards not same suit?
		   ) {
			break;
		}

		if(get_rank(f.t[from][i]) == get_rank(f.t[to][top_to])-1) { //TODO: to empty pile
			for (;i <= top_from; i++) {
				top_to++;
				f.t[to][top_to] = f.t[from][i];
				f.t[from][i] = NO_CARD;
			}
			turn_over(f.t[from]);
			//TODO: test if k..a complete; move to foundation if so
			return 0;
		}
	}

	return 1; /* no such move possible */
}
int s2t(int from, int to) {
	if (f.z <= 0) return 1; /* stack out of cards */
	for (int pile = 0; pile < NUM_PILES; pile++)
		if (f.t[pile][0] == NO_CARD) return 1; /*no piles may be empty*/
	for (int pile = 0; pile < NUM_PILES; pile++) {
		f.t[pile][find_top(f.t[pile])+1] = f.s[--f.z];
	}
	return 0;
}
#endif
int nop(int from, int to) { return 1; }

int get_cmd (int* from, int* to) {
	//returns 0 on success or an error code indicating game quit, new game,...
	//TODO: check validity
	char f, t;
	f = getchar();
#ifdef SPIDER
	if (f=='\n') {
		*from = 10;
		*to = 0;
		return CMD_MOVE;
	}
#endif
	switch (f) {
	case 'q': return CMD_QUIT;
	case 'r': return CMD_NEW;
	default: if (f < '0' || f > '9') return CMD_INVAL;
	}
	t =
#ifdef KLONDIKE
	    (f=='8')?'9':
#endif
	                  getchar();
	*from = f-'0';
	*to = t-'0';
	return CMD_MOVE;
}

void deal(void) {
	f = (const struct playfield){0}; /* clear playfield */
	card_t deck[DECK_SIZE*NUM_DECKS];
	int avail = DECK_SIZE*NUM_DECKS;
	for (int i = 0; i < DECK_SIZE*NUM_DECKS; i++) deck[i] = (i%DECK_SIZE)+1;
#ifdef SPIDER
	//if spider-mode easy/medium:
	for (int i = 0; i < DECK_SIZE*NUM_DECKS; i++) {
		int easy = 0;
		int medium = 1; //XXX
		if (easy||medium) deck[i] = 1+((deck[i]-1) | 2);
		if (easy        ) deck[i] = 1+((deck[i]-1) | 1);
		// the 1+ -1 dance gets rid of the offset created by NO_CARD
	}
#endif
	srandom (time(NULL));
	for (int i = DECK_SIZE*NUM_DECKS-1; i > 0; i--) { //fisher-yates
		int j = random() % (i+1);
		if (j-i) deck[i]^=deck[j],deck[j]^=deck[i],deck[i]^=deck[j];
	}

	/* deal cards: */
	for (int i = 0; i < NUM_PILES; i++) {
#ifdef KLONDIKE
		int closed = i; // tableu pile n has n closed cards, then 1 open
#elif defined SPIDER
		int closed = i<4?5:4; //tableu pile 1-4 have 5, 5-10 have 4 closed cards
#endif
		
		for (int j = 0; j < closed; j++) f.t[i][j] = -deck[--avail]; //face-down cards are negative
		f.t[i][closed] = deck[--avail]; //the face-up card
	}
	//rest of the cards to the stock (should be 50 for spider):
	for (f.z = 0; avail; f.z++) f.s[f.z] = deck[--avail];
	f.w = -1; /* @start: nothing on waste (no waste in spider -> const) */
}

void print_table(void) { //{{{
printf("\033[2J\033[H");//TEMP XXX -- use raw term mode
#ifdef KLONDIKE
	//print stock, waste and foundation:
	for (int line = 0; line < op.s->height; line++) {
		printf ("%s", ( /* stock */
			(f.w < f.z-1)?op.s->facedown
			:op.s->placeholder)[line]);
		printf ("%s", ( /* waste */
			((short)f.w >= 0)?op.s->card[f.s[f.w]] //TODO: sometimes segfaults because f.w == (int)((short)-1)
			:op.s->placeholder)[line]);
		printf ("%s", op.s->card[NO_CARD][line]); /* spacer */
		/* foundation: */
		for (int pile = 0; pile < NUM_SUITS; pile++) {
			for (int i = NUM_RANKS-1; i >=0; i--) {
				if (f.f[pile][i]) {
					printf ("%s", op.s->card[f.f[pile][i]][line]);
					goto next;
				}
			}
			printf ("%s", op.s->placeholder[line]);
next:;
		}
		printf("\n");
	}
	printf("\n");
#endif
	//print tableu piles:
	int row[NUM_PILES] = {0};
	int line[NUM_PILES]= {0};
	int line_had_card; // :|
int label[NUM_PILES] = {0};//XXX
	do {
		line_had_card = 0;
		for (int pile = 0; pile < NUM_PILES; pile++) {
			card_t card = f.t[pile][row[pile]];
			card_t next = f.t[pile][row[pile]+1];
			printf ("%s", (
				(card<0)?op.s->facedown
				:op.s->card[card]
				)[line[pile]]);

			if (++line[pile] >= (next?op.s->overlap:op.s->height)) {
				//TODO: allow extreme overlap on sequences
				line[pile]=0;
				row[pile]++;
			}
			if(!card && !label[pile]) label[pile] = 1, printf ("\b\b%d ", (pile+1) % 10);//XXX: print tableu labels
			line_had_card |= !!card;
		}
		printf ("\n");
	} while (line_had_card);
}//}}}

void append_undo (int n, int f, int t) {
	//check if we have to free redo buffer (.next)
	//malloc
	//update pointers
	*NULL;
}

void raw_mode(int enable) { //{{{
	static struct termios saved_term_mode;
	struct termios raw_term_mode;

	if (enable) {
		tcgetattr(STDIN_FILENO, &saved_term_mode);
		raw_term_mode = saved_term_mode;
		raw_term_mode.c_lflag &= ~(ICANON | ECHO);
		raw_term_mode.c_cc[VMIN] = 1 ;
		raw_term_mode.c_cc[VTIME] = 0;
		tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_term_mode);
	} else {
		tcsetattr(STDIN_FILENO, TCSAFLUSH, &saved_term_mode);
	}
} //}}}

//vim: foldmethod=marker
