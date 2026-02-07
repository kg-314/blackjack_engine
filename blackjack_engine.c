#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include "blackjack_engine.h"

#define DECK_SIZE 52
// May be removed later, as this will be a user-inputed value.
#define NUM_DECKS 1 // For when this program is extended to allow for shoe with multiple decks

static void create_deck(uint8_t **deck);
static void engine(uint8_t *cards, Player *player, Dealer *dealer, int numCards);
static void shuffle(uint8_t *cards, int numCards);

// Currently main for testing, will be turning into setup(bool automate)
// so that other programs can call to start a game
// May call it load_engine later, or even start_engine()
int start_engine() {
    uint8_t *cards = NULL;
    create_deck(&cards);
    if (cards == NULL) {
        fprintf(stderr, "An error occurred in deck creation\n");
        return -1;
    }

    // Create player
    Player *player = (Player *) calloc(1, sizeof(Player));
    player->money = 1000;

    // Create dealer
    Dealer *dealer = (Dealer *) calloc(1, sizeof(Dealer));

    srand(time(NULL));
    int numCards = DECK_SIZE * NUM_DECKS;
    shuffle(cards, numCards);

    // Randomness Check
    // for (int i = 0; i < DECK_SIZE; i++) {
    //     printf("%d ", cards[i]);
    // }
    // printf("%s", "\n");
    // shuffle(cards, numCards);

    // for (int i = 0; i < DECK_SIZE; i++) {
    //     printf("%d ", cards[i]);

    // }

    engine(cards, player, dealer, numCards);

    free(cards);
    free(player); // No internal pointers
    free(dealer); // No internal pointers
    return 0;
}

// The main function that drives the game
static void engine(uint8_t *cards, Player *player, Dealer *dealer, int numCards) {
    // Deck will be shuffled whenever at least 31 cards have been used (single deck).

    // Some basic functions are needed for the operation of blackjack.
    // dealCard() -- Deals a card to the active player
    // changeTurns() -- changes the active player (switch between player and dealer)
    // checkBust() -- check if player/dealer busted from card dealt.
    // payout() -- pay player if they win
    // bet() -- how much money you want to bet before a round starts
    // startRound() and finishRound()
    // doubleDown() -- triggers everything that follows from doubling down
    // buyInsurance()
    // evenMoney()

    // Player called functions:
    // hit()
    // bet()
    // doubleDown()
    // buyInsurance()
    // evenMoney()
    // quit()

    // Step 1: Query the player for how much they want to bet.
    // Engine needs to send some kind of askBet signal to user interface.
    for (int i = 0; i < DECK_SIZE; i++) {
        printf("%d ", cards[i]);
    }
    printf("%s", "\n");
    shuffle(cards, numCards);

    for (int i = 0; i < DECK_SIZE; i++) {
        printf("%d ", cards[i]);

    }

    // Game should end if player reaches 0 money or player quits.
}

static void create_deck(uint8_t **deck) {
    // May want to just malloc instead of calloc since deck values are immediately set.
    *deck = calloc(DECK_SIZE, sizeof(uint8_t));
    // *deck = malloc(sizeof(uint8_t)*DECK_SIZE);

    for (int i = 0; i < DECK_SIZE; i++) {
        (*deck)[i] = i & 0xFF;
    }
    // for (int i = 0; i < DECK_SIZE; i++) {
    //     printf("%d%s", (*deck)[i], "\n");
    // }
}

// Shuffles the deck of cards into a random order
static void shuffle(uint8_t *cards, int numCards) {
    // Fisher-Yates Algo for shuffling deck.
    if (numCards > 1) { // Should always be true.
        for (int i = numCards - 1; i > 0; i--) {
            int j = rand() % (i + 1);

            uint8_t tempCard = cards[i];
            cards[i] = cards[j];
            cards[j] = tempCard;
        }
    }

}