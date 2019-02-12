#define _DEFAULT_SOURCE /* for getopt, sigaction, usleep */
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>

#include "sol.h"
#include "schemes.h"

struct playfield f;
struct opts op;

// action table {{{
/* stores a function pointer for every takeable action; called by game loop */
int (*action[NUM_PLACES][10])(int,int,int) = {
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
/* 1 */	{ t2f, t2t, t2t, t2t, t2t, t2t, t2t, t2t, t2t, t2t },
/* 2 */	{ t2t, t2f, t2t, t2t, t2t, t2t, t2t, t2t, t2t, t2t },
/* 3 */	{ t2t, t2t, t2f, t2t, t2t, t2t, t2t, t2t, t2t, t2t },
/* 4 */	{ t2t, t2t, t2t, t2f, t2t, t2t, t2t, t2t, t2t, t2t },
/* 5 */	{ t2t, t2t, t2t, t2t, t2f, t2t, t2t, t2t, t2t, t2t },
/* 6 */	{ t2t, t2t, t2t, t2t, t2t, t2f, t2t, t2t, t2t, t2t },
/* 7 */	{ t2t, t2t, t2t, t2t, t2t, t2t, t2f, t2t, t2t, t2t },
/* 8 */	{ t2t, t2t, t2t, t2t, t2t, t2t, t2t, t2f, t2t, t2t },
/* 9 */	{ t2t, t2t, t2t, t2t, t2t, t2t, t2t, t2t, t2f, t2t },
/*10 */	{ t2t, t2t, t2t, t2t, t2t, t2t, t2t, t2t, t2t, t2f },
/*stk*/	{ s2t, s2t, s2t, s2t, s2t, s2t, s2t, s2t, s2t, s2t },
#endif
};
// }}}

// argv parsing, game loops, cleanup {{{
int main(int argc, char** argv) {
	/* opinionated defaults: */
	op.s = &unicode_large_color;
#ifdef SPIDER
	op.m = MEDIUM;
#endif

	int optget;
	opterr = 0; /* don't print message on unrecognized option */
	while ((optget = getopt (argc, argv, "+:hs:vbcm")) != -1) {
		switch (optget) {
#ifdef SPIDER
		case 's': /* number of suits */
			switch (optarg[0]) {
			case '1': op.m = EASY; break;
			case '2': op.m = MEDIUM; break;
			case '4': op.m = NORMAL; break;
			default: goto error;
			} break;
#endif
		case 'b': op.s = &unicode_large_mono; break;
		case 'c': op.s = &unicode_large_color; break;
		case 'm': op.s = &unicode_small_mono; break; /* "mini" */
		case 'h': default: goto error;
		error:
			fprintf (stderr, SHORTHELP LONGHELP KEYHELP, argv[0]);
			return optget != 'h';
		}
	}

	signal_setup();
	atexit (*quit);

	signal_handler(SIGWINCH); /* initialize window size */

newgame:
	screen_setup(1);

	switch(sol()) {
	case GAME_NEW: goto newgame;
	case GAME_WON:
		print_table(NO_HI, NO_HI);
		win_anim();
		if (getch(NULL)=='q') return 0;
		goto newgame;
	case GAME_QUIT: return 0;
	}
}

int sol(void) {
	int ret;
	long seed = time(NULL);
restart:
	free_undo(f.u);
	deal(seed);

	int from, to, opt;
	for(;;) {
		switch (get_cmd(&from, &to, &opt)) {
		case CMD_MOVE:
			ret = action[from][to](from,to,opt);
			if (ret == ERR) /* try again with from/to swapped: */
				ret = action[to][from](to,from,opt);
			switch (ret) {
			case OK:  break;
			case ERR: visbell(); break;
			case WON: return GAME_WON;
			}
			break;
		case CMD_JOIN:
			switch (join(to)) {
			case OK:  break;
			case ERR: visbell(); break;
			case WON: return GAME_WON;
			}
			break;
		case CMD_HINT:  break;//TODO: show a possible (and sensible) move. if possible, involve active cursor
		case CMD_UNDO:  undo_pop(f.u); break;
		case CMD_INVAL: visbell(); break;
		case CMD_NEW:   return GAME_NEW;
		case CMD_AGAIN: goto restart;
		case CMD_QUIT:  return GAME_QUIT;
		case CMD_HELP: printf (KEYHELP "\nPress any key to continue."); getch(NULL);break;
		}
	}
}

void quit(void) {
	screen_setup(0);
	free_undo(f.u);
}
//}}}

// card games helper functions {{{
#define get_suit(card) \
	((card-1) % NUM_SUITS)
#define get_rank(card) \
	((card-1) / NUM_SUITS)
#define get_color(card) \
	((get_suit(card) ^ get_suit(card)>>1) & 1)

#define is_tableu(where) (where <= TAB_MAX)

int find_top(card_t* pile) {
	int i;
	for(i=PILE_SIZE-1; i>=0 && !pile[i]; i--);
	return i;
}
int first_movable(card_t* pile) {
	int i = 0;
	for (;pile[i] && !is_movable(pile, i); i++);
	return i;
}
int turn_over(card_t* pile) {
	int top = find_top(pile);
	if (pile[top] < 0) {
		pile[top] *= -1;
		return 1;
	} else return 0;
}
int check_won(void) {
	for (int pile = 0; pile < NUM_DECKS*NUM_SUITS; pile++)
		if (f.f[pile][NUM_RANKS-1] == NO_CARD) return 0;

	return 1;
}
int rank_next (card_t a, card_t b) {
	return get_rank(a) == get_rank(b)-1;
}
int is_consecutive (card_t* pile, int pos) {
	if (pos+1 >= PILE_SIZE) return 1; /* card is last */
	if (pile[pos+1] == NO_CARD) return 1; /* card is first */

#ifdef KLONDIKE
	/* ranks consecutive? */
	if (!rank_next(pile[pos+1], pile[pos])) return 0;
	/* color opposite? */
	if (get_color(pile[pos+1]) == get_color(pile[pos])) return 0;
#elif defined SPIDER
	/* ranks consecutive? */
	if (!rank_next(pile[pos+1], pile[pos])) return 0;
	/* same suit? */
	if (get_suit(pile[pos+1]) != get_suit(pile[pos])) return 0;
#endif

	return 1;
}

