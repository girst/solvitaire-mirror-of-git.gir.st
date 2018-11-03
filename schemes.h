#ifndef __SCHEMES_H__
#define __SCHEMES_H__

#include "sol.h"

struct scheme {
	int width; /* column alignment */
	int height; /* height of a card */ //TODO: obsoletes NULL termination
	int overlap; /* no of lines to draw when cards overlapp */
	char** card[_NUM_CARDS_internal];
	char** facedown;
	char** placeholder;
	char* init_seq; /* for setting up charset */
	char* reset_seq;
};

#define ULCARD(s, r) (char*[]) \
{"╭───╮",\
 "│"s" "r"│",\
 "│ "s" │",\
 "╰───╯"}
#define RULCARD(s, r) \
	ULCARD("\033[91m" s "\033[0m", r)
#define BULCARD(s, r) \
	ULCARD("\033[37m" s "\033[0m", r)
#define USCARD(c) (char*[]){c}

const struct scheme unicode_large_mono = {
	.width = 5,
	.height = 4,
	.overlap = 2,
	.card = {
		[NO_CARD] = (char*[]){"     ","     ","     ","     "},
		[CLU_A] = ULCARD("♣","A"), [DIA_A] = ULCARD("♦","A"),
		[HEA_A] = ULCARD("♥","A"), [SPA_A] = ULCARD("♠","A"),
		[CLU_2] = ULCARD("♣","2"), [DIA_2] = ULCARD("♦","2"),
		[HEA_2] = ULCARD("♥","2"), [SPA_2] = ULCARD("♠","2"),
		[CLU_3] = ULCARD("♣","3"), [DIA_3] = ULCARD("♦","3"),
		[HEA_3] = ULCARD("♥","3"), [SPA_3] = ULCARD("♠","3"),
		[CLU_4] = ULCARD("♣","4"), [DIA_4] = ULCARD("♦","4"),
		[HEA_4] = ULCARD("♥","4"), [SPA_4] = ULCARD("♠","4"),
		[CLU_5] = ULCARD("♣","5"), [DIA_5] = ULCARD("♦","5"),
		[HEA_5] = ULCARD("♥","5"), [SPA_5] = ULCARD("♠","5"),
		[CLU_6] = ULCARD("♣","6"), [DIA_6] = ULCARD("♦","6"),
		[HEA_6] = ULCARD("♥","6"), [SPA_6] = ULCARD("♠","6"),
		[CLU_7] = ULCARD("♣","7"), [DIA_7] = ULCARD("♦","7"),
		[HEA_7] = ULCARD("♥","7"), [SPA_7] = ULCARD("♠","7"),
		[CLU_8] = ULCARD("♣","8"), [DIA_8] = ULCARD("♦","8"),
		[HEA_8] = ULCARD("♥","8"), [SPA_8] = ULCARD("♠","8"),
		[CLU_9] = ULCARD("♣","9"), [DIA_9] = ULCARD("♦","9"),
		[HEA_9] = ULCARD("♥","9"), [SPA_9] = ULCARD("♠","9"),
		[CLU_X] = ULCARD("♣","X"), [DIA_X] = ULCARD("♦","X"),
		[HEA_X] = ULCARD("♥","X"), [SPA_X] = ULCARD("♠","X"),
		[CLU_J] = ULCARD("♣","J"), [DIA_J] = ULCARD("♦","J"),
		[HEA_J] = ULCARD("♥","J"), [SPA_J] = ULCARD("♠","J"),
		[CLU_Q] = ULCARD("♣","Q"), [DIA_Q] = ULCARD("♦","Q"),
		[HEA_Q] = ULCARD("♥","Q"), [SPA_Q] = ULCARD("♠","Q"),
		[CLU_K] = ULCARD("♣","K"), [DIA_K] = ULCARD("♦","K"),
		[HEA_K] = ULCARD("♥","K"), [SPA_K] = ULCARD("♠","K"),
	},
	.facedown = (char*[]){
		"╭───╮",
		"│▚▚▚│",
		"│▚▚▚│",
		"╰───╯"
	},
	.placeholder = (char*[]){
		"╭╌╌╌╮",
		"╎   ╎",
		"╎   ╎",
		"╰╌╌╌╯"
	},
	.init_seq = NULL,
	.reset_seq = NULL,
};
const struct scheme unicode_large_color = {
	.width = 5,
	.height = 4,
	.overlap = 2,
	.card = {
		[NO_CARD] = (char*[]){"     ","     ","     ","     "},
		[CLU_A] = BULCARD("♣","A"), [DIA_A] = RULCARD("♦","A"),
		[HEA_A] = RULCARD("♥","A"), [SPA_A] = BULCARD("♠","A"),
		[CLU_2] = BULCARD("♣","2"), [DIA_2] = RULCARD("♦","2"),
		[HEA_2] = RULCARD("♥","2"), [SPA_2] = BULCARD("♠","2"),
		[CLU_3] = BULCARD("♣","3"), [DIA_3] = RULCARD("♦","3"),
		[HEA_3] = RULCARD("♥","3"), [SPA_3] = BULCARD("♠","3"),
		[CLU_4] = BULCARD("♣","4"), [DIA_4] = RULCARD("♦","4"),
		[HEA_4] = RULCARD("♥","4"), [SPA_4] = BULCARD("♠","4"),
		[CLU_5] = BULCARD("♣","5"), [DIA_5] = RULCARD("♦","5"),
		[HEA_5] = RULCARD("♥","5"), [SPA_5] = BULCARD("♠","5"),
		[CLU_6] = BULCARD("♣","6"), [DIA_6] = RULCARD("♦","6"),
		[HEA_6] = RULCARD("♥","6"), [SPA_6] = BULCARD("♠","6"),
		[CLU_7] = BULCARD("♣","7"), [DIA_7] = RULCARD("♦","7"),
		[HEA_7] = RULCARD("♥","7"), [SPA_7] = BULCARD("♠","7"),
		[CLU_8] = BULCARD("♣","8"), [DIA_8] = RULCARD("♦","8"),
		[HEA_8] = RULCARD("♥","8"), [SPA_8] = BULCARD("♠","8"),
		[CLU_9] = BULCARD("♣","9"), [DIA_9] = RULCARD("♦","9"),
		[HEA_9] = RULCARD("♥","9"), [SPA_9] = BULCARD("♠","9"),
		[CLU_X] = BULCARD("♣","X"), [DIA_X] = RULCARD("♦","X"),
		[HEA_X] = RULCARD("♥","X"), [SPA_X] = BULCARD("♠","X"),
		[CLU_J] = BULCARD("♣","J"), [DIA_J] = RULCARD("♦","J"),
		[HEA_J] = RULCARD("♥","J"), [SPA_J] = BULCARD("♠","J"),
		[CLU_Q] = BULCARD("♣","Q"), [DIA_Q] = RULCARD("♦","Q"),
		[HEA_Q] = RULCARD("♥","Q"), [SPA_Q] = BULCARD("♠","Q"),
		[CLU_K] = BULCARD("♣","K"), [DIA_K] = RULCARD("♦","K"),
		[HEA_K] = RULCARD("♥","K"), [SPA_K] = BULCARD("♠","K"),
	},
	.facedown = (char*[]){
		"╭───╮",
		"│\033[94m▚▚▚\033[0m│",
		"│\033[94m▚▚▚\033[0m│",
		"╰───╯"
	},
	.placeholder = (char*[]){
		"╭╌╌╌╮",
		"╎   ╎",
		"╎   ╎",
		"╰╌╌╌╯"
	},
	.init_seq = NULL,
	.reset_seq = NULL,
};
const struct scheme unicode_small_mono = {
	.width = 2,
	.height = 1,
	.overlap = 1,
	.card = {
		[NO_CARD] = (char*[]){"  "},
		[CLU_A] = USCARD("🃑 "), [DIA_A] = USCARD("🃁 "),
		[HEA_A] = USCARD("🂱 "), [SPA_A] = USCARD("🂡 "),
		[CLU_2] = USCARD("🃒 "), [DIA_2] = USCARD("🃂 "),
		[HEA_2] = USCARD("🂲 "), [SPA_2] = USCARD("🂢 "),
		[CLU_3] = USCARD("🃓 "), [DIA_3] = USCARD("🃃 "),
		[HEA_3] = USCARD("🂳 "), [SPA_3] = USCARD("🂣 "),
		[CLU_4] = USCARD("🃔 "), [DIA_4] = USCARD("🃄 "),
		[HEA_4] = USCARD("🂴 "), [SPA_4] = USCARD("🂤 "),
		[CLU_5] = USCARD("🃕 "), [DIA_5] = USCARD("🃅 "),
		[HEA_5] = USCARD("🂵 "), [SPA_5] = USCARD("🂥 "),
		[CLU_6] = USCARD("🃖 "), [DIA_6] = USCARD("🃆 "),
		[HEA_6] = USCARD("🂶 "), [SPA_6] = USCARD("🂦 "),
		[CLU_7] = USCARD("🃗 "), [DIA_7] = USCARD("🃇 "),
		[HEA_7] = USCARD("🂷 "), [SPA_7] = USCARD("🂧 "),
		[CLU_8] = USCARD("🃘 "), [DIA_8] = USCARD("🃈 "),
		[HEA_8] = USCARD("🂸 "), [SPA_8] = USCARD("🂨 "),
		[CLU_9] = USCARD("🃙 "), [DIA_9] = USCARD("🃉 "),
		[HEA_9] = USCARD("🂹 "), [SPA_9] = USCARD("🂩 "),
		[CLU_X] = USCARD("🃚 "), [DIA_X] = USCARD("🃊 "),
		[HEA_X] = USCARD("🂺 "), [SPA_X] = USCARD("🂪 "),
		[CLU_J] = USCARD("🃛 "), [DIA_J] = USCARD("🃋 "),
		[HEA_J] = USCARD("🂻 "), [SPA_J] = USCARD("🂫 "),
		[CLU_Q] = USCARD("🃝 "), [DIA_Q] = USCARD("🃍 "),
		[HEA_Q] = USCARD("🂽 "), [SPA_Q] = USCARD("🂭 "),
		[CLU_K] = USCARD("🃞 "), [DIA_K] = USCARD("🃎 "),
		[HEA_K] = USCARD("🂾 "), [SPA_K] = USCARD("🂮 "),
	},
	.facedown = (char*[]){"🂠 "},
	.placeholder = (char*[]){"❲❳"},
	.init_seq = NULL,
	.reset_seq = NULL,
};

