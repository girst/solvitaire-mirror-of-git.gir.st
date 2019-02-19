# solVItaire

Play klondike and spider solitaire in your unicode terminal.

Supports `vi` style keybindings (hjkl), cursor keys and the mouse. Press `J` to
'join' a well-suited pile to the cursor. `Space` locks in a selection, and
`Return` pulls cards from the stack in spider solitaire. Piles can also be
*directly addressed* by the number below them (or 8, 9, 0 for stack, waste and
foundation in klondike).    
See `$PROGNAME -h` for details.

## Details

For details and screenshots see the project homepage at
**https://gir.st/sol.htm**

## Makefile Options

By default the character set uses an `X` to for the 10 card. Compile with `make
DECIMAL_TEN=1` to display the letters `10` instead.

## License

You can use, study, share and improve this software under the terms of the GNU
General Public License, version 3. Copyright 2019 Tobias Girstmair. For
licensing details, see `LICENSE`.

<details><summary>
<h2>TODO</h2>
</summary>

### P1
<!-- * none! \o/-->
 * TODO FREECELL: autodetect possible moves (card selector choice dialog)
### P2
 * TODO FREECELL: join()
 * TODO: klondike/freecell: auto-move to foundation (optional)
 * TODO: suggest moves (`h` => hint)
 * TODO: cleanup: in `x2y()` functions there is a lot of replication and noise
         when calculating legal moves, top cards, etc. 
### P3
 * TODO: visbell changes: instead of screen, flash active {cursor,card} {red,twice}?
 * TODO: differential drawing mode (at least for highlighting cards)
 * TODO: `.` command (repeat last action)
### P4
 * TODO: ellipsis overlap: if more than three cards in a row overlap, replace
         middle ones with an ellipsis
 * TODO: mouse mode improvements:
          - spider/freecell: detect how many cards to move to empty pile
            (must find exact position despite `extreme_overlap`)
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
 * DONE: make portable (clang on FreeBSD)
         makefile's `$<` refuses to work if prequisite name != target name
         Fix: 
             ln -s sol.c. spider.c
             sed -i 's/^spider: sol.c/spider: spider.c/' Makefile
 * DONE: escape sequences (for real cursor keys)
 * DONE: inverted moves (if A->B doesn't work, try B->A)
 * DONE:  - allow dragging the mouse
 * DONE: use `10` instead of `X` on card face (/u/Valley6660)
 * DONE: `:help` to show keyhelp (/u/Valley6660)
 * DONE: don't show cursor-highlight when inputting directly or with the mouse (/u/TooEarlyForMe)
 * DONE: remove sysmlink trickery and just call sol.c explicitly
 * DONE FREECELL: t2t() multi card move and accompanying pop_undo()
 * DONE:   - mouse (term2pile() cell encoding, set_mouse(), wait_mouse_up())
 * DONE FREECELL: don't allow selecting more cards than movable (`DO_HI, TOP_HI, cursor_{up,down}`)
 * DONE: differentiate foundation placeholder from cells ph. (freecell)
</details>

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
