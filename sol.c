#define _DEFAULT_SOURCE
#include <poll.h>
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
#define PILE_SIZE MAX_HIDDEN+NUM_RANKS
#elif defined SPIDER
#define MAX_HIDDEN 5
#define NUM_PILES 10
#define MAX_STOCK 50 /*how many cards can be dealt onto the piles*/
#define NUM_DECKS 2
#define PILE_SIZE DECK_SIZE*NUM_DECKS /* no maximum stack size in spider :/ */
#endif

#define get_suit(card) \
	((card-1) % NUM_SUITS)
#define get_rank(card) \
	((card-1) / NUM_SUITS)
#define get_color(card) \
	((get_suit(card) ^ get_suit(card)>>1) & 1)

struct playfield {
	card_t s[MAX_STOCK]; /* stock */
	int z; /* stock size */
	int w; /* waste; index into stock (const -1 in spider) */
	card_t f[NUM_DECKS*NUM_SUITS][PILE_SIZE]; /* foundation */
	card_t t[NUM_PILES][PILE_SIZE]; /* tableu piles */
	struct undo {
		int f; /* pile cards were taken from (overloaded:128+n=stock) */
		int t; /* pile cards were moved to */
		int n; /* number of cards moved */
		struct undo* prev;
		struct undo* next;
	}* u;
} f;
struct opts {
#ifdef SPIDER
	int m; /* difficulty mode */
#endif
	const struct scheme* s;
} op;

// action table {{{
/* stores a function pointer for every takeable action; called by game loop */
int (*action[NUM_PLACES][10])(int,int) = {
#ifdef KLONDIKE
	/* 1    2    3    4    5    6    7   stk  wst  fnd*/
/* 1 */	{ t2f, t2t, t2t, t2t, t2t, t2t, t2t, nop, nop, t2f },
/* 2 */	{ t2t, t2f, t2t, t2t, t2t, t2t, t2t, nop, nop, t2f },
/* 3 */	{ t2t, t2t, t2f, t2t, t2t, t2t, t2t, nop, nop, t2f },
/* 4 */	{ t2t, t2t, t2t, t2f, t2t, t2t, t2t, nop, nop, t2f },
/* 5 */	{ t2t, t2t, t2t, t2t, t2f, t2t, t2t, nop, nop, t2f },
/* 6 */	{ t2t, t2t, t2t, t2t, t2t, t2f, t2t, nop, nop, t2f },
/* 7 */	{ t2t, t2t, t2t, t2t, t2t, t2t, t2f, nop, nop, t2f },
/*stk*/	{ nop, nop, nop, nop, nop, nop, nop, nop, s2w, nop },
/*wst*/	{ w2t, w2t, w2t, w2t, w2t, w2t, w2t, w2s, w2f, w2f },
/*fnd*/	{ f2t, f2t, f2t, f2t, f2t, f2t, f2t, nop, nop, nop },
#elif defined SPIDER
	/* 1    2    3    4    5    6    7    8    9    10*/
/* 1 */	{ nop, t2t, t2t, t2t, t2t, t2t, t2t, t2t, t2t, t2t },
/* 2 */	{ t2t, nop, t2t, t2t, t2t, t2t, t2t, t2t, t2t, t2t },
/* 3 */	{ t2t, t2t, nop, t2t, t2t, t2t, t2t, t2t, t2t, t2t },
/* 4 */	{ t2t, t2t, t2t, nop, t2t, t2t, t2t, t2t, t2t, t2t },
/* 5 */	{ t2t, t2t, t2t, t2t, nop, t2t, t2t, t2t, t2t, t2t },
/* 6 */	{ t2t, t2t, t2t, t2t, t2t, nop, t2t, t2t, t2t, t2t },
/* 7 */	{ t2t, t2t, t2t, t2t, t2t, t2t, nop, t2t, t2t, t2t },
/* 8 */	{ t2t, t2t, t2t, t2t, t2t, t2t, t2t, nop, t2t, t2t },
/* 9 */	{ t2t, t2t, t2t, t2t, t2t, t2t, t2t, t2t, nop, t2t },
/*10 */	{ t2t, t2t, t2t, t2t, t2t, t2t, t2t, t2t, t2t, nop },
/*stk*/	{ s2t, s2t, s2t, s2t, s2t, s2t, s2t, s2t, s2t, s2t },
#endif
};
// }}}

