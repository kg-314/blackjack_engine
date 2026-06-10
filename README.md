# blackjack_engine

A blackjack game engine written in C, built to power large-scale strategy simulations — measuring how different playing strategies affect the house edge — as well as interactive front-ends. The engine handles all game logic: dealing, player actions, dealer auto-play, and bet resolution (including 3:2 payouts on naturals). It exposes a small, phase-gated API so any front-end (TUI, simulation harness, GUI, web bridge) can drive a game without knowing anything about the internals.

The project is split into two layers:

- **The engine** (`src/blackjack_engine.{c,h}`) — pure game logic, no I/O, no heap allocation. All state lives in a self-contained `GameState` struct owned by the caller, so many independent games can run side by side (e.g. one per simulation worker thread).
- **Front-ends** — a terminal interface (`src/blackjackTUI.c`, early skeleton) and a planned simulation harness for running millions of hands against scripted strategies.

## Design principles

The engine follows a few rules inspired by safety-critical C guidelines (NASA's Power of 10):

- **No dynamic allocation.** `GameState` embeds the card shoe and player array directly. The caller owns the storage — stack, heap, arena, or a reused per-worker slot — and the engine only initializes and mutates it in place. There is no `malloc`, `free`, or `exit()` anywhere in the engine.
- **Bounded everything.** All loops have fixed upper bounds (`MAX_HAND`, `MAX_SHOE`, `MAX_PLAYERS`); hand and shoe arrays are fixed-size.
- **Failure by return value.** Fallible operations (`engine_init`, `deal`, `apply_action`, `draw_card`, `engine_set_deck`) return `bool` and never tear down the process. A failed call performs no partial state mutation.
- **Phase-gated entry points.** A `Phase` enum (`PHASE_PLAYER_TURN`, `PHASE_DEALER_TURN`, `PHASE_PAYOUT`) guards every entry point, so a front-end can't take an action in the wrong state — illegal calls are rejected rather than corrupting the game.
- **Single entry point for player actions.** All player decisions go through `apply_action()` with an `Action` enum, keeping front-ends simple and making the strategy interface for simulations uniform.

ISO C compliant; the only dependencies are a C compiler and `make`. The per-instance xorshift64\* PRNG makes every game independently seedable and fully reproducible.

## Project layout

```
.
├── src/
│   ├── blackjack_engine.c   # Engine implementation
│   ├── blackjack_engine.h   # Public engine API
│   └── blackjackTUI.c       # Terminal front-end (early skeleton)
├── test/
│   ├── test_hand.c          # Hand-scoring unit tests
│   ├── test_engine.c        # End-to-end engine tests with deterministic decks
│   └── test_utils.h         # Lightweight ASSERT_EQ / ASSERT_TRUE macros
├── Makefile
└── README.md
```

## Building and running the tests

```bash
make           # builds and runs both test binaries
make clean     # removes test binaries
```

`make` produces two executables in `test/` and runs them. A successful run ends with `All hand tests passed.` and `All engine tests passed.`.

## How the engine works

### Card encoding

Cards are `uint8_t` values from 0 to 51. The rank is `(card % 13) + 1`, where 1 is an Ace, 2–10 are pip cards, and J/Q/K collapse to a value of 10. Suit is implicit in the encoding but unused by game logic — it only matters if a front-end wants to display the card.

### Hand scoring

`get_hand_value()` totals a hand with the standard ace rule: each ace starts at 11 and is demoted to 1 while the hand is busted and a demotion is still possible. This handles soft hands correctly (A + K = 21, A + A + A = 13).

### Game flow

A typical round, from a front-end's perspective:

1. `engine_init(&game, player_money, num_players, num_decks, seed)` — initialize caller-owned storage in place: build a shoe of 1–8 decks, seed the per-instance RNG, shuffle, and set up `num_players` players plus the dealer. The game starts in `PHASE_PAYOUT` ("between hands, ready to deal").
2. `deal(&game, initial_bet)` — start a round. Each player and the dealer receive two cards; the dealer's first card is recorded as the up-card. The game enters `PHASE_PLAYER_TURN`.
3. The front-end calls `apply_action(&game, action)` for the current player with `ACTION_HIT`, `ACTION_STAND`, or `ACTION_DOUBLE`. The engine advances `curr_player` automatically when a player busts, stands, or doubles; once every player has acted, the phase moves to `PHASE_DEALER_TURN`.
4. The front-end calls `resolve_dealer(&game)` — the dealer draws to 17 or higher, then all bets are settled directly on each player's `money` field. The game returns to `PHASE_PAYOUT`.
5. Call `deal()` again for the next round. There is nothing to destroy — the caller simply stops using (or reuses) the `GameState`.

Splitting player choice (`apply_action`) from the dealer's fixed, deterministic play (`resolve_dealer`) keeps the two concerns separate: front-ends decide *when* the dealer turn happens, but never *how* it plays out.

### Payout rules

`compute_win()` (internal, run by `resolve_dealer`) settles bets in this order:

- A player bust loses the bet, regardless of the dealer's hand.
- A **natural blackjack** — 21 on exactly the first two cards — pays **3:2**. A dealer natural beats any non-natural hand, including a multi-card 21; two naturals push.
- Otherwise: dealer bust or a higher player total wins even money, a lower total loses, equal totals push.

### Public API

From `blackjack_engine.h`:

```c
// Lifecycle: in-place init on caller-owned storage; no allocation, nothing to destroy
bool engine_init(struct GameState *game, int player_money, int num_players,
                 int num_decks, uint64_t seed);

// Game actions (phase-gated)
bool deal(struct GameState *game, int initial_bet);        // false if a hand could not start
bool apply_action(struct GameState *game, Action action);  // false if the action was illegal
void resolve_dealer(struct GameState *game);               // dealer plays out; bets settle

// Utilities (also used by the test suite)
bool draw_card(struct GameState *game, uint8_t *out_card); // false if no card available
void add_card(Hand *h, uint8_t card);
int  get_hand_value(Hand *h);
bool engine_set_deck(struct GameState *game, const uint8_t *deck, int size);
```

## Testing strategy

The test suite uses two approaches:

- **Hand tests** (`test_hand.c`) exercise `get_hand_value()` directly with crafted hands — simple totals, soft-ace adjustment, and multiple aces.
- **Engine tests** (`test_engine.c`) use `engine_set_deck()` to install a deterministic deck. In this test mode the engine reports deck exhaustion through `draw_card()`'s return value instead of reshuffling, so tests can assert exact outcomes — a player busting on a specific hit, the dealer landing on an exact total, or a natural paying out at precisely 3:2 — with zero flakiness.

`test_utils.h` provides three minimal macros — `ASSERT_EQ`, `ASSERT_TRUE`, `TEST_PASS` — that print a clear pass/fail line and exit on the first failure.

## Roadmap

The engine core (deal, hit/stand/double, dealer resolution, natural payouts, multi-deck shoes) is complete and tested. Next steps, in order:

1. **Simulation harness** (`sim/`) — a single-threaded runner that plays scripted strategies (starting with an always-stand baseline and full basic strategy) over millions of hands and reports the measured house edge against textbook values.
2. **Multithreading** — `pthread` fan-out across workers, each with an independent `GameState` and a per-worker seed derived from a master seed, so large runs stay fully reproducible.
3. **Split support** — `ACTION_SPLIT` is declared (commented out) in the `Action` enum; the engine change is contained and intentionally deferred until the harness exists to validate it.
4. **TUI** — `blackjackTUI.c` currently has a REPL loop and prompt, but command parsing isn't implemented yet. Completing it is deferred until the game logic (including split) is final.

Insurance, even money, and surrender are further out; `compute_win()` is structured so even-money settlement slots in cleanly when insurance is added.