int is_movable(card_t* pile, int n) {
#ifdef KLONDIKE
	return(pile[n] > NO_CARD); /*non-movable cards don't exist in klondike*/
#elif defined SPIDER
	int top = find_top(pile);
	for (int i = top; i >= 0; i--) {
		if (pile[i] <= NO_CARD) return 0; /*no card or card face down?*/
		if (!is_consecutive(pile, i)) return 0;
		if (i == n) return 1; /* card reached, must be movable */
	}
	return 0;
#endif
}
//}}}

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
int t2f(int from, int to, int opt) { /* tableu to foundation */
	(void) to; (void) opt; /* don't need */
	int top_from = find_top(f.t[from]);
	to = get_suit(f.t[from][top_from]);
	int top_to   = find_top(f.f[to]);
	if ((top_to < 0 && get_rank(f.t[from][top_from]) == RANK_A)
	|| (top_to >= 0 && rank_next(f.f[to][top_to],f.t[from][top_from]))) {
		f.f[to][top_to+1] = f.t[from][top_from];
		f.t[from][top_from] = NO_CARD;
		undo_push(from, FOUNDATION, to,
		turn_over(f.t[from]));
		if (check_won()) return WON;
		return OK;
	} else return ERR;
}
int w2f(int from, int to, int opt) { /* waste to foundation */
	(void) from; (void) to; (void) opt; /* don't need */
	if (f.w < 0) return ERR;
	to = get_suit(f.s[f.w]);
	int top_to = find_top(f.f[to]);
	if ((top_to < 0 && get_rank(f.s[f.w]) == RANK_A)
	|| (top_to >= 0 && rank_next(f.f[to][top_to], f.s[f.w]))) {
		undo_push(WASTE, FOUNDATION, f.w | to<<16, 0);//ugly encoding :|
		f.f[to][top_to+1] = stack_take();
		if (check_won()) return WON;
		return OK;
	} else return ERR;
	
}
int s2w(int from, int to, int opt) { /* stock to waste */
	(void) from; (void) to; (void) opt; /* don't need */
	if (f.z == 0) return ERR;
	f.w++;
	if (f.w == f.z) f.w = -1;
	return OK;
}
int w2s(int from, int to, int opt) { /* waste to stock (undo stock to waste) */
	(void) from; (void) to; (void) opt; /* don't need */
	if (f.z == 0) return ERR;
	f.w--;
	if (f.w < -1) f.w = f.z-1;
	return OK;
}
int f2t(int from, int to, int opt) { /* foundation to tableu */
	(void) from; /* don't need */
	int top_to = find_top(f.t[to]);
	from = opt;
	int top_from = find_top(f.f[from]);
	
	if ((get_color(f.t[to][top_to]) != get_color(f.f[from][top_from]))
	&& (rank_next(f.f[from][top_from], f.t[to][top_to]))) {
		f.t[to][top_to+1] = f.f[from][top_from];
		f.f[from][top_from] = NO_CARD;
		undo_push(FOUNDATION, to, from, 0);
		return OK;
	} else return ERR;
}
int w2t(int from, int to, int opt) { /* waste to tableu */
	(void) from; (void) opt; /* don't need */
	int top_to = find_top(f.t[to]);
	if (((get_color(f.t[to][top_to]) != get_color(f.s[f.w]))
	   && (rank_next(f.s[f.w], f.t[to][top_to])))
	|| (top_to < 0 && get_rank(f.s[f.w]) == RANK_K)) {
		undo_push(WASTE, to, f.w, 0);
		f.t[to][top_to+1] = stack_take();
		return OK;
	} else return ERR;
}
int t2t(int from, int to, int opt) { /* tableu to tableu */
	(void) opt; /* don't need */
	int top_to = find_top(f.t[to]);
	int top_from = find_top(f.t[from]);
	int count = 0; //NOTE: could probably be factored out
	for (int i = top_from; i >=0; i--) {
		if (((get_color(f.t[to][top_to]) != get_color(f.t[from][i]))
		   && (rank_next(f.t[from][i], f.t[to][top_to]))
		   && f.t[from][i] > NO_CARD) /* card face up? */
		|| (top_to < 0 && get_rank(f.t[from][i]) == RANK_K)) {
			/* move cards [i..top_from] to their destination */
			for (;i <= top_from; i++) {
				top_to++;
				f.t[to][top_to] = f.t[from][i];
				f.t[from][i] = NO_CARD;
				count++;
			}
			undo_push(from, to, count,
			turn_over(f.t[from]));
			return OK;
		}
	}
	return ERR; /* no such move possible */
}
#elif defined SPIDER
int remove_if_complete (int pileno) { //cleanup!
	card_t* pile = f.t[pileno];
	/* test if K...A complete; move to foundation if so */
	int top_from = find_top(pile);
	if (get_rank(pile[top_from]) != RANK_A) return 0;
	for (int i = top_from; i>=0; i--) {
		if (!is_consecutive (pile, i)) return 0;
		if (i+RANK_K == top_from /* if ace to king: remove it */
		    && get_rank(pile[top_from-RANK_K]) == RANK_K) {
			for(int i=top_from, j=0; i>top_from-NUM_RANKS; i--,j++){
				f.f[f.w][j] = pile[i];
				pile[i] = NO_CARD;
			}
			undo_push(pileno, FOUNDATION, f.w,
			turn_over(pile));
			f.w++;
			return 1;
		}
	}

	return 0;
}
int t2t(int from, int to, int opt) { //in dire need of cleanup
	int top_from = find_top(f.t[from]);
	int top_to = find_top(f.t[to]);
	int empty_to = (top_to < 0)? opt: -1; /* empty pile? */
	int count = 0; //NOTE: could probably be factored out

	for (int i = top_from; i >= 0; i--) {
		if (!is_consecutive(f.t[from], i)) break;

		/* is consecutive OR to empty pile and rank ok? */
		if (rank_next(f.t[from][i], f.t[to][top_to])
		|| (empty_to >= RANK_A && get_rank(f.t[from][i]) == empty_to)) {
			for (;i <= top_from; i++) {
				top_to++;
				f.t[to][top_to] = f.t[from][i];
				f.t[from][i] = NO_CARD;
				count++;
			}
			undo_push(from, to, count,
			turn_over(f.t[from]));
			remove_if_complete(to);
			if (check_won()) return WON;
			return OK;
		}
	}

	return ERR; /* no such move possible */
}
int s2t(int from, int to, int opt) {
	(void) from; (void) to; (void) opt; /* don't need */
	if (f.z <= 0) return ERR; /* stack out of cards */
	for (int pile = 0; pile < NUM_PILES; pile++)
		if (f.t[pile][0]==NO_CARD) return ERR; /*no piles may be empty*/
	for (int pile = 0; pile < NUM_PILES; pile++) {
		f.t[pile][find_top(f.t[pile])+1] = f.s[--f.z];
		remove_if_complete(pile);
		if (check_won()) return WON;
	}
	undo_push(STOCK, TABLEU, 1, 0);/*NOTE: puts 1 card on each tableu pile*/
	return OK;
}
int t2f(int from, int to, int opt) {
	(void) to; (void) opt; /* don't need */
	/* manually retrigger remove_if_complete() (e.g. after undo_pop) */
	return remove_if_complete(from)?OK:ERR;
}
#endif
//TODO: generalize prediction engine for CMD_HINT
#ifdef KLONDIKE
#define would_complete(pile) 0
#elif defined SPIDER
#define would_complete(pile) \
	(get_rank(f.t[pile][r[pile].top]) == RANK_A \
	&& get_rank(f.t[to][bottom_to]) == RANK_K)
