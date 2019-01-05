#ifndef __SOL_H__
#define __SOL_H__

#define SHORTHELP "%s [OPTIONS]\n"
#ifdef KLONDIKE
#define LONGHELP_SPECIFIC ""
#define DIRECT_ADDR_KEYHELP \
	"    1 .. 7: directly address tableu\n" \
	"    8,9,0 : directly address stock/waste/foundation\n"
#elif defined SPIDER
#define LONGHELP_SPECIFIC \
	"    -d(ifficulty) (eady|medium|hard)\n"
#define DIRECT_ADDR_KEYHELP \
	"    1 .. 0: directly address tableu\n"
#endif
#define LONGHELP \
	"OPTIONS:\n" \
	LONGHELP_SPECIFIC \
	"    -o(ption) (consv=conserve vertical space)\n" \
	"    -s(cheme) (color|mono|small)\n" \
	"    -h(elp)\n" \
	"\n"
#define KEYHELP \
	"Keybindings:\n" \
	"    hjkl  : move cursor\n" \
	"    H, L  : move cursor to first/last tableu pile\n" \
	"    J, K  : join to here, show hint\n" \
	"    n, q  : new game, quit\n" \
	"    space : select at cursor\n" \
	"    return: draw from stock\n" \
	DIRECT_ADDR_KEYHELP

#define DECK_SIZE 52
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
#define WASTE 0; /* need it for get_cmd(), but don't count it in NUM_PLACES */
#define TABLEU NUM_PLACES; /* for undo .t when .f==STACK */
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
};

enum difficulty {
	NORMAL,
	MEDIUM,
	EASY,
};

typedef signed char card_t;

struct cursor {
	int pile;
	int opt; /* klondike: foundation id; spider: move nth movable card */
};
const struct cursor no_hi = {-1, -1};
#define NO_HI &no_hi

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
void remove_if_complete (card_t* pile);
int t2t(int from, int to, int opt);
int s2t(int from, int to, int opt);
#endif
int nop(int from, int to, int opt);
void cursor_left (struct cursor* cursor);
void cursor_down (struct cursor* cursor);
void cursor_up (struct cursor* cursor);
void cursor_right (struct cursor* cursor);
int get_cmd (int* from, int* to, int* opt);
void deal(void);
void print_hi(int invert, int grey_bg, int bold, char* str);
void print_table(const struct cursor* active, const struct cursor* inactive);
void visbell (void);
void win_anim(void);
void append_undo (int n, int f, int t);
void screen_setup (int enable);
void raw_mode(int enable);
void signal_handler (int signum);
void signal_setup(void);

#endif
