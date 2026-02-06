#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#define DECK_SIZE 52
#define NUM_DECKS 1

// character values
typedef struct player {
    int money;
    int hand;
} Player;

typedef struct dealer {
    int showing;
    int hand;
} Dealer;

static void create_deck(uint8_t **deck);
static void engine(uint8_t *cards, Player *player, Dealer *dealer);
static void shuffle(uint8_t *cards);

// Currently main for testing, will be turning into setup(bool automate)
// so that other programs can call to start a game
int main(int argc, char **argv) {
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
    shuffle(cards);

    engine(cards, player, dealer);

    free(cards);
    free(player); // No internal pointers
    free(dealer); // No internal pointers
    return 0;
}

// The main function that drives the game
static void engine(uint8_t *cards, Player *player, Dealer *dealer) {

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
static void shuffle(uint8_t *cards) {
    // I need an algorithm to take an array of NULL values and fill with numbers
    // to represent the cards. No repeats allowed, all cards used, positions cannot 
    // It needs to randomly sort the numbers in the array.

    // The array does not necessarily have to be null to begin with though.

    int r = rand();
    printf("%d%s", r%52, "\n");
    r = rand();
    printf("%d%s", r%52, "\n");
}