#endif
#define would_turn(pile) \
	(f.t[pile][r[pile].pos-1] < 0)
#define would_empty(pile) \
	(r[pile].pos == 0)

int join(int to) {
	int top_to = find_top(f.t[to]);
#ifdef SPIDER
	int bottom_to = first_movable(f.t[to]);
#endif

#ifdef KLONDIKE
	if (to == FOUNDATION) {
		int status = ERR;
		for (int i = 0; i <= TAB_MAX; i++)
			switch ((i?t2f:w2f)(i-1, FOUNDATION, 0)) {
			case WON: return WON;
			case OK:  status = OK;
			case ERR: /* nop */;
			}
		return status;
	}

	if (top_to < 0) { /* move a king to empty pile: */
		for (int i = 0; i < TAB_MAX; i++) {
			if (f.t[i][0] < 0) /* i.e. would turn? */ //TODO: this calculation is wrong!
				//DOES NOT FIRE when king is in the middle
				if (t2t(i, to, 0) == OK) return OK;
		}
		return w2t(WASTE, to, 0);
	}
#endif

	struct rating {
		int ok:1;    /* card to move in pile? */
		int above;   /* number of movable cards above */
		int below;   /* number of cards below ours */
		int pos;     /* where the card to move is in the pile */
		int top;     /* find_top() */
	} r[NUM_PILES] = {{0}};
	int complete = 0;/* SPIDER: true if any pile would complete a stack */
	int turn =  0; /* SPIDER: true if any pile would turn_over */
	int empty = 0; /* true if any pile would become empty */

	/* 1. rate each pile: */
#ifdef SPIDER
	if (top_to < 0) {
		for (int pile = 0; pile < NUM_PILES; pile++) {
			if (pile == to) continue;
			int top = find_top(f.t[pile]);
			int bottom = first_movable(f.t[pile]);
			r[pile].pos = bottom; /* need for would_empty */

			if (top < 0) continue; /* no cards to move */
			if (would_empty(pile)) continue; /* doesn't help */

			r[pile].ok++;
			r[pile].above = 0; /* always take as many as possible */
			r[pile].below = top - bottom;
			r[pile].top = top;
			complete |= would_complete(pile); /* never happens */
			turn     |= would_turn(pile);
			empty    |= would_empty(pile);
		}
	} else
#endif
	for (int pile = 0; pile < NUM_PILES; pile++) {
		r[pile].top = r[pile].pos = find_top(f.t[pile]);
		/* backtrack until we find a compatible-to-'to'-pile card: */
		while (r[pile].pos >= 0 && is_movable(f.t[pile], r[pile].pos)) {
			int rankdiff = get_rank(f.t[pile][r[pile].pos])
			               - get_rank(f.t[to][top_to]);
			if (rankdiff >= 0) break; /* past our card */
			if (rankdiff == -1 /* rank matches */
#ifdef KLONDIKE
			&& get_color(f.t[pile][r[pile].pos]) /* color OK */
			   != get_color(f.t[to][top_to])
#elif defined SPIDER
			&& get_suit(f.t[pile][r[pile].pos]) /* color OK */
			   == get_suit(f.t[to][top_to])
#endif
			) {
				r[pile].ok++;
				complete |= would_complete(pile);
				turn     |= would_turn(pile);
				empty    |= would_empty(pile);
				for (int i = r[pile].pos; i >= 0; i--)
					if (is_movable(f.t[pile], i-1))
						r[pile].above++;
					else break;
				break;
			}
			r[pile].pos--;
			r[pile].below++;
		}
	}

	/* 2. find optimal pile: (optimized for spider) */
	//todo: in spider, prefer longest piles if above==0 (faster completions)
	int from = -1;
	for (int pile = 0, above = 99, below = 99; pile < NUM_PILES; pile++) {
		if (!r[pile].ok) continue;
		/* don't bother if another pile would be better: prefer ... */
		/* ... to complete a stack: */
		if (!would_complete(pile) && complete) continue;
		/* ... emptying piles: */
		if (!would_empty(pile) && empty && !complete) continue;
		/* ... to turn_over: */
		if (!would_turn(pile) && turn && !complete && !empty) continue;
		/* ... not to rip apart too many cards: */
		if (r[pile].above > above) continue;
		/* if tied, prefer ... */
		if (r[pile].above == above
		   /* ... larger pile if destination is empty */
		   && (top_to < 0? r[pile].below < below
		   /* ... shorter pile otherwise */
			         : r[pile].below > below))
			continue;

		from = pile;
		above = r[pile].above;
		below = r[pile].below;
	}

	/* 3. move cards over and return: */
#ifdef KLONDIKE
	/* prefer waste if it wouldn't turn_over: */
	if (!turn && w2t(WASTE, to, 0) == OK)
		return OK;
	if (from < 0) /* nothing found */
		return ERR;
	return t2t(from, to, 0);
#elif defined SPIDER
	if (from < 0) /* nothing found */
		return ERR;
	int bottom = first_movable(f.t[from]);
	return t2t(from, to, get_rank(f.t[from][bottom]));
#endif
}
#undef would_empty
#undef would_turn
#undef would_complete
int nop(int from, int to, int opt) { (void)from;(void)to;(void)opt;return ERR; }
// }}}

