# solVIItaire

play klondike and spider solitaire in your unicode terminal.

## TODO

 * TODO: make piles 0-indexed in klondike as well
 * TODO: sigcont handler!
 * TODO: extreme overlapping: if we are printing a sequence or multiple face down
	 cards, only print `.overlap` lines of the ends, and `1` line for the
         middle cards
 * TODO: cleanup: in `x2y()` functions there is a lot of replication and noise
         when calculating legal moves, top cards, etc. 
 * TODO: hjkl keyboard mode
 * TODO: mouse mode
 * TODO: some input functions are in `x2y()` -- move them to `get_cmd()` (req.
         for mouse, hjkl modes)
 * TODO: highlight `from` pile, so users can see at what input stage they are
 * TODO: scores, variants: draw 3, max. n overturns
 * TODO: screen size > stack height => rendering issues!
 * TODO: undo:
         CURRENT DS FOR STACK/WASTE ("move over") IS INCOMPATIBLE WITH UNDO!!!!
         keeping taken card slots empty allows them to be reinserted by undo()
         also need field where to put it back into stack
 * TODO: cancel command mode with `^C` (SIGINT handler)
 * TODO: online (key-)help `?`, `-h`, `-v`
 * TODO: suggest moves (`h` => hint)
 * TODO: vt220 mode
 * TODO: ed(1) mode (solEDaire): playable on a line printer; ascii/ibm only?

 * DONE: use `#ifdef`s to differentiate games (sol, spider, ed-sol, ed-spider)
 * DONE: keyboard alias: twice same key == waste/pile -> foundation
 * DONE: spider keyboard: `<from><to>` stacks; 1-9,0=tableu, return=draw
 * DONE: spider: easy/medium difficulty: only deal 1/2 suits instead of 4 -> deal()
 * DONE: patience: allow taking from 0(foundation)

## BUGS

 + TODO: duplicate card ♠A found in tableu: added check at start to monitor this

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
