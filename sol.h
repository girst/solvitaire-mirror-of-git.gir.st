#ifndef __SOL_H__
#define __SOL_H__

// enums and constants {{{
#define DECK_SIZE 52
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

enum cards {
	NO_CARD,
	CLU_A,	DIA_A,	HEA_A,	SPA_A,
	CLU_2,	DIA_2,	HEA_2,	SPA_2,
	CLU_3,	DIA_3,	HEA_3,	SPA_3,
	CLU_4,	DIA_4,	HEA_4,	SPA_4,
	CLU_5,	DIA_5,	HEA_5,	SPA_5,
	CLU_6,	DIA_6,	HEA_6,	SPA_6,
	CLU_7,	DIA_7,	HEA_7,	SPA_7,
	CLU_8,	DIA_8,	HEA_8,	SPA_8,
	CLU_9,	DIA_9,	HEA_9,	SPA_9,
	CLU_X,	DIA_X,	HEA_X,	SPA_X,
	CLU_J,	DIA_J,	HEA_J,	SPA_J,
	CLU_Q,	DIA_Q,	HEA_Q,	SPA_Q,
	CLU_K,	DIA_K,	HEA_K,	SPA_K,
	_NUM_CARDS_internal
};
enum colors {
	BLK,
	RED,
	NUM_COLORS
};
enum suits {
	CLUBS,
	DIAMONDS,
	HEARTS,
	SPADES,
	NUM_SUITS
};
enum ranks {
	RANK_A,
	RANK_2,
	RANK_3,
	RANK_4,
	RANK_5,
	RANK_6,
	RANK_7,
	RANK_8,
	RANK_9,
	RANK_X,
	RANK_J,
	RANK_Q,
	RANK_K,
	NUM_RANKS
};

enum action_return {
	OK,  /*move successful*/
	ERR, /*invalid move*/
	WON, /*game won*/
};
enum game_states {
	GAME_NEW,
	GAME_WON,
	GAME_QUIT,
};

/* WARN: stock must always follow immediately after `TAB_*`! */
#define TAB_MAX (STOCK-1)
enum field_places {
	TAB_1,
	TAB_2,
	TAB_3,
	TAB_4,
	TAB_5,
	TAB_6,
	TAB_7,
#ifdef SPIDER
	TAB_8,
	TAB_9,
	TAB_10,
	STOCK,
#define WASTE 0            /* for action[][10] (must be valid index) */
#define TABLEU STOCK+1     /* for undo{.t} (value never read) */
#define FOUNDATION STOCK+2 /* for undo{.t} (must be unique) */
#elif defined KLONDIKE
	STOCK,
	WASTE,
	FOUNDATION,
#endif
	NUM_PLACES,
};
enum special_cmds {
	CMD_MOVE,
	CMD_INVAL,
	CMD_NONE,
	CMD_QUIT,
	CMD_NEW,
	CMD_AGAIN,
	CMD_HINT,
	CMD_JOIN,
	CMD_UNDO,
};

enum event {
	/* for getctrlseq() */
	KEY_NULL     =  0,
	KEY_EOF      = -1,
	KEY_INVAL    = -2,
	MOUSE_ANY    = -3,
	/* for getch() */
	MOUSE_LEFT   = -4,
	MOUSE_MIDDLE = -5,
	MOUSE_RIGHT  = -6,
	MOUSE_DRAG   = -7,
	KEY_LEFT     = -8,
	KEY_DOWN     = -9,
	KEY_UP       = -10,
	KEY_RIGHT    = -11,
	KEY_HOME     = -12,
	KEY_END      = -13,
	KEY_INS      = -14,
	KEY_PGUP     = -15,
	KEY_PGDN     = -16,
};

enum difficulty {
	NORMAL,
	MEDIUM,
	EASY,
};
//}}}

typedef signed char card_t;