// keyboard input handling {{{
// cursor functions{{{
#ifdef KLONDIKE
void cursor_left (struct cursor* cursor) {
	if (is_tableu(cursor->pile)) {
		if (cursor->pile > 0) cursor->pile--;
		cursor->opt = 0;
	} else { /* stock/waste/foundation*/
		switch (cursor->pile) {
		case WASTE: cursor->pile = STOCK; cursor->opt = 0; break;
		case FOUNDATION:
			if (cursor->opt <= 0)
				cursor->pile = WASTE;
			else
				cursor->opt--;
		}
	}
}
void cursor_down (struct cursor* cursor) {
	if (!is_tableu(cursor->pile)) {
		switch (cursor->pile) {
		case STOCK: cursor->pile = TAB_1; break;
		case WASTE: cursor->pile = TAB_2; break;
		case FOUNDATION:
			cursor->pile = TAB_4 + cursor->opt;
		}
		cursor->opt = 0;
	}
}
void cursor_up (struct cursor* cursor) {
	if (is_tableu(cursor->pile)) {
		switch (cursor->pile) { //ugly :|
		case TAB_1: cursor->pile = STOCK; break;
		case TAB_2: cursor->pile = WASTE; break;
		case TAB_3: cursor->pile = WASTE; break;
		case TAB_4: case TAB_5: case TAB_6: case TAB_7:
			cursor->opt=cursor->pile-TAB_4;
			cursor->pile = FOUNDATION;
			break;
		}
	}
}
void cursor_right (struct cursor* cursor) {
	if (is_tableu(cursor->pile)) {
		if (cursor->pile < TAB_MAX) cursor->pile++;
	} else {
		switch (cursor->pile) {
		case STOCK: cursor->pile = WASTE; break;
		case WASTE: cursor->pile = FOUNDATION;cursor->opt = 0; break;
		case FOUNDATION:
			if (cursor->opt < NUM_SUITS-1)
				cursor->opt++;
		}
	}
}
#elif defined SPIDER
/*NOTE: one can't highlight the stock due to me being too lazy to implement it*/
void cursor_left (struct cursor* cursor) {
	if (cursor->pile > 0) cursor->pile--;
	cursor->opt = 0;
}
void cursor_down (struct cursor* cursor) {
	int first = first_movable(f.t[cursor->pile]);
	int top = find_top(f.t[cursor->pile]);
	if (first + cursor->opt < top)
		cursor->opt++;
}
void cursor_up (struct cursor* cursor) {
	if (cursor->opt > 0) cursor->opt--;
}
void cursor_right (struct cursor* cursor) {
	if (cursor->pile < TAB_MAX) cursor->pile++;
	cursor->opt = 0;
}
#endif
void cursor_to (struct cursor* cursor, int pile) {
	cursor->pile = pile;
	cursor->opt = 0;
}
int set_mouse(int pile, int* main, int* opt) {
	if (pile < 0) return 1;
	*main = pile;
#ifdef KLONDIKE
	if (pile >= FOUNDATION)
		*main = FOUNDATION,
		*opt  = pile - FOUNDATION;
#elif defined SPIDER
	(void)opt;
#endif
	return 0;
}
//}}}
int get_cmd (int* from, int* to, int* opt) {
	int _f, t;
	unsigned char mouse[6] = {0}; /* must clear [3]! */
	struct cursor inactive = {-1,-1};
	static struct cursor active = {0,0};
	active.opt = 0; /* always reset offset, but keep pile */

	/***/
from_l:	print_table(&active, &inactive);
	_f = getch(mouse);

	switch (_f) {
	/* direct addressing: */
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
	case '0': *from = TAB_10;break;
#elif defined KLONDIKE
	case '9': *from = WASTE; break;
	case '0': *from = FOUNDATION; break;
	case '8': /* fallthrough */
#endif
	case '\n': *from = STOCK; break;
	/* cursor keys addressing: */
	case KEY_LEFT:
	case 'h': cursor_left (&active); goto from_l;
	case KEY_DOWN:
	case 'j': cursor_down (&active); goto from_l;
	case KEY_UP:
	case 'k': cursor_up   (&active); goto from_l;
	case KEY_RIGHT:
	case 'l': cursor_right(&active); goto from_l;
	case KEY_HOME:
	case 'H': cursor_to(&active,TAB_1);  goto from_l; /* leftmost  tableu */
	case KEY_END:
	case 'L': cursor_to(&active,TAB_MAX);goto from_l; /* rigthmost tableu */
	case KEY_INS:
	case 'M': cursor_to(&active,TAB_MAX/2); goto from_l; /* center tableu */
	case ' ': /* continue with second cursor */
		*from = active.pile;
#ifdef KLONDIKE
		*opt = active.opt; /* when FOUNDATION */
#endif
		inactive = active;
		break;
	/* mouse addressing: */
	case MOUSE_MIDDLE: return CMD_NONE;
	case MOUSE_RIGHT:
		if (set_mouse(term2pile(mouse), to, opt))
			return CMD_INVAL;
		return CMD_JOIN;
	case MOUSE_LEFT:
		if (set_mouse(term2pile(mouse), from, opt))
			return CMD_INVAL;
		break;
	/* misc keys: */
	case ':':
		{char buf[256];
		fprintf (stderr, ":");
		raw_mode(0); /* turn on echo */
		fgets (buf, 256, stdin);
		raw_mode(1);
		switch(buf[0]) {
		case 'q': return CMD_QUIT;
		case 'n': return CMD_NEW;
		case 'r': return CMD_AGAIN;
		case 'h': return CMD_HELP;
		default:  return CMD_INVAL;
		}}
	case 'J':
		*to = active.pile;
#ifdef KLONDIKE
		if (*to == FOUNDATION) return CMD_JOIN;
#endif
		if (*to > TAB_MAX) return CMD_INVAL;
		return CMD_JOIN;
	case 'K': /* fallthrough */
	case '?': return CMD_HINT;
	case 'u': return CMD_UNDO;
	case EOF: return CMD_NONE; /* sent by SIGCONT */
	default: return CMD_INVAL;
	}
	inactive.pile = *from; /* for direct addressing highlighting */
	if (is_tableu(*from) && f.t[*from][0] == NO_CARD) return CMD_INVAL;

	if (*from == STOCK) {
		*to = WASTE;
		return CMD_MOVE;
	}

	/***/
to_l:	print_table(&active, &inactive);
	t = getch(mouse);

	switch (t) {
	case KEY_LEFT:
	case 'h': cursor_left (&active); goto to_l;
	case KEY_DOWN:
	case 'j': cursor_down (&active); goto to_l;
	case KEY_UP:
	case 'k': cursor_up   (&active); goto to_l;
	case KEY_RIGHT:
	case 'l': cursor_right(&active); goto to_l; 
	case KEY_HOME:
	case 'H': cursor_to(&active,TAB_1);     goto to_l;
	case KEY_END:
	case 'L': cursor_to(&active,TAB_MAX);   goto to_l;
	case KEY_INS:
	case 'M': cursor_to(&active,TAB_MAX/2); goto to_l;
	case 'J': /* fallthrough; just join selected pile */
	case ' ':
		*to = active.pile;
		break; /* continues with the foundation/empty tableu check */
	case MOUSE_MIDDLE:
	case MOUSE_RIGHT: return CMD_NONE;
	case MOUSE_LEFT:
		if (set_mouse(term2pile(mouse), to, opt))
			return CMD_INVAL;
/*#ifdef SPIDER
		//TODO: set opt if to field is empty; suppress "up do" dialog from below
		if (is_tableu(*to) && f.t[*to][0] == NO_CARD) {
			int top = find_top(f.t[*from]);
			if (top < 0) return CMD_INVAL;
			if (top >= 0 && !is_movable(f.t[*from], top-1)) {
				*opt = get_rank(f.t[*from][top]);
			} else {
			// ask user
			}
		}
#endif*/
		break;
	case 'K': /* fallthrough */
	case '?': return CMD_HINT;
	case 'u': return CMD_NONE; /* cancel selection */
	case EOF: return CMD_NONE; /* sent by SIGCONT */
	default:
		if (t < '0' || t > '9') return CMD_INVAL;
		if (t == '0')
#ifdef KLONDIKE
			*to = FOUNDATION;
#elif defined SPIDER
			*to = TAB_10;
#endif
		else
			*to = t-'1';
	}

	/***/
#ifdef KLONDIKE
	if (*from == FOUNDATION) {
		int top = find_top(f.t[*to]);
		if (top < 0) return CMD_INVAL;
		int color = get_color(f.t[*to][top]);
		int choice_1 = 1-color; /* selects piles of   */
		int choice_2 = 2+color; /* the opposite color */
		int top_c1 = find_top(f.f[choice_1]);
		int top_c2 = find_top(f.f[choice_2]);

		switch ((rank_next(f.f[choice_1][top_c1], f.t[*to][top])
		         && top_c1 >= 0 ) << 0
		       |(rank_next(f.f[choice_2][top_c2], f.t[*to][top])
		         && top_c2 >= 0 ) << 1) {
		case (       1<<0): *opt = choice_1; break; /* choice_1 only */
		case (1<<1       ): *opt = choice_2; break; /* choice_2 only */
		case (1<<1 | 1<<0): /* both, ask user which to pick from */
			printf ("take from (1-4): "); fflush (stdout);
			*opt = getch(NULL) - '1';
			if (*opt < 0 || *opt > 3) return CMD_INVAL;
			break;
		default: return CMD_INVAL; /* none matched */
		}
		/* `opt` is the foundation index (0..3) */
	}
#elif defined SPIDER
	/* moving to empty tableu? */
	if (is_tableu(*to) && f.t[*to][0] == NO_CARD) {
		int bottom = first_movable(f.t[*from]);
		if (inactive.opt >= 0) { /*if from was cursor addressed: */
			*opt = get_rank(f.t[*from][bottom + inactive.opt]);
			return CMD_MOVE;
		}
		int top = find_top(f.t[*from]);
		if (top < 0) return CMD_INVAL;
		if (top >= 0 && !is_movable(f.t[*from], top-1)) {
			*opt = get_rank(f.t[*from][top]);
		} else { /* only ask the user if it's unclear: */
			printf ("\rup to ([a23456789xjqk] or space/return): ");
			*opt = getch(NULL);
			switch (*opt) {
			case ' ': *opt = get_rank(f.t[*from][top]); break;
			case'\n': *opt = get_rank(f.t[*from][bottom]); break;
			case 'a': case 'A': *opt = RANK_A; break;
			case '0': /* fallthrough */
			case 'x': case 'X': *opt = RANK_X; break;
			case 'j': case 'J': *opt = RANK_J; break;
			case 'q': case 'Q': *opt = RANK_Q; break;
			case 'k': case 'K': *opt = RANK_K; break;
			default: *opt -= '1';
			}
			if (*opt < RANK_A || *opt > RANK_K) return ERR;
		}
		/* `opt` is the rank of the highest card to move */
	}
#endif
	return CMD_MOVE;
}