int main(int argc, char** argv) {
	(void) argc;(void) argv;
	op.s = &unicode_large_color;
#ifdef SPIDER
	op.m = MEDIUM; //TODO: make configurable
	op.m = EASY;
#endif
	screen_setup(1);
	sol(); //TODO: restart, etc.
	screen_setup(0);
}

void sol(void) {
	deal();

	int from, to;
	print_table(NO_HI);
	for(;;) {
		switch (get_cmd(&from, &to)) {
		case CMD_MOVE:
			switch (action[from][to](from,to)) {
			case OK:  break;
			case ERR: visbell(); break;
			case WON: 
				print_table(NO_HI);
				win_anim();
				getchar(); /* consume char left by win_anim() */
				return;
			}
			break;
		case CMD_QUIT: return;
		}
		print_table(NO_HI);
	}
}

int find_top(card_t* pile) {
	int i;
	for(i=PILE_SIZE-1; i>=0 && !pile[i]; i--);
	return i;
}
void turn_over(card_t* pile) {
	int top = find_top(pile);
	if (pile[top] < 0) pile[top] *= -1;
}
int check_won(void) {
	for (int pile = 0; pile < NUM_DECKS*NUM_SUITS; pile++)
		if (f.f[pile][NUM_RANKS-1] == NO_CARD) return 0;

	return 1;
}
void win_anim(void) {
	printf ("\033[?25l"); /* hide cursor */
	for (;;) {
		/* set cursor to random location */
		int row = 1+random()%(24-op.s->width);
		int col = 1+random()%(80-op.s->height);

		/* draw random card */
		int face = 1 + random() % 52;
		for (int l = 0; l < op.s->height; l++) {
			printf ("\033[%d;%dH", row+l, col);
			printf ("%s", op.s->card[face][l]);
		}
		fflush (stdout);

		/* exit on keypress */
		struct pollfd p = {STDIN_FILENO, POLLIN, 0};
		if (poll (&p, 1, 80)) goto fin;
	}
fin:
	printf ("\033[?25h"); /* show cursor */
	return;
}
// takeable actions {{{
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
	(void) to; //don't need
	int top_from = find_top(f.t[from]);
	to = get_suit(f.t[from][top_from]);
	int top_to   = find_top(f.f[to]);
	if ((top_to < 0 && get_rank(f.t[from][top_from]) == RANK_A)
	|| (top_to >= 0 && get_rank(f.f[to][top_to]) == get_rank(f.t[from][top_from])-1)) {
		f.f[to][top_to+1] = f.t[from][top_from];
		f.t[from][top_from] = NO_CARD;
		turn_over(f.t[from]);
		if (check_won()) return WON;
		return OK;
	} else return ERR;
}
int w2f(int from, int to) { /* waste to foundation */
	(void) from; (void) to; //don't need
	if (f.w < 0) return ERR;
	to = get_suit(f.s[f.w]);
	int top_to = find_top(f.f[to]);
	if ((top_to < 0 && get_rank(f.s[f.w]) == RANK_A)
	|| (top_to >= 0 && get_rank(f.f[to][top_to]) == get_rank(f.s[f.w])-1)) {
		f.f[to][top_to+1] = stack_take();
		if (check_won()) return WON;
		return OK;
	} else return ERR;
	
}
int s2w(int from, int to) { /* stock to waste */
	(void) from; (void) to; //don't need
	if (f.z == 0) return ERR;
	f.w++;
	if (f.w == f.z) f.w = -1;
	return OK;
}
int w2s(int from, int to) { /* waste to stock (undoes stock to waste) */
	(void) from; (void) to; //don't need
	if (f.z == 0) return ERR;
	f.w--;
	if (f.w < -1) f.w = f.z-1;
	return OK;
}
int f2t(int from, int to) { /* foundation to tableu */
	int top_to = find_top(f.t[to]);
	printf ("take from (1-4): "); fflush (stdout);
	from = getchar() - '1';
	if (from < 0 || from > 3) return ERR;
	int top_from = find_top(f.f[from]);
	
	if ((get_color(f.t[to][top_to]) != get_color(f.f[from][top_from]))
	&& (get_rank(f.t[to][top_to]) == get_rank(f.f[from][top_from])+1)) {
		f.t[to][top_to+1] = f.f[from][top_from];
		f.f[from][top_from] = NO_CARD;
		return OK;
	} else return ERR;
}
int w2t(int from, int to) { /* waste to tableu */
	(void) from; //don't need
	int top_to = find_top(f.t[to]);
	if (((get_color(f.t[to][top_to]) != get_color(f.s[f.w]))
	   && (get_rank(f.t[to][top_to]) == get_rank(f.s[f.w])+1))
	|| (top_to < 0 && get_rank(f.s[f.w]) == RANK_K)) {
		f.t[to][top_to+1] = stack_take();
		return OK;
	} else return ERR;
}
int t2t(int from, int to) { /* tableu to tableu */
	int top_to = find_top(f.t[to]);
	int top_from = find_top(f.t[from]);
	for (int i = top_from; i >=0; i--) {
		if (((get_color(f.t[to][top_to]) != get_color(f.t[from][i]))
		   && (get_rank(f.t[to][top_to]) == get_rank(f.t[from][i])+1)
		   && f.t[from][i] > NO_CARD) /* card face up? */
		|| (top_to < 0 && get_rank(f.t[from][i]) == RANK_K)) {
			/* move cards [i..top_from] to their destination */
			for (;i <= top_from; i++) {
				top_to++;
				f.t[to][top_to] = f.t[from][i];
				f.t[from][i] = NO_CARD;
			}
			turn_over(f.t[from]);
			return OK;
		}
	}
	return ERR; /* no such move possible */
}
#elif defined SPIDER
void remove_if_complete (card_t* pile) { //TODO: cleanup
	static int foundation = 0; /* where to put pile onto (1 set per stack)*/
	/* test if K...A complete; move to foundation if so */
	int top_from = find_top(pile);
	if (get_rank(pile[top_from]) != RANK_A) return;
	for (int i = top_from; i>=0; i--) {
		if ((i+1 < PILE_SIZE && pile[i+1] != NO_CARD) // card below or last? XXX: copied from t2t()--make function
		    && (get_rank(pile[i+1]) != get_rank(pile[i])-1) //cards not consecutive?
		   ) {
			return;
		}
		if ((i+1 < PILE_SIZE && pile[i+1] != NO_CARD) // card below  or last?
		    && (get_suit(pile[i+1]) != get_suit(pile[i])) //cards not same suit?
		   ) {
			return;
		}
		if (i+RANK_K == top_from
		    && get_rank(pile[top_from-RANK_K]) == RANK_K) { //ace to king ok, remove it
			for (int i = top_from, j = 0; i > top_from-NUM_RANKS; i--, j++) {
				f.f[foundation][j] = pile[i];
				pile[i] = NO_CARD;
			}
			foundation++;
			turn_over(pile);
			return;
		}
	}
}
int t2t(int from, int to) { //TODO: in dire need of cleanup
	//TODO: segfaulted once on large column
	//TODO: sometimes moving doesn't work (ERR when it should be OK) XXX

	int top_from = find_top(f.t[from]);
	int top_to = find_top(f.t[to]);
	int empty_to = -1; //awful, nondescriptive name :/
	if (top_to < 0) { /* empty pile? */
		printf ("\rup to (a23456789xjqk): "); //TODO: automatically do it if only 1 card movable
		empty_to = getchar();
		switch (empty_to) {
		case 'a': case 'A': empty_to = RANK_A; break;
		case '0': /* fallthrough */
		case 'x': case 'X': empty_to = RANK_X; break;
		case 'j': case 'J': empty_to = RANK_J; break;
		case 'q': case 'Q': empty_to = RANK_Q; break;
		case 'k': case 'K': empty_to = RANK_K; break;
		default: empty_to -= '1';
		}
		if (empty_to < RANK_A || empty_to > RANK_K) return ERR;
	}
	for (int i = top_from; i >= 0; i--) {
		if ((i+1 < PILE_SIZE && f.t[from][i+1] != NO_CARD) // card below or last?
		    && (get_rank(f.t[from][i+1]) != get_rank(f.t[from][i])-1) //cards not consecutive?
		   ) {
			break;
		}
		if ((i+1 < PILE_SIZE && f.t[from][i+1] != NO_CARD) // card below  or last?
		    && (get_suit(f.t[from][i+1]) != get_suit(f.t[from][i])) //cards not same suit?
		   ) {
			break;
		}

		if ((get_rank(f.t[from][i]) == get_rank(f.t[to][top_to])-1) // consecutive?
		|| (empty_to >= RANK_A && get_rank(f.t[from][i]) == empty_to)) { //to empty pile and rank ok?
			for (;i <= top_from; i++) {
				top_to++;
				f.t[to][top_to] = f.t[from][i];
				f.t[from][i] = NO_CARD;
			}
			turn_over(f.t[from]);
			remove_if_complete (f.t[to]);
			if (check_won()) return WON;
			return OK;
		}
	}

	return ERR; /* no such move possible */
}
int s2t(int from, int to) { //TODO: check remove, won
	(void) from; (void) to; //don't need
	if (f.z <= 0) return ERR; /* stack out of cards */
	for (int pile = 0; pile < NUM_PILES; pile++)
		if (f.t[pile][0]==NO_CARD) return ERR; /*no piles may be empty*/
	for (int pile = 0; pile < NUM_PILES; pile++) {
		f.t[pile][find_top(f.t[pile])+1] = f.s[--f.z];
	}
	return OK;
}
#endif
int nop(int from, int to) { (void)from;(void)to; return ERR; }
// }}}

