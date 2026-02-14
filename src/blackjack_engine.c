#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include "blackjack_engine.h"

#define DECK_SIZE 52
// May be removed later, as this will be a user-inputed value.
#define NUM_DECKS 1 // For when this program is extended to allow for shoe with multiple decks

struct Player *player;
struct Dealer *dealer;
uint8_t *cards;

static void create_deck(uint8_t **deck);
static void shuffle(uint8_t *cards, int num_cards);
void deal(int initial_bet);
void bet(int amount);
void hit();
void stand();
void double_down();
void buy_insurance();
void even_money();

// static void engine(uint8_t *cards, struct Player *player, struct Dealer *dealer, int numCards);

/* Currently main for testing, will be turning into setup(bool automate)
so that other programs can call to start a game
May call it load_engine later, or even start_engine() */
int start_engine(int player_money) {

    create_deck(&cards);
    if (cards == NULL) {
        /* Cards were not created successfully */
        fprintf(stderr, "An error occurred in deck creation\n");
        return -1;
    }

    // Create player
    player = (struct Player *) calloc(1, sizeof(struct Player));
    player->money = player_money;

    // Create dealer
    dealer = (struct Dealer *) calloc(1, sizeof(struct Dealer));

    srand(time(NULL));
    int num_cards = DECK_SIZE * NUM_DECKS;
    shuffle(cards, num_cards);

    // Randomness Check
    // for (int i = 0; i < DECK_SIZE; i++) {
    //     printf("%d ", cards[i]);
    // }
    // printf("%s", "\n");
    // shuffle(cards, numCards);

    // for (int i = 0; i < DECK_SIZE; i++) {
    //     printf("%d ", cards[i]);

    // }

    // engine(cards, player, dealer, numCards);

    return 0;
}

/**
 * Returns the player money at end of game.
 * Frees all memory that was used for game.
 */
int stop_engine() {
    if (cards != NULL) {
        free(cards);
    }

    if (player != NULL) {
        free(player);
    }

    if (dealer != NULL) {
        free(dealer);
    }
    return 0;
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

// Creates a deck of cards.
// Future functionality will be a shoe with multiple decks.
static void create_deck(uint8_t **deck) {
    // May want to just malloc instead of calloc since deck values are immediately set.
    *deck = calloc(DECK_SIZE, sizeof(uint8_t));
    // *deck = malloc(sizeof(uint8_t)*DECK_SIZE);

    for (int i = 0; i < DECK_SIZE; i++) {
        (*deck)[i] = i & 0xFF;
    }
}

/* Shuffle deck into a random order of cards. 
Used at beginning of engine and whenever a threshold 
of the number of cards have been played. */
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