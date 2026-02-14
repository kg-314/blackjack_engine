#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#include "blackjack_engine.h"

int eval(char *input);

static char prompt[] = "blackjack> "; /* Command line prompt */

int main(int argc, char **argv) {

    printf("%s", "Welcome to the Blackjack game engine.\n");
    printf("%s", "To start a game, Type: start <INITIAL_MONEY>\n");

    while (1) {
        printf("%s", prompt);

        char buffer[100];

        /* fgets failed for whatever reason. */
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            fprintf(stderr, "Error receiving a user input.");
        }

        if (eval(buffer) == -1) {
            /* A command failed */
            fprintf(stderr, "Game engine failure.");
            stop_engine();
            return 1;
        }

    }
}

// Need to tokenize inputs.

int eval(char *input) {

}