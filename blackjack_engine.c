#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define DECK_SIZE 52

typedef struct player {
    int money;
    int hand;
} Player;

typedef struct dealer {
    int showing;
    int hand;
} Dealer;

void create_deck(uint8_t **deck);
void engine(uint8_t *cards, Player *player, Dealer *dealer);

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

}