int get_cmd (int* from, int* to) {
	//returns 0 on success or an error code indicating game quit, new game,...
	//TODO: escape sequences (mouse, cursor keys)
	int f, t;
	f = getchar();

	switch (f) {
	case '1': *from = TAB_1; break;
	case '2': *from = TAB_2; break;
	case '3': *from = TAB_3; break;
	case '4': *from = TAB_4; break;
	case '5': *from = TAB_5; break;
	case '6': *from = TAB_6; break;
	case '7': *from = TAB_7; break;
#ifdef SPIDER
	case '8': *from = TAB_8; break;
	case '9': *from = TAB_9; break;
	case '0': *from = TAB_10; break;
#elif defined KLONDIKE
	case '9': *from = WASTE; break;
	case '0': *from = FOUNDATION; break;
	case '8': /* fallthrough */
#endif
	case '\n': /* shortcut for dealing from stock */
		*from = STOCK;
		*to = WASTE;
		return CMD_MOVE;
	case 'q': return CMD_QUIT;
	case 'r': return CMD_NEW;  //TODO
	case 'h': return CMD_HINT; //TODO
	case '?': return CMD_HELP; //TODO
	case '/': return CMD_FIND; //TODO: highlight card of given rank (even non-movable)
	case '\033': return CMD_INVAL; //TODO: cntlseq
	default: return CMD_INVAL;
	}
	print_table(*from);

	t = getchar();
	if (t < '0' || t > '9') return CMD_INVAL;
	if (t == '0')
#ifdef KLONDIKE
		*to = FOUNDATION;
#elif defined SPIDER
		*to = TAB_10;
#endif
	else
		*to = t-'1';
	return CMD_MOVE;
}

