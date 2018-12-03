#ifndef __SOL_H__
#define __SOL_H__

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

#define NO_HI -1 /* print_table()'s highlighter also uses field_places */
/* WARN: stock must always follow immediately after `TAB_*` for is_tableu() */
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
	CMD_HINT,
	CMD_HELP,
	CMD_FIND,
};

enum difficulty {
	NORMAL,
	MEDIUM,
	EASY,
};

typedef signed char card_t;

void sol(void);
int find_top(card_t* pile);
void turn_over(card_t* pile);
int check_won(void);
void win_anim(void);
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
int is_consecutive (card_t* pile, int pos);
void remove_if_complete (card_t* pile);
int t2t(int from, int to, int opt);
int s2t(int from, int to, int opt);
#endif
int nop(int from, int to, int opt);
int get_cmd (int* from, int* to, int* opt);
void deal(void);
int is_movable(card_t* pile, int n);
void print_table(int highlight);
void visbell (void);
void append_undo (int n, int f, int t);
void screen_setup (int enable);
void raw_mode(int enable);

#endif
