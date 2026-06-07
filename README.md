# blackjack_engine

A blackjack game engine written in C. The engine handles all game logic — dealing, hitting, standing, doubling down, dealer auto-play, win resolution — and exposes a clean API so that any front-end (TUI, GUI, web bridge, simulation harness) can drive the game without knowing anything about the internals.

The project is split into two layers:

- **The engine** (`src/blackjack_engine.{c,h}`) — pure game logic, no I/O. Multiple independent games can run side-by-side because all state lives in a `GameState` struct that the caller creates and destroys.
- **A front-end** (`src/blackjackTUI.c`) — a terminal interface that talks to the engine. This is a work in progress.

## Why a separate engine?

Keeping game logic isolated from presentation means the same engine can power a CLI, a graphical client, an automated card-counting simulator, or a network service — without re-implementing the rules each time. It also makes the rules easy to test in isolation, which is what the `test/` directory does.

## Project layout

```
.
├── src/
│   ├── blackjack_engine.c   # Engine implementation
│   ├── blackjack_engine.h   # Public engine API
│   └── blackjackTUI.c       # Terminal front-end (in progress)
├── test/
│   ├── test_hand.c          # Hand-scoring unit tests
│   ├── test_engine.c        # End-to-end engine tests with deterministic decks
│   └── test_utils.h         # Lightweight ASSERT_EQ / ASSERT_TRUE macros
├── Makefile
└── README.md
```

## Building and running the tests

The engine has no external dependencies — just a C compiler and `make`.

```bash
make           # builds and runs both test binaries
make clean     # removes test binaries
```

`make test` produces two executables in `test/` and runs them. A successful run ends with `All hand tests passed.` and `All engine tests passed.`.

## How the engine works

### Card encoding

Cards are stored as `uint8_t` values from 0 to 51. The rank is derived as `(card % 13) + 1`, where 1 is an Ace, 2–10 are pip cards, and 11/12/13 collapse to a value of 10 (J/Q/K). Suit is encoded implicitly in the division by 13, but the engine doesn't use suit for any game logic — it only matters if a front-end wants to display the card.

### Hand scoring

`get_hand_value()` totals a hand using the standard blackjack rule for aces: each ace starts as 11, and any ace is downgraded to 1 as long as the hand is busted and a downgrade is still possible. This handles soft hands correctly (e.g. A + K = 21, A + A + A = 13).

### Game flow

A typical round, from a front-end's perspective:

1. `engine_create(player_money, num_players)` — allocate a game, shuffle a fresh deck, set up `num_players` players plus a dealer.
2. `deal(game, initial_bet)` — start a new round. Each player and the dealer get two cards; the dealer's first card is recorded as their up-card.
3. The front-end calls `hit()`, `stand()`, or `double_down()` for the current player. The engine advances `curr_player` automatically when a player busts, stands, or doubles.
4. Once all human players have acted, the engine takes over the dealer turn: it draws until the dealer hits 17 or higher, then resolves all bets (`computeWin()` internally). Payouts are settled directly on each player's `money` field — push on ties, win or lose the current bet otherwise.
5. Call `deal()` again to start the next round, or `engine_destroy(game)` to release memory.

The dealer's turn is intentionally driven by the engine rather than the front-end, since the rule "dealer stands on 17" is fixed and removing it from the UI's responsibility keeps front-ends simple.

### Public API

From `blackjack_engine.h`:

```c
// Lifecycle
struct GameState *engine_create(int player_money, int num_players);
void engine_destroy(struct GameState *game);

// Game actions
void deal(struct GameState *game, int initial_bet);
int  hit(struct GameState *game);          // returns the drawn card
void stand(struct GameState *game);
void double_down(struct GameState *game);

// Utilities (also used by the test suite)
uint8_t draw_card(struct GameState *game);
void    add_card(Hand *h, uint8_t card);
int     get_hand_value(Hand *h);
void    engine_set_deck(struct GameState *game, uint8_t *deck, int size);
void    test_dealer_draw(struct GameState *game);
```

## Testing strategy

The test suite uses two approaches:

- **Hand tests** (`test_hand.c`) exercise `get_hand_value()` directly with crafted hands — simple totals, soft-ace adjustment, and multiple aces.
- **Engine tests** (`test_engine.c`) use `engine_set_deck()` to swap the shuffled deck for a deterministic one. This lets tests assert exact outcomes — that a player busts on a specific hit, or that the dealer ends on exactly 20 — without flakiness from randomness. `test_mode` also disables the dealer's automatic draw at end-of-turn so the test can call `test_dealer_draw()` itself and avoid the engine drawing past the end of the rigged deck.

The `test_utils.h` header provides three minimal macros — `ASSERT_EQ`, `ASSERT_TRUE`, `TEST_PASS` — that print a clear pass/fail line and exit on the first failure.

## Current limitations and roadmap

The engine currently supports a single deck and a single seat per player. The following are scaffolded but not yet implemented:

- **Insurance** (`buy_insurance`) and **even money** (`even_money`) — stub functions only.
- **Split** — not yet present.
- **Multi-deck shoe** — `NUM_DECKS` is defined but `create_deck` fills a single 52-card deck; extending this is a planned change.
- **TUI** — `blackjackTUI.c` has a REPL loop and prompt but `eval()` is empty, so commands aren't parsed yet. The intended interface is `start <INITIAL_MONEY>` followed by per-turn commands.
