#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define DECK_SIZE 52

void create_deck(uint8_t **deck);

int main(int argc, char **argv) {
    uint8_t *cards = NULL;
    create_deck(&cards);
    if (cards == NULL) {
        fprintf(stderr, "An error occurred in deck creation\n");
        return -1;
    }
    
    free(cards);
    return 0;
}

void create_deck(uint8_t **deck) {
    *deck = malloc(sizeof(uint8_t)*DECK_SIZE);
    for (int i = 0; i < DECK_SIZE; i++) {
        (*deck)[i] = i & 0xFF;
    }
    for (int i = 0; i < DECK_SIZE; i++) {
        printf("%d%s", (*deck)[i], "\n");
    }
}