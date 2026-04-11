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

// character values
// struct Player {
//     int money;
//     int current_bet;
//     int hand; // Hand just an int because suit does not really matter.
// };

// struct Dealer {
//     int show_card;
//     int hand;
// };

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
    struct Character *players;
};

// struct GameState {
//     int currentBet;
//     struct Player *player;
//     struct Dealer *dealer;
// };

// Global variables that should be accessible by any user interface.
extern struct Player *player;
extern struct Dealer *dealer;

// Prototypes
int start_engine(int player_money, int num_players);
int stop_engine();
void deal(int initial_bet);
int hit();
void stand();
void double_down();
void buy_insurance();
void even_money();

// Prototypes for testing
uint8_t draw_card();
void add_card(Hand *h, uint8_t card);
int get_hand_value(Hand *h);

#endif // BLACKJACK_ENGINE_H