#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include "blackjack_engine.h"

// May be removed later, as this will be a user-inputed value.
#define NUM_DECKS 1 // For when this program is extended to allow for shoe with multiple decks

// struct Character *player;
// struct Character *dealer;
struct GameState *game;
uint8_t *cards;

static void create_deck(uint8_t **deck);
static void shuffle(uint8_t *cards, int num_cards);
void deal(int initial_bet);
// void bet(int amount);
int hit();
void stand();
static void changeTurn();
void double_down();
void buy_insurance();
void even_money();
// static bool checkBust();
static void computeWin();

// static void engine(uint8_t *cards, struct Player *player, struct Dealer *dealer, int numCards);

/* 
Currently main for testing, will be turning into setup(bool automate)
so that other programs can call to start a game
May call it load_engine later, or even start_engine() 
*/
int start_engine(int player_money, int num_players) {

    create_deck(&cards);
    if (cards == NULL) {
        /* Cards were not created successfully */
        fprintf(stderr, "An error occurred in deck creation\n");
        return -1;
    }

    num_players++; // Account for dealer.

    game = (struct GameState *) malloc(sizeof(struct GameState));
    game->curr_player = 0;
    game->num_players = num_players;
    game->deck_pos = 0;
    game->players = malloc(sizeof(struct Character)*num_players);

    // All players start with the same initial money (except dealer has infinite money).
    for (int i = 0; i < num_players; i++) {
        if (i == num_players - 1) {
            game->players[i].type = TYPE_D;
            game->players[i].data.d.hand.count = 0;
            game->players[i].data.d.show_card = 0;
        } else {
            game->players[i].type = TYPE_P;
            game->players[i].data.p.current_bet = 0;
            game->players[i].data.p.hand.count = 0;
            game->players[i].data.p.money = player_money;
        }
    }
    // Create player
    // player = (struct Player *) calloc(1, sizeof(struct Player));
    // player->money = player_money;

    // // Create dealer
    // dealer = (struct Dealer *) calloc(1, sizeof(struct Dealer));

    srand(time(NULL));
    int num_cards = DECK_SIZE * NUM_DECKS;
    shuffle(cards, num_cards);
    
    // engine(cards, player, dealer, numCards);

    return 0;
}

/*
Returns the player money at end of game.
Frees all memory that was used for game.
 */
int stop_engine() {
    if (cards != NULL) {
        free(cards);
    }

    // if (player != NULL) {
    //     free(player);
    // }

    // if (dealer != NULL) {
    //     free(dealer);
    // }

    if (game != NULL) {
        // TODO: Need to free the character structs.
        free(game);
    }
    return 0;
}

int get_hand_value(Hand *h) {
    int total = 0;
    int aces = 0;

    for (int i = 0; i < h->count; i++) {
        int value = (h->cards[i] % 13) + 1;

        if (value > 10) value = 10;

        if (value == 1) {
            aces++;
            total += 11; // assume ace = 11 first
        } else {
            total += value;
        }
    }

    // downgrade aces from 11 → 1 if bust
    while (total > 21 && aces > 0) {
        total -= 10;
        aces--;
    }

    return total;
}

// dealer stands on 17.
/*
This function will control who's turn it is.
It will also act to automate the dealer's turn because 
dealer always has to stand on 17. Thus, the UI does not 
need to control dealer's turn.
*/
static void changeTurn() {
    game->curr_player++;

    // If all players finished → dealer turn
    if (game->curr_player == game->num_players - 1) {
        struct Character *dealer = &game->players[game->curr_player];

        while (get_hand_value(&dealer->data.d.hand) < 17) {
            add_card(&dealer->data.d.hand, draw_card());
        }

        computeWin();
    }
}

/* 
Every time a hit occurs, it should be checked 
if the hit taker busted. 
*/
// static bool checkBust() {

// }
// hit() now does checkBust() function.

/*
At the end of each hand (when the dealer stands or busts),
it must be computer who won and they must be paid.
*/
static void computeWin() {

}

// The main function that drives the game.
// Engine should evaluate an input from the user interface,
// and take the associated action.
// static void engine(uint8_t *cards, struct Player *player, struct Dealer *dealer, int numCards) {
//     // Deck will be shuffled whenever at least 31 cards have been used (single deck).

