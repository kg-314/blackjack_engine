#ifndef BLACKJACK_ENGINE_H
#define BLACKJACK_ENGINE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#define DECK_SIZE 52
#define MAX_HAND 12  // worst case (all aces)

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
    int curr_player;
    int num_players;
    int deck_pos;
    uint8_t *cards;
    struct Character *players;
    bool test_mode;
    int test_size;
};

// "Constructor / Destructor" (Ability to run multiple games at once)
struct GameState *engine_create(int player_money, int num_players);
void engine_destroy(struct GameState *game);

// Game actions
void deal(struct GameState *game, int initial_bet);
int hit(struct GameState *game);
void stand(struct GameState *game);
void double_down(struct GameState *game);

// Utilities (keep for testing)
uint8_t draw_card(struct GameState *game);
void add_card(Hand *h, uint8_t card);
int get_hand_value(Hand *h);
void engine_set_deck(struct GameState *game, uint8_t *deck, int size);
void test_dealer_draw(struct GameState *game);

#endif // BLACKJACK_ENGINE_H