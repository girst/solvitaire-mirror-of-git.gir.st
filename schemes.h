#ifndef __SCHEMES_H__
#define __SCHEMES_H__

#include "sol.h"

struct scheme {
	int width; /* column alignment */
	int height; /* height of a card */
	int overlap; /* no of lines to draw when cards overlapp */
	char** card[_NUM_CARDS_internal];
	char** facedown;
	char** placeholder;
	char** foundation;
	char** halfstack; /* left half of stack (for spider) */
	int* halfcard; /* byte offset of right half of cards (for spider) */
	int halfwidth[3]; /* printed width of halfstack([0]) and halfcard([1]),
	                     byte width of halfcard([2]) */
};

#define p "\017" /*padding, [RB]?CARD must have same length; alternative: \177*/
#define ULCARD(s, r) (char*[]) \
{"╭───╮",\
 "│"s" "r"│",\
 "│ "s" │",\
 "╰───╯"}
#define RULCARD(s, r) \
	ULCARD("\033[91m" s "\033[39m", r)
#define BULCARD(s, r) \
	ULCARD(p "\033[2m" s "\033[22m", r)//WARN: requires hack in print_hi()
#define USCARD(c) (char*[]){c}
#define RUSCARD(c) (char*[]){"\033[91m" c "\033[39m "}
#define BUSCARD(c) (char*[]){c p p p p p p p p p p " "}

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
#ifdef DECIMAL_TEN
		[CLU_X] = ULCARD("♣","\b10"), [DIA_X] = ULCARD("♦","\b10"),
		[HEA_X] = ULCARD("♥","\b10"), [SPA_X] = ULCARD("♠","\b10"),
#else
		[CLU_X] = ULCARD("♣","X"), [DIA_X] = ULCARD("♦","X"),
		[HEA_X] = ULCARD("♥","X"), [SPA_X] = ULCARD("♠","X"),
#endif
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
	.foundation = (char*[]){
		"╭╌╌╌╮",
		"╎⢠⠒⡄╎",
		"╎⠸⠉⠇╎",
		"╰╌╌╌╯"
	},
	.halfstack = (char*[]) {
		"╭─",
		"│▚",
		"│▚",
		"╰─"
	},
	.halfcard = (int[]){6, 6, 4, 6},
	.halfwidth = {2, 3, 999},
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
#ifdef DECIMAL_TEN
		[CLU_X] = BULCARD("♣","\b10"), [DIA_X] = RULCARD("♦","\b10"),
		[HEA_X] = RULCARD("♥","\b10"), [SPA_X] = BULCARD("♠","\b10"),
#else
		[CLU_X] = BULCARD("♣","X"), [DIA_X] = RULCARD("♦","X"),
		[HEA_X] = RULCARD("♥","X"), [SPA_X] = BULCARD("♠","X"),
#endif
		[CLU_J] = BULCARD("♣","J"), [DIA_J] = RULCARD("♦","J"),
		[HEA_J] = RULCARD("♥","J"), [SPA_J] = BULCARD("♠","J"),
		[CLU_Q] = BULCARD("♣","Q"), [DIA_Q] = RULCARD("♦","Q"),
		[HEA_Q] = RULCARD("♥","Q"), [SPA_Q] = BULCARD("♠","Q"),
		[CLU_K] = BULCARD("♣","K"), [DIA_K] = RULCARD("♦","K"),
		[HEA_K] = RULCARD("♥","K"), [SPA_K] = BULCARD("♠","K"),
	},
	.facedown = (char*[]){
		"╭───╮",
		"│\033[94m▚▚▚\033[39m│",
		"│\033[94m▚▚▚\033[39m│",
		"╰───╯"
	},
	.placeholder = (char*[]){
		"╭╌╌╌╮",
		"╎   ╎",
		"╎   ╎",
		"╰╌╌╌╯"
	},
	.foundation = (char*[]){
		"╭╌╌╌╮",
		"╎\033[2m⢠⠒⡄\033[22m╎",
		"╎\033[2m⠸⠉⠇\033[22m╎",
		"╰╌╌╌╯"
	},
	.halfstack = (char*[]) {
		"╭─",
		"│\033[94m▚\033[39m",
		"│\033[94m▚\033[39m",
		"╰─"
	},
	.halfcard = (int[]){6, 16, 4, 6},
	.halfwidth = {2, 3, 999},
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
	.foundation = (char*[]){"⟨⟩"},
	.halfstack = (char*[]){"🂠"},
	.halfcard = (int[]){0},
	.halfwidth = {1, 1, 4},
};
const struct scheme unicode_small_color = {
	.width = 2,
	.height = 1,
	.overlap = 1,
	.card = {
		[NO_CARD] = (char*[]){"  "},
		[CLU_A] = BUSCARD("🃑"), [DIA_A] = RUSCARD("🃁"),
		[HEA_A] = RUSCARD("🂱"), [SPA_A] = BUSCARD("🂡"),
		[CLU_2] = BUSCARD("🃒"), [DIA_2] = RUSCARD("🃂"),
		[HEA_2] = RUSCARD("🂲"), [SPA_2] = BUSCARD("🂢"),
		[CLU_3] = BUSCARD("🃓"), [DIA_3] = RUSCARD("🃃"),
		[HEA_3] = RUSCARD("🂳"), [SPA_3] = BUSCARD("🂣"),
		[CLU_4] = BUSCARD("🃔"), [DIA_4] = RUSCARD("🃄"),
		[HEA_4] = RUSCARD("🂴"), [SPA_4] = BUSCARD("🂤"),
		[CLU_5] = BUSCARD("🃕"), [DIA_5] = RUSCARD("🃅"),
		[HEA_5] = RUSCARD("🂵"), [SPA_5] = BUSCARD("🂥"),
		[CLU_6] = BUSCARD("🃖"), [DIA_6] = RUSCARD("🃆"),
		[HEA_6] = RUSCARD("🂶"), [SPA_6] = BUSCARD("🂦"),
		[CLU_7] = BUSCARD("🃗"), [DIA_7] = RUSCARD("🃇"),
		[HEA_7] = RUSCARD("🂷"), [SPA_7] = BUSCARD("🂧"),
		[CLU_8] = BUSCARD("🃘"), [DIA_8] = RUSCARD("🃈"),
		[HEA_8] = RUSCARD("🂸"), [SPA_8] = BUSCARD("🂨"),
		[CLU_9] = BUSCARD("🃙"), [DIA_9] = RUSCARD("🃉"),
		[HEA_9] = RUSCARD("🂹"), [SPA_9] = BUSCARD("🂩"),
		[CLU_X] = BUSCARD("🃚"), [DIA_X] = RUSCARD("🃊"),
		[HEA_X] = RUSCARD("🂺"), [SPA_X] = BUSCARD("🂪"),
		[CLU_J] = BUSCARD("🃛"), [DIA_J] = RUSCARD("🃋"),
		[HEA_J] = RUSCARD("🂻"), [SPA_J] = BUSCARD("🂫"),
		[CLU_Q] = BUSCARD("🃝"), [DIA_Q] = RUSCARD("🃍"),
		[HEA_Q] = RUSCARD("🂽"), [SPA_Q] = BUSCARD("🂭"),
		[CLU_K] = BUSCARD("🃞"), [DIA_K] = RUSCARD("🃎"),
		[HEA_K] = RUSCARD("🂾"), [SPA_K] = BUSCARD("🂮"),
	},
	.facedown = (char*[]){"🂠 "},
	.placeholder = (char*[]){"❲❳"},
	.foundation = (char*[]){"⟨⟩"},
	.halfstack = (char*[]){"🂠"},
	.halfcard = (int[]){0},
	.halfwidth = {1, 1, 14},
};
#undef p
#endif