const struct scheme vt220_small = { //TODO: this is a placeholder
	.width = 2,
	.height = 1,
	.overlap = 1,
	.card = {
		[NO_CARD] = (char*[]){"  "},
		[CLU_A] = USCARD("CA"), [DIA_A] = USCARD("DA"),
		[HEA_A] = USCARD("HA"), [SPA_A] = USCARD("SA"),
		[CLU_2] = USCARD("C2"), [DIA_2] = USCARD("D2"),
		[HEA_2] = USCARD("H2"), [SPA_2] = USCARD("S2"),
		[CLU_3] = USCARD("C3"), [DIA_3] = USCARD("D3"),
		[HEA_3] = USCARD("H3"), [SPA_3] = USCARD("S3"),
		[CLU_4] = USCARD("C4"), [DIA_4] = USCARD("D4"),
		[HEA_4] = USCARD("H4"), [SPA_4] = USCARD("S4"),
		[CLU_5] = USCARD("C5"), [DIA_5] = USCARD("D5"),
		[HEA_5] = USCARD("H5"), [SPA_5] = USCARD("S5"),
		[CLU_6] = USCARD("C6"), [DIA_6] = USCARD("D6"),
		[HEA_6] = USCARD("H6"), [SPA_6] = USCARD("S6"),
		[CLU_7] = USCARD("C7"), [DIA_7] = USCARD("D7"),
		[HEA_7] = USCARD("H7"), [SPA_7] = USCARD("S7"),
		[CLU_8] = USCARD("C8"), [DIA_8] = USCARD("D8"),
		[HEA_8] = USCARD("H8"), [SPA_8] = USCARD("S8"),
		[CLU_9] = USCARD("C9"), [DIA_9] = USCARD("D9"),
		[HEA_9] = USCARD("H9"), [SPA_9] = USCARD("S9"),
		[CLU_X] = USCARD("CX"), [DIA_X] = USCARD("DX"),
		[HEA_X] = USCARD("HX"), [SPA_X] = USCARD("SX"),
		[CLU_J] = USCARD("CJ"), [DIA_J] = USCARD("DJ"),
		[HEA_J] = USCARD("HJ"), [SPA_J] = USCARD("SJ"),
		[CLU_Q] = USCARD("CQ"), [DIA_Q] = USCARD("DQ"),
		[HEA_Q] = USCARD("HQ"), [SPA_Q] = USCARD("SQ"),
		[CLU_K] = USCARD("CK"), [DIA_K] = USCARD("DK"),
		[HEA_K] = USCARD("HK"), [SPA_K] = USCARD("SK"),
	},
	.facedown = (char*[]){"##"},
	.placeholder = (char*[]){"()"},
	.init_seq = NULL,
	.reset_seq = NULL,
};
#endif