int getctrlseq(unsigned char* buf) {
	int c;
	enum esc_states {
		START,
		ESC_SENT,
		CSI_SENT,
		MOUSE_EVENT,
	} state = START;
	int offset = 0x20; /* never sends control chars as data */
	while ((c = getchar()) != EOF) {
		switch (state) {
		case START:
			switch (c) {
			case '\033': state=ESC_SENT; break;
			default: return c;
			}
			break;
		case ESC_SENT:
			switch (c) {
			case '[': state=CSI_SENT; break;
			default: return KEY_INVAL;
			}
			break;
		case CSI_SENT:
			switch (c) {
			case 'A': return KEY_UP;
			case 'B': return KEY_DOWN;
			case 'C': return KEY_RIGHT;
			case 'D': return KEY_LEFT;
			/*NOTE: home/end send ^[[x~ . no support for modifiers*/
			case 'H': return KEY_HOME;
			case 'F': return KEY_END;
			case '2': getchar(); return KEY_INS;
			case '5': getchar(); return KEY_PGUP;
			case '6': getchar(); return KEY_PGDN;
			case 'M': state=MOUSE_EVENT; break;
			default: return KEY_INVAL;
			}
			break;
		case MOUSE_EVENT:
			if (buf == NULL) return KEY_INVAL;
			buf[0] = c - offset;
			buf[1] = getchar() - offset;
			buf[2] = getchar() - offset;
			return MOUSE_ANY;
		default:
			return KEY_INVAL;
		}
	}
	return 2;
}
int term2pile(unsigned char *mouse) {
	int line   = (mouse[2]-1);
	int column = (mouse[1]-1) / op.s->width;

	if (line < op.s->height) { /* first line */
#ifdef KLONDIKE
		switch (column) {
		case 0: return STOCK;
		case 1: return WASTE;
		case 2: return -1; /* spacer */
		case 3: return FOUNDATION+0;
		case 4: return FOUNDATION+1;
		case 5: return FOUNDATION+2;
		case 6: return FOUNDATION+3;
		}
#elif defined SPIDER
		if (column < 3) return STOCK;
		return -1;
#endif
	} else if (line > op.s->height) { /* tableu */
		if (column <= TAB_MAX) return column;
	}
	return -1;
}
int wait_mouse_up(unsigned char* mouse) {
	struct cursor cur = {-1,-1};
	int level = 1;
	/* note: if dragged [3]==1 and second position is in mouse[0,4,5] */

	/* display a cursor while mouse button is pushed: */
	int pile = term2pile(mouse);
	cur.pile = pile;
#ifdef KLONDIKE
	if (pile >= FOUNDATION) {
		cur.pile = FOUNDATION;
		cur.opt = pile-FOUNDATION;
	}
#endif
	print_table(&cur, NO_HI); //TODO: should not overwrite inactive cursor!

	while (level > 0) {
		if (getctrlseq (mouse+3) == MOUSE_ANY) {
			/* ignore mouse wheel events: */
			if (mouse[3] & 0x40) continue;

			else if((mouse[3]&3) == 3) level--; /* release event */
			else level++; /* another button pressed */
		}
	}

	int success = mouse[1] == mouse[4] && mouse[2] == mouse[5];
	if (success) {
		mouse[3] = 0;
	}
	return success;
}

int getch(unsigned char* buf) {
/* returns a character, EOF, or constant for an escape/control sequence - NOT
compatible with the ncurses implementation of same name */
	int action;
	if (buf && buf[3]) {
		/* mouse was dragged; return 'ungetted' previous destination */
		action = MOUSE_DRAG;
		/* keep original [0], as [3] only contains release event */
		buf[1] = buf[4];
		buf[2] = buf[5];
		buf[3] = 0;
	} else {
		action = getctrlseq(buf);
	}

	switch (action) {
	case MOUSE_ANY:
		if (buf[0] > 3) break; /* ignore wheel events */
		wait_mouse_up(buf);
		/* fallthrough */
	case MOUSE_DRAG:
		switch (buf[0]) {
		case 0: return MOUSE_LEFT;
		case 1: return MOUSE_MIDDLE;
		case 2: return MOUSE_RIGHT;
		}
	}

	return action;
}
// }}}

// shuffling and dealing {{{
void deal(long seed) {
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
	srand (seed);
	for (int i = DECK_SIZE*NUM_DECKS-1; i > 0; i--) { /* fisher-yates */
		int j = rand() % (i+1);
		if (j-i) deck[i]^=deck[j],deck[j]^=deck[i],deck[i]^=deck[j];
	}

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
#ifdef KLONDIKE
	f.w = -1; /* @start: nothing on waste */
#elif defined SPIDER
	f.w = 0; /* number of used foundations */
#endif

	f.u = &undo_sentinel;
}
//}}}

