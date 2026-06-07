#ifndef BLACKJACK_ENGINE_H
#define BLACKJACK_ENGINE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#define DECK_SIZE 52
#define MAX_DECKS 8
#define MAX_SHOE (MAX_DECKS * DECK_SIZE)  // 416: upper bound for the shoe
#define MAX_PLAYERS 7                     // table cap (dealer is separate)
#define MAX_HAND 22                       // Worst case (all aces) for an 8 deck shoe plus one extra to allow for bust

/* Phases */
typedef enum {
    PHASE_PLAYER_TURN,   // waiting for apply_action() on curr_player
    PHASE_DEALER_TURN,   // all players done; waiting for resolve_dealer()
    PHASE_PAYOUT,        // initial state or bets settled; ready for next deal()
} Phase;

/* Player Actions */
typedef enum {
    ACTION_HIT,
    ACTION_STAND,
    ACTION_DOUBLE,
    // ACTION_SPLIT, ACTION_SURRENDER, ACTION_EVEN_MONEY later
} Action;

/* Data type for player / dealer current hand and card count. */
typedef struct Hand {
    uint8_t cards[MAX_HAND];
    uint8_t count;
} Hand;

/* Data type for player data. */
typedef struct Player {
    int money;
    int current_bet;
    Hand hand;
} PlayerData;

/* Data type for dealer data. */
typedef struct Dealer {
    Hand hand;
    uint8_t show_card;
} DealerData;

/*
Self-contained game state: no internal pointers, no hidden allocation.
The caller owns the storage (stack, heap, arena, per-worker slot) and the
engine only ever initializes and mutates it in place.

Note: The element ordering is to minimize padding.
*/
struct GameState {
    uint64_t rng_state;              // per-instance RNG; must be non-zero

    int curr_player;                 // Whose turn it is at the table
    int num_players;                 // Active players (NOT including dealer); 1..MAX_PLAYERS
    int deck_pos;                    // Current read position in the shoe
    int deck_size;                   // Active cards in the shoe (DECK_SIZE * num_decks for a live game, or test deck size)
    
    Phase phase;                     // Current phase; gates which entry points are valid
    PlayerData players[MAX_PLAYERS]; // Embedded; only [0, num_players) are active
    DealerData dealer;               // Dealer is separate from players, one per game instance
    uint8_t cards[MAX_SHOE];         // Embedded shoe
    bool test_mode;                  // Deterministic deck: do not reshuffle, signal exhaustion instead
};

/*
In-place initialization. The caller owns the storage for `game` (stack, heap,
arena, or a reused per-worker slot) and is responsible for its lifetime; the
engine performs no allocation of its own. Returns false on invalid arguments
(NULL game, num_players or num_decks out of range).
*/
bool engine_init(struct GameState *game, int player_money, int num_players, int num_decks, uint64_t seed);

// PLayer card actions are controlled through apply_action()
bool deal(struct GameState *game, int initial_bet);     // false if a hand could not be dealt
bool apply_action(struct GameState *game, Action action);
void resolve_dealer(struct GameState *game);

// // Utilities (also used by tests)
bool draw_card(struct GameState *game, uint8_t *out_card); // false if no card available
void add_card(Hand *h, uint8_t card);
int get_hand_value(Hand *h);
bool engine_set_deck(struct GameState *game, const uint8_t *deck, int size); // false on bad args

#endif // BLACKJACK_ENGINE_H