void deal(void) {
	f = (const struct playfield){0}; /* clear playfield */
	card_t deck[DECK_SIZE*NUM_DECKS];
	int avail = DECK_SIZE*NUM_DECKS;
	for (int i = 0; i < DECK_SIZE*NUM_DECKS; i++) deck[i] = (i%DECK_SIZE)+1;
#ifdef SPIDER
	if (op.m != NORMAL) for (int i = 0; i < DECK_SIZE*NUM_DECKS; i++) {
		if (op.m == MEDIUM) deck[i] = 1+((deck[i]-1) | 2);
		if (op.m == EASY)   deck[i] = 1+((deck[i]-1) | 2 | 1);
		/* the 1+ -1 dance gets rid of the offset created by NO_CARD */
	}
#endif
	srandom (time(NULL));
/*XXX*/	long seed = time(NULL);
/*XXX*/	srandom (seed);
	for (int i = DECK_SIZE*NUM_DECKS-1; i > 0; i--) { //fisher-yates
		int j = random() % (i+1);
		if (j-i) deck[i]^=deck[j],deck[j]^=deck[i],deck[i]^=deck[j];
	}
///////////////////////////////////////////////XXX
//sometimes we see duplicate cards. this tries to catch that
int count[_NUM_CARDS_internal] = {0};
for (int i = 0; i < DECK_SIZE*NUM_DECKS; i++)
	count[deck[i]]++;
for (int i = 0; i < _NUM_CARDS_internal; i++){ //0 is NO_CARD
#ifdef SPIDER
	int x = op.m==MEDIUM?2:op.m==EASY?4:1;
#else
	int x = 1;
#endif
	if (deck[i] == NO_CARD) continue;
	if (count[deck[i]] != NUM_DECKS*x) {
		screen_setup(0);
		printf ("found duplicate card with seed %lx!\n", seed);
		for (int i = 1; i < _NUM_CARDS_internal; i++)
			printf ("%3d of %2d\n", count[deck[i]], deck[i]);
		exit(1);
	}
}
///////////////////////////////////////////////XXX

	/* deal cards: */
	for (int i = 0; i < NUM_PILES; i++) {
#ifdef KLONDIKE
		int closed = i; /* pile n has n closed cards, then 1 open */
#elif defined SPIDER
		int closed = i<4?5:4; /* pile 1-4 have 5, 5-10 have 4 closed */
#endif
		/* face down cards are negated: */
		for (int j = 0; j < closed; j++) f.t[i][j] = -deck[--avail];
		f.t[i][closed] = deck[--avail]; /* the face-up card */
	}
	/* rest of the cards to the stock; NOTE: assert(avail==50) for spider */
	for (f.z = 0; avail; f.z++) f.s[f.z] = deck[--avail];
	f.w = -1; /* @start: nothing on waste (no waste in spider -> const) */
}