// screen drawing routines {{{
void print_hi(int invert, int grey_bg, int bold, char* str) {
	if (bold && op.s == &unicode_large_color){ //awful hack for bold + faint
		int offset = str[3]==017?16:str[4]==017?17:0;
		printf ("%s%s%s""%.*s%s%s""%s%s%s",
			"\033[1m", invert?"\033[7m":"", grey_bg?"\033[100m":"",
			offset, str, bold?"\033[1m":"", str+offset,
			grey_bg?"\033[49m":"", invert?"\033[27m":"","\033[22m");
		return;
	}
	printf ("%s%s%s%s%s%s%s",
		bold?"\033[1m":"", invert?"\033[7m":"", grey_bg?"\033[100m":"",
		str,
		grey_bg?"\033[49m":"", invert?"\033[27m":"",bold?"\033[22m":"");
}
void print_table(const struct cursor* active, const struct cursor* inactive) {
	printf("\033[2J\033[H"); /* clear screen, reset cursor */
#ifdef KLONDIKE
	/* print stock, waste and foundation: */
	for (int line = 0; line < op.s->height; line++) {
		/* stock: */
		print_hi (active->pile == STOCK, inactive->pile == STOCK, 1, (
			(f.w < f.z-1)?op.s->facedown
			:op.s->placeholder)[line]);
		/* waste: */
		print_hi (active->pile == WASTE, inactive->pile == WASTE, 1, (
			/* NOTE: cast, because f.w sometimes is (short)-1 !? */
			((short)f.w >= 0)?op.s->card[f.s[f.w]]
			:op.s->placeholder)[line]);
		printf ("%s", op.s->card[NO_CARD][line]); /* spacer */
		/* foundation: */
		for (int pile = 0; pile < NUM_SUITS; pile++) {
			int card = find_top(f.f[pile]);
			print_hi (active->pile==FOUNDATION && active->opt==pile,
				inactive->pile==FOUNDATION && (
					/* cursor addr.     || direct addr.   */
					inactive->opt==pile || inactive->opt < 0
				), 1,
				(card < 0)?op.s->placeholder[line]
				:op.s->card[f.f[pile][card]][line]);
		}
		printf("\n");
	}
	printf("\n");
#elif defined SPIDER
	int fdone; for (fdone = NUM_DECKS*NUM_SUITS; fdone; fdone--)
		if (f.f[fdone-1][RANK_K]) break;  /*number of completed stacks*/
	int spacer_from = f.z?(f.z/10-1) * op.s->halfwidth[0] + op.s->width:0;
	int spacer_to   = NUM_PILES*op.s->width -
		((fdone?(fdone-1) * op.s->halfwidth[1]:0)+op.s->width);
	for (int line = 0; line < op.s->height; line++) {
		/* available stock: */
		for (int i = f.z/10; i; i--) {
			if (i==1) printf ("%s", op.s->facedown[line]);
			else      printf ("%s", op.s->halfstack[line]);
		}
		/* spacer: */
		for (int i = spacer_from; i < spacer_to; i++) printf (" ");
		/* foundation (overlapping): */
		for (int i = NUM_DECKS*NUM_SUITS-1, half = 0; i >= 0; i--) {
			int overlap = half? op.s->halfcard[line]: 0;
			if (f.f[i][RANK_K]) printf ("%.*s", op.s->halfwidth[2],
				op.s->card[f.f[i][RANK_K]][line]+overlap),
				half++;
		}
		printf("\n");
	}
	printf("\n");
#endif
#ifdef KLONDIKE
#define DO_HI(cursor) (cursor->pile == pile && (movable || empty))
#define TOP_HI(c) 1 /* can't select partial stacks in KLONDIKE */
#define INC_OFFSET
#elif defined SPIDER
	int offset[NUM_PILES]={1,1,1,1,1,1,1,1,1,1}; // :|
#define DO_HI(cursor) (cursor->pile == pile && (movable || empty) \
	&& offset[pile] > cursor->opt)
#define TOP_HI(cursor) (cursor->pile == pile && movable \
	&& offset[pile]-1 == cursor->opt)
#define INC_OFFSET if (movable) offset[pile]++
#endif
	/* print tableu piles: */
	int row[NUM_PILES] = {0};
	int line[NUM_PILES]= {0};
	int label[NUM_PILES]={0};
	int line_had_card;
	int did_placeholders = 0;
	do {
		line_had_card = 0;
		for (int pile = 0; pile < NUM_PILES; pile++) {
			card_t card = f.t[pile][row[pile]];
			card_t next = f.t[pile][row[pile]+1];
			int movable = is_movable(f.t[pile], row[pile]);
			int empty   = !card && row[pile] == 0;

			print_hi (DO_HI(active), DO_HI(inactive), movable, (
				(!card && row[pile] == 0)?op.s->placeholder
				:(card<0)?op.s->facedown
				:op.s->card[card]
				)[line[pile]]);

			int extreme_overlap = ( 3  /* spacer, labels, status */
				+ 2 * op.s->height /* stock, top tableu card */
				+ find_top(f.t[pile]) * op.s->overlap) >op.w[0];
			/* normal overlap: */
			if (++line[pile] >= (next?op.s->overlap:op.s->height)
			/* extreme overlap on closed cards: */
			|| (extreme_overlap &&
			    line[pile] >= 1 &&
			    f.t[pile][row[pile]] < 0 &&
			    f.t[pile][row[pile]+1] <0)
			/* extreme overlap on sequences: */
			|| (extreme_overlap &&
			    !TOP_HI(active) && /*always show top selected card*/
			    line[pile] >= 1 && row[pile] > 0 &&
			    f.t[pile][row[pile]-1] > NO_CARD &&
			    is_consecutive (f.t[pile], row[pile]) &&
			    is_consecutive (f.t[pile], row[pile]-1) &&
			    f.t[pile][row[pile]+1] != NO_CARD)
			) {
				line[pile]=0;
				row[pile]++;
				INC_OFFSET;
			}
			/* tableu labels: */
			if(!card && !label[pile] && row[pile]>0&&line[pile]>0) {
				label[pile] = 1;
				printf ("\b\b%d ", (pile+1) % 10); //XXX: hack
			}
			line_had_card |= !!card;
			did_placeholders |= row[pile] > 0;
		}
		printf ("\n");
	} while (line_had_card || !did_placeholders);
}