//     // Some basic functions are needed for the operation of blackjack.
//     // dealCard() -- Deals a card to the active player
//     // changeTurns() -- changes the active player (switch between player and dealer)
//     // checkBust() -- check if player/dealer busted from card dealt.
//     // payout() -- pay player if they win
//     // bet() -- how much money you want to bet before a round starts
//     // startRound() and finishRound()
//     // doubleDown() -- triggers everything that follows from doubling down
//     // buyInsurance()
//     // evenMoney()

//     // Player called functions:
//     // hit()
//     // bet()
//     // doubleDown()
//     // buyInsurance()
//     // evenMoney()
//     // quit()

//     // Step 1: Query the player for how much they want to bet.
//     // Engine needs to send some kind of askBet signal to user interface.
//     for (int i = 0; i < DECK_SIZE; i++) {
//         printf("%d ", cards[i]);
//     }
//     printf("%s", "\n");
//     shuffle(cards, numCards);

//     for (int i = 0; i < DECK_SIZE; i++) {
//         printf("%d ", cards[i]);

//     }

//     // Game should end if player reaches 0 money or player quits.
// }

/* 
Creates a deck of cards.
Future functionality will be a shoe with multiple decks. 
*/
static void create_deck(uint8_t **deck) {
    // May want to just malloc instead of calloc since deck values are immediately set.
    *deck = calloc(DECK_SIZE, sizeof(uint8_t));
    // *deck = malloc(sizeof(uint8_t)*DECK_SIZE);

    for (int i = 0; i < DECK_SIZE; i++) {
        (*deck)[i] = i & 0xFF;
    }
}

/* 
Shuffle deck into a random order of cards. 
Used at beginning of engine and whenever a threshold 
of the number of cards have been played. 
*/
static void shuffle(uint8_t *cards, int num_cards) {
    // Fisher-Yates Algo for shuffling deck.
    if (num_cards > 1) { // Should always be true.
        for (int i = num_cards - 1; i > 0; i--) {
            int j = rand() % (i + 1);

            uint8_t temp_card = cards[i];
            cards[i] = cards[j];
            cards[j] = temp_card;
        }
    }
}

/*
This should begin a new hand.
Player will input their initial bet for the hand.
For simplicity, all players have same initial bet in this current version.
*/
void deal(int initial_bet) {
    game->curr_player = 0;

    // Reset all hands
    for (int i = 0; i < game->num_players; i++) {
        if (game->players[i].type == TYPE_P) {
            game->players[i].data.p.hand.count = 0;
            game->players[i].data.p.is_active = true;
            game->players[i].data.p.current_bet = initial_bet;
        } else {
            game->players[i].data.d.hand.count = 0;
        }
    }

    // Deal 2 rounds
    for (int r = 0; r < 2; r++) {
        for (int i = 0; i < game->num_players; i++) {
            uint8_t card = draw_card();

            if (game->players[i].type == TYPE_P) {
                add_card(&game->players[i].data.p.hand, card);
            } else {
                add_card(&game->players[i].data.d.hand, card);
                if (r == 0) {
                    game->players[i].data.d.show_card = card;
                }
            }
        }
    }

    game->curr_player = 0;
}

// void bet(int amount) {

// }

/*
Helper function that draws a card from the deck and reshuffles if deck (shoe) is empty.
*/
uint8_t draw_card() {
    if (game->deck_pos >= DECK_SIZE * NUM_DECKS) {
        shuffle(cards, DECK_SIZE * NUM_DECKS);
        game->deck_pos = 0;
    }
    return cards[game->deck_pos++];
}

/*
Helper function to add a card to someone's hand.
*/
void add_card(Hand *h, uint8_t card) {
    if (h->count < MAX_HAND) {
        h->cards[h->count++] = card;
    }
}

/*
Add next card in deck to current player's hand.
Change turns if player busted.
Return the dealt card.
*/
int hit() {
    struct Character *curr = &game->players[game->curr_player];

    uint8_t card = draw_card();

    if (curr->type == TYPE_P) {
        add_card(&curr->data.p.hand, card);

        if (get_hand_value(&curr->data.p.hand) > 21) {
            curr->data.p.is_active = false;
            changeTurn();
        }
    } else {
        add_card(&curr->data.d.hand, card);
    }

    return card;
}

void stand() {
    changeTurn();
}

void double_down() {

}

void buy_insurance() {

}

void even_money() {

}