int is_movable(card_t* pile, int n) { //TODO cleanup, code deduplication, needs entry in sol.h
#ifdef KLONDIKE
	return(pile[n] > NO_CARD); /*non-movable cards don't exist in klondike*/
#elif defined SPIDER
	int top = find_top(pile);
	for (int i = top; i; i--) {
		if (pile[i] <= NO_CARD) return 0; //card face down?
		if ((i+1 < PILE_SIZE && pile[i+1] != NO_CARD) // card below or last? //COPIED FROM t2t
		    && (get_rank(pile[i+1]) != get_rank(pile[i])-1) //cards not consecutive?
		   ) {
			return 0;
		}
		if ((i+1 < PILE_SIZE && pile[i+1] != NO_CARD) // card below  or last?
		    && (get_suit(pile[i+1]) != get_suit(pile[i])) //cards not same suit?
		   ) {
			return 0;
		}
		if (i == n) return 1; //card reached, must be movable
	}
	return 0;
#endif
}
#define print_hi(test, str) /* for highlighting during get_cmd() */ \
	printf ("%s%s%s", (test)?"\033[7m":"", str, (test)?"\033[27m":"")
void print_table(int highlight) { //{{{
	printf("\033[2J\033[H"); /* clear screen, reset cursor */
#ifdef KLONDIKE
	/* print stock, waste and foundation: */
	for (int line = 0; line < op.s->height; line++) {
		/* stock: */
		print_hi (highlight == STOCK, (
			(f.w < f.z-1)?op.s->facedown
			:op.s->placeholder)[line]);
		/* waste: */
		print_hi (highlight == WASTE, (
			/* NOTE: cast, because f.w sometimes is (short)-1 !? */
			((short)f.w >= 0)?op.s->card[f.s[f.w]]
			:op.s->placeholder)[line]);
		printf ("%s", op.s->card[NO_CARD][line]); /* spacer */
		/* foundation: */
		for (int pile = 0; pile < NUM_SUITS; pile++) {
			int card = find_top(f.f[pile]);
			print_hi (highlight == FOUNDATION,
				(card < 0)?op.s->placeholder[line]
				:op.s->card[f.f[pile][card]][line]);
		}
		printf("\n");
	}
	printf("\n");
#endif
	/* print tableu piles: */
	int row[NUM_PILES] = {0};
	int line[NUM_PILES]= {0};
	int label[NUM_PILES]={0};// :|
	int line_had_card; // :|
	do {
		line_had_card = 0;
		for (int pile = 0; pile < NUM_PILES; pile++) {
			card_t card = f.t[pile][row[pile]];
			card_t next = f.t[pile][row[pile]+1];
			int movable = is_movable(f.t[pile], row[pile]);

			if (movable) printf ("\033[1m"); /* bold */ //TODO: interferes with grey color!
			print_hi (highlight == pile && movable, (
				(card<0)?op.s->facedown
				:op.s->card[card]
				)[line[pile]]);
			printf ("\033[22m"); /* normal intensity (no bold) */

			if (++line[pile] >= (next?op.s->overlap:op.s->height) //normal overlap
#if 0 //XXX
			|| (line[pile] >= 1 &&
			    f.t[pile][row[pile]] < 0 &&
			    f.t[pile][row[pile]+1] <0) //extreme overlap on closed
			|| (0) //extreme overlap on sequence TODO
#endif
			) {
				line[pile]=0;
				row[pile]++;
			}
			if(!card && !label[pile]) { /* tableu labels: */
				label[pile] = 1;
				printf ("\b\b%d ", (pile+1) % 10); //XXX: hack
			}
			line_had_card |= !!card;
		}
		printf ("\n");
	} while (line_had_card);
}//}}}

void visbell (void) {
	printf ("\033[?5h"); fflush (stdout);
	usleep (100000);
	printf ("\033[?5l"); fflush (stdout);
}

void append_undo (int n, int f, int t) {
	(void)n;(void)f;(void)t;
	//check if we have to free redo buffer (.next)
	//malloc
	//update pointers
	//TODO: undo; needs operations to be written by x2y()
}

void screen_setup (int enable) {
	if (enable) {
		raw_mode(1);
		printf ("\033[s\033[?47h"); /* save cursor, alternate screen */
		printf ("\033[H\033[J"); /* reset cursor, clear screen */
		//TODO//printf ("\033[?1000h\033[?25l"); /* enable mouse, hide cursor */
	} else {
		//TODO//printf ("\033[?9l\033[?25h"); /* disable mouse, show cursor */
		printf ("\033[?47l\033[u"); /* primary screen, restore cursor */
		raw_mode(0);
	}
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