struct playfield {
	int z; /* stock size */
	int w; /* waste; index into stock (occupied foundations in spider) */
	card_t s[MAX_STOCK]; /* stock */
	card_t f[NUM_DECKS*NUM_SUITS][PILE_SIZE]; /* foundation */
	card_t t[NUM_PILES][PILE_SIZE]; /* tableu piles */
	struct undo {
		int f; /* pile cards were taken from */
		int t; /* pile cards were moved to */
		int n; /* if tableu: number of cards moved */
		       /* else: index into stock/foundation */
		int o; /* turn_over() fired? */
		struct undo* prev;
		struct undo* next;
	}* u;
};
struct opts {
#ifdef SPIDER
	int m; /* difficulty mode */
#endif
	unsigned short w[2]; /* terminal window rows/columns */
	const struct scheme* s;
};
struct cursor {
	int pile;
	int opt; /* klondike: foundation id; spider: move nth movable card */
};
const struct cursor no_hi = {-1, -1};
#define NO_HI &no_hi

struct undo undo_sentinel;

// help texts {{{
#define SHORTHELP "%s [OPTIONS]\n"
#ifdef KLONDIKE
#define LONGHELP_SPECIFIC ""
#define DIRECT_ADDR_KEYHELP \
	"    1 .. 7: directly address tableu\n" \
	"    8,9,0 : directly address stock/waste/foundation\n"
#elif defined SPIDER
#define LONGHELP_SPECIFIC \
	"    -s(uits) <1, 2 or 4>\n"
#define DIRECT_ADDR_KEYHELP \
	"    1 .. 0: directly address tableu\n"
#endif
#define LONGHELP \
	"OPTIONS:\n" \
	LONGHELP_SPECIFIC \
	"    -b(land colorscheme)\n" \
	"    -c(olorful colorscheme)\n" \
	"    -m(iniature colorscheme)\n" \
	"    -h(elp)\n" \
	"\n"
#define KEYHELP \
	"Keybindings:\n" \
	"    hjkl  : move cursor (or cursor keys)\n" \
	"    H,M,L : move cursor to first/centre/last tableu pile (or home/ins/end)\n" \
	"    J     : join to here\n" \
	/*"    K     : show hint\n" */\
	"    space : select at cursor\n" \
	"    return: draw from stock\n" \
	"    :n    : new game\n" \
	"    :q    : quit\n" \
	DIRECT_ADDR_KEYHELP
//}}}

int sol(void);
void quit(void);
int find_top(card_t* pile);
int first_movable(card_t* pile);
int turn_over(card_t* pile);
int check_won(void);
int rank_next (card_t a, card_t b);
int is_consecutive (card_t* pile, int pos);
int is_movable(card_t* pile, int n);
#ifdef KLONDIKE
card_t stack_take(void);
int t2f(int from, int to, int opt);
int w2f(int from, int to, int opt);
int s2w(int from, int to, int opt);
int w2s(int from, int to, int opt);
int f2t(int from, int to, int opt);
int w2t(int from, int to, int opt);
int t2t(int from, int to, int opt);
#elif defined SPIDER
int remove_if_complete (int pileno);
int t2t(int from, int to, int opt);
int s2t(int from, int to, int opt);
int t2f(int from, int to, int opt);
#endif
int join(int to);
int nop(int from, int to, int opt);
void cursor_left (struct cursor* cursor);
void cursor_down (struct cursor* cursor);
void cursor_up (struct cursor* cursor);
void cursor_right (struct cursor* cursor);
void cursor_to (struct cursor* cursor, int pile);
int get_cmd (int* from, int* to, int* opt);
int getctrlseq(unsigned char* buf);
int term2pile(unsigned char *mouse);
int wait_mouse_up(unsigned char* mouse);
int getch(unsigned char* buf);
void deal(long seed);
void print_hi(int invert, int grey_bg, int bold, char* str);
void print_table(const struct cursor* active, const struct cursor* inactive);
void visbell (void);
void win_anim(void);
void undo_push (int f, int t, int n, int o);
void undo_pop (struct undo* u);
void free_undo (struct undo* u);
void screen_setup (int enable);
void raw_mode(int enable);
void signal_handler (int signum);
void signal_setup(void);

#endif
