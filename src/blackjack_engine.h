#ifndef BLACKJACK_ENGINE_H
#define BLACKJACK_ENGINE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#define DECK_SIZE 52
#define MAX_HAND 12  // worst case (all aces)

// Add near the top, alongside the other typedefs

typedef enum {
    PHASE_PLAYER_TURN,   // waiting for apply_action() on curr_player
    PHASE_DEALER_TURN,   // all players done; waiting for resolve_dealer()
    PHASE_PAYOUT,        // initial state or bets settled; ready for next deal()
} Phase;

typedef enum {
    ACTION_HIT,
    ACTION_STAND,
    ACTION_DOUBLE,
    // ACTION_SPLIT, ACTION_SURRENDER later
} Action;

typedef struct Hand {
    uint8_t cards[MAX_HAND];
    int count;
} Hand;

typedef struct Player {
    Hand hand;
    int money;
    int current_bet;
    //bool is_active;   // for bust/stand
} PlayerData;

typedef struct Dealer {
    Hand hand;
    uint8_t show_card;
} DealerData;

enum Type {
    TYPE_P,
    TYPE_D
};

struct Character {
    enum Type type;
    union {
        PlayerData p;
        DealerData d;
    } data;
};

/*
All changes to player states will be controlled through the game state.
*/
struct GameState {
    int curr_player;            // Who's turn it currently is at the table
    int num_players;            // How many players are at the table (including dealer)
    int deck_pos;               // The position in the deck
    uint8_t *cards;             // The deck of cards
    struct Character *players;  // Pointers to the players
    bool test_mode;             // Makes sure a deterministic test ends after cards run out
    int test_size;              // The number of cards being tested
    uint64_t rng_state;         // per-instance RNG; must be non-zero
    Phase phase;                // The current phase of game, determines what must take place
};

// "Constructor / Destructor" (Ability to run multiple games at once)
struct GameState *engine_create(int player_money, int num_players, uint64_t seed);
void engine_destroy(struct GameState *game);

// PLayer card actions are controlled through apply_action()
void deal(struct GameState *game, int initial_bet);
bool apply_action(struct GameState *game, Action action);
void resolve_dealer(struct GameState *game);
//int hit(struct GameState *game);
//void stand(struct GameState *game);
//void double_down(struct GameState *game);

// Utilities (keep for testing)
uint8_t draw_card(struct GameState *game);
void add_card(Hand *h, uint8_t card);
int get_hand_value(Hand *h);
void engine_set_deck(struct GameState *game, uint8_t *deck, int size);

#endif // BLACKJACK_ENGINE_H