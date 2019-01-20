# solVIItaire

play klondike and spider solitaire in your unicode terminal.

## TODO

### P1
 * TODO: make portable (clang on FreeBSD)
         makefile's `$<` refuses to work if prequisite name != target name
### P2
 * TODO: suggest moves (`h` => hint)
 * TODO: escape sequences (for real cursor keys)
 * TODO: inverted moves (if A->B doesn't work, try B->A)
 * TODO: cleanup: in `x2y()` functions there is a lot of replication and noise
         when calculating legal moves, top cards, etc. 
### P3
 * TODO: differential drawing mode (at least for highlighting cards)
### P4
 * TODO: mouse mode (problems: finding position with `extreme_overlap`)
 * TODO: scores, variants: draw 3, max. n overturns
 * TODO: vt220 mode (problems: charset, slow baudrate and no differential drawing mode)
 * TODO: ed(1) mode (solEDaire): playable on a line printer; ascii/ibm only?

### DONE
 * DONE: use `#ifdef`s to differentiate games (sol, spider, ed-sol, ed-spider)
 * DONE: keyboard alias: twice same key == waste/pile -> foundation
 * DONE: spider keyboard: `<from><to>` stacks; 1-9,0=tableu, return=draw
 * DONE: spider: easy/medium difficulty: only deal 1/2 suits instead of 4 -> deal()
 * DONE: patience: allow taking from 0(foundation)
 * DONE: highlight `from` pile, so users can see at what input stage they are
 * DONE: make piles 0-indexed in klondike as well
 * DONE: duplicate card ♠A found in tableu: added check at start to monitor this
         Cannot reproduce, removed check
 * DONE: bugs in spider's t2t()
         * once segfaulted on large column
         * sometimes doesn't work (ERR when it should be OK)
         Cannot reproduce
 * DONE: some input functions are in `x2y()` -- move them to `get_cmd()` (req.
         for mouse, hjkl modes)
 * DONE: sigint, sigcont handler! atexit() to avoid inconsistent term state
 * DONE: hjkl keyboard mode
 * DONE: more vim mode keys (first/last tableu)
 * DONE: online (key-)help `?`, `-h`, `-v` (NOTE: implemented -h, rest deemed
         not usedul)
 * DONE: extreme overlapping: if we are printing a sequence or multiple face down
	 cards, only print `.overlap` lines of the ends, and `1` line for the
         middle cards
 * DONE: in direct addressing 'up to' dialog: make return/space select the lowest/highest option
 * DONE: undo:
	 - insert append_undo() in x2y() functions
	 - to encode stack position we need to overload `f.u.n` as index.
	   (similar for foundation: overload `f.u.n` as foundation id)
	 - w2f(): have to encode both stock index and foundation id
	          don't want to add a struct field for this one case; 
	          for the time being (i.e. forever) I'm ORing waste index and
	          (foundation id<<16). ugly but works :/
	 - turning over cards: this needs to be encoded, because the card might 
	   be consecutive and there's no way to tell what its previous state was. 
	    * indicate that a card was turned (can be at most 1) by negating u.n
	      pros: no wasted space (it's just a boolean value), negation pattern 
	            already used for closed cards
	      cons: dirty C hack, less obvious than in other places, no need to
	            conserve memory this tightly
 * DONE: screen size > stack height => rendering issues!
         as good as it's going to get, since we can't just not draw cards at all
 * DONE: 'join to here' command (should take longest chain available)
 * DONE: expose deal() seed for restarting the game

## Notes

 - terminology:
```
    {stock}[waste]    [4*foundation]

    []   {}   {}   {}   {}   {}   {}
         []  (tableu piles)  {}   {}
              []   {}   {}   {}   {}
                   []   {}   {}   {}
                        []   {}   {}
                             []   {}
                                  []
```
 - data structures:
    - enum for each card (e.g. `SPADES_ACE`, `HEARTS_10`)
    - each pile is an array holding (13 open cards + up to 6 closed)    
      [0] is the "northmost"/bottom-most card; unoccupied are NULL/NO_CARD
    - a single card is represented in the 'cards' enum; if it is closed, it is negated.    
    - the foundation are 4 arrays of size 13
    - the stock pile is an array holding n cards and an index to the one to display
      when removing, decrement stack size and move all cards above index 1 over
    - previous states array: where to move which cards to get back to the state before    
    - undo:    
      double-linked list (follow `.prev` to undo, `.next` to redo)    
      "N cards were moved from X to Y" (do Y->X to undo)    
      allows jumping forwards in time as well (by repeating X->Y)    
      warn: when appending state, must check if `.next` was non-NULL and free rest of chain if so.
 - multiple card sizes: schemes.h will store cards like below. if we want to draw a card 
   that has one or more other cards below it, we only draw the first `.overlap` lines, 
   otherwise if it is the last one, we draw the whole one.
   this will give a look like in `~/solitaire-tests`
```
    .grid=7, /*vertical alignment*/
    .overlap=2,
    .cards = [
       ["╭───╮",
        "│♠ X│",
        "│ ♠ │",
        "╰───╯",
          NULL],
    ]
    /*or:*/
    .grid=2,
    .overlap=1,
    .cards = [
      [ "🃖 ", NULL ],
    ]
```
"open": face up card
"closed": face down card