void visbell (void) {
	printf ("\033[?5h"); fflush (stdout);
	usleep (100000);
	printf ("\033[?5l"); fflush (stdout);
}
void win_anim(void) {
	printf ("\033[?25l"); /* hide cursor */
	for (;;) {
		/* set cursor to random location */
		int row = 1+rand()%(24-op.s->width);
		int col = 1+rand()%(80-op.s->height);

		/* draw random card */
		int face = 1 + rand() % 52;
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
//}}}

// undo logic {{{
void undo_push (int _f, int t, int n, int o) {
	struct undo* new = malloc(sizeof(struct undo));
	new->f = _f;
	new->t = t;
	new->n = n;
	new->o = o;
	new->prev = f.u;
	new->next = NULL;
	f.u->next = new;
	f.u = f.u->next;
}
void undo_pop (struct undo* u) {
	if (u == &undo_sentinel) return;

#ifdef KLONDIKE
	if (u->f == FOUNDATION) {
		/* foundation -> tableu */
		int top_f = find_top(f.f[u->n]);
		int top_t = find_top(f.t[u->t]);
		f.f[u->n][top_f+1] = f.t[u->t][top_t];
		f.t[u->t][top_t] = NO_CARD;
	} else if (u->f == WASTE && u->t == FOUNDATION) {
		/* waste -> foundation */
		/* split u->n into wst and fnd: */
		int wst = u->n & 0xffff;
		int fnd = u->n >> 16;
		/* move stock cards one position up to make room: */
		for (int i = f.z; i >= wst; i--) f.s[i+1] = f.s[i];
		/* move one card from foundation to waste: */
		int top = find_top(f.f[fnd]);
		f.s[wst] = f.f[fnd][top];
		f.f[fnd][top] = NO_CARD;
		f.z++;
		f.w++;
	} else if (u->f == WASTE) {
		/* waste -> tableu */
		/* move stock cards one position up to make room: */
		for (int i = f.z; i >= u->n; i--) f.s[i+1] = f.s[i];
		/* move one card from tableu to waste: */
		int top = find_top(f.t[u->t]);
		f.s[u->n] = f.t[u->t][top];
		f.t[u->t][top] = NO_CARD;
		f.z++;
		f.w++;
	} else if (u->t == FOUNDATION) {
		/* tableu -> foundation */
		int top_f = find_top(f.t[u->f]);
		int top_t = find_top(f.f[u->n]);
		/* close topcard if previous action caused turn_over(): */
		if (u->o) f.t[u->f][top_f] *= -1;
		/* move one card from foundation to tableu: */
		f.t[u->f][top_f+1] = f.f[u->n][top_t];
		f.f[u->n][top_t] = NO_CARD;
	} else {
		/* tableu -> tableu */
		int top_f = find_top(f.t[u->f]);
		int top_t = find_top(f.t[u->t]);
		/* close topcard if previous action caused turn_over(): */
		if (u->o) f.t[u->f][top_f] *= -1;
		/* move n cards from tableu[f] to tableu[t]: */
		for (int i = 0; i < u->n; i++) {
			f.t[u->f][top_f+u->n-i] = f.t[u->t][top_t-i];
			f.t[u->t][top_t-i] = NO_CARD;
		}
	}
#elif defined SPIDER
	if (u->f == STOCK) {
		/* stock -> tableu */
		/*remove a card from each pile and put it back onto the stock:*/
		for (int pile = NUM_PILES-1; pile >= 0; pile--) {
			int top = find_top(f.t[pile]);
			f.s[f.z++] = f.t[pile][top];
			f.t[pile][top] = NO_CARD;
		}
	} else if (u->t == FOUNDATION) {
		/* tableu -> foundation */
		int top = find_top(f.t[u->f]);
		/* close topcard if previous action caused turn_over(): */
		if (u->o) f.t[u->f][top] *= -1;
		/* append cards from foundation to tableu */
		for (int i = RANK_K; i >= RANK_A; i--) {
			f.t[u->f][++top] = f.f[u->n][i];
			f.f[u->n][i] = NO_CARD;
		}
		f.w--; /* decrement complete-foundation-counter */

	} else {
		/* tableu -> tableu */
		int top_f = find_top(f.t[u->f]);
		int top_t = find_top(f.t[u->t]);
		/* close topcard if previous action caused turn_over(): */
		if (u->o) f.t[u->f][top_f] *= -1;
		/* move n cards from tableu[f] to tableu[t]: */
		for (int i = 0; i < u->n; i++) {
			f.t[u->f][top_f+u->n-i] = f.t[u->t][top_t-i];
			f.t[u->t][top_t-i] = NO_CARD;
		}
	}
#endif

	void* old = f.u;
	f.u = f.u->prev;
	free(old);
}
void free_undo (struct undo* u) {
	while (u && u != &undo_sentinel) {
		void* old = u;
		u = u->prev;
		free (old);
	}
}
//}}}

// initialization stuff {{{
void screen_setup (int enable) {
	if (enable) {
		raw_mode(1);
		printf ("\033[s\033[?47h"); /* save cursor, alternate screen */
		printf ("\033[H\033[J"); /* reset cursor, clear screen */
		printf ("\033[?1000h"); /* enable mouse */
	} else {
		printf ("\033[?1000l"); /* disable mouse */
		printf ("\033[?47l\033[u"); /* primary screen, restore cursor */
		raw_mode(0);
	}
}

void raw_mode(int enable) {
	static struct termios saved_term_mode;
	struct termios raw_term_mode;

	if (enable) {
		if (saved_term_mode.c_lflag == 0)/*don't overwrite stored mode*/
			tcgetattr(STDIN_FILENO, &saved_term_mode);
		raw_term_mode = saved_term_mode;
		raw_term_mode.c_lflag &= ~(ICANON | ECHO);
		raw_term_mode.c_cc[VMIN] = 1 ;
		raw_term_mode.c_cc[VTIME] = 0;
		tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_term_mode);
	} else {
		tcsetattr(STDIN_FILENO, TCSAFLUSH, &saved_term_mode);
	}
}

void signal_handler (int signum) {
	struct winsize w;
	switch (signum) {
	case SIGTSTP:
		screen_setup(0);
		signal(SIGTSTP, SIG_DFL); /* NOTE: assumes SysV semantics! */
		raise(SIGTSTP);
		break;
	case SIGCONT:
		screen_setup(1);
		print_table(NO_HI, NO_HI);
		break;
	case SIGINT: //TODO: don't exit; just warn like vim does
		exit(128+SIGINT);
	case SIGWINCH:
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
		op.w[0] = w.ws_row;
		op.w[1] = w.ws_col;
		break;
	}
}
void signal_setup(void) {
	struct sigaction saction;

	saction.sa_handler = signal_handler;
	sigemptyset(&saction.sa_mask);
	saction.sa_flags = 0;
	if (sigaction(SIGTSTP, &saction, NULL) < 0) {
		perror ("SIGTSTP");
		exit (1);
	}
	if (sigaction(SIGCONT, &saction, NULL) < 0) {
		perror ("SIGCONT");
		exit (1);
	}
	if (sigaction(SIGINT, &saction, NULL) < 0) {
		perror ("SIGINT");
		exit (1);
	}
	if (sigaction(SIGWINCH, &saction, NULL) < 0) {
		perror ("SIGWINCH");
		exit (1);
	}
}
//}}}

//vim: foldmethod=marker
