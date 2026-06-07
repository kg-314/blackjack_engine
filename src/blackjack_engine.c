#include "blackjack_engine.h"

// Will be removed later, as this will be a user-inputed value.
#define NUM_DECKS 1 // For when this program is extended to allow for shoe with multiple decks

static void create_deck(uint8_t **deck);
static void shuffle(uint8_t *cards, int num_cards, uint64_t *rng_state);
static void advance_turn(struct GameState *game);
static void compute_win(struct GameState *game);
static uint64_t next_rand(uint64_t *state);
int get_hand_value(Hand *h);

/* 
Replaced start_engine() with this function to allow UI to run multiple games at once.
It allocates memory and sets the most general initial values for game state.
*/
struct GameState *engine_create(int player_money, int num_players, uint64_t seed) {

    struct GameState *game = malloc(sizeof(struct GameState));
    if (game == NULL) {
        return NULL;
    }

    create_deck(&game->cards);

    if (game->cards == NULL) {
        free(game);
        return NULL;
    }

    //num_players++; // include dealer

    game->curr_player = 0;
    game->num_players = num_players;
    game->deck_pos = 0;
    game->phase = PHASE_PAYOUT; // No round played yet; treat as "between hands, ready to deal"
    game->players = malloc(sizeof(PlayerData) * num_players);
    game->test_mode = false;
    game->test_size = 0;

    if (game->players == NULL) {
        free(game->cards);
        free(game);
        return NULL;
    }

    // Set dealer initial card count to zero
    game->dealer.hand.count = 0;
    
    // Set player initial card count to zero and set their initial money
    for (int i = 0; i < num_players; i++) {
        game->players[i].hand.count = 0;
        game->players[i].money = player_money;
    }

    game->rng_state = (seed != 0) ? seed : 1;   // xorshift can't start at 0
    shuffle(game->cards, DECK_SIZE * NUM_DECKS, &game->rng_state);

    return game;
}

/*
Free all memory associated with the game.
*/
void engine_destroy(struct GameState *game) {
    if (!game) return;

    if (game->cards) free(game->cards);
    if (game->players) free(game->players);

    free(game);
}

/*
This function will control who's turn it is.
It advances through players when required and 
sets the dealer phase when it is reached.
*/
static void advance_turn(struct GameState *game) {
    game->curr_player++;

    // Reaching num_players means it's the dealer's turn
    if (game->curr_player >= game->num_players) {
        game->phase = PHASE_DEALER_TURN;
    }
}

/*
At the end of each hand (when the dealer stands or busts),
it must be computer who won and they must be paid.
*/
static void compute_win(struct GameState *game) {
    int dealer_val = get_hand_value(&game->dealer.hand);

    for (int i = 0; i < game->num_players; i++) {
        PlayerData *p = &game->players[i];

        int player_val = get_hand_value(&p->hand);

        if (player_val > 21) {
            p->money -= p->current_bet;
        }
        else if (dealer_val > 21 || player_val > dealer_val) {
            p->money += p->current_bet;
        }
        else if (player_val < dealer_val) {
            p->money -= p->current_bet;
        }
        // tie -> no change
    }
}

/*
Calculates the value of a player's hand.
Non-static for test cases.
*/
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

/* 
Creates a deck of cards.
Future functionality will be a shoe with multiple decks. 
*/
static void create_deck(uint8_t **deck) {
    *deck = malloc(NUM_DECKS * DECK_SIZE * sizeof(uint8_t));

    // Return early if malloc failed.
    if ((*deck) == NULL) {
        return;
    }

    for (int i = 0; i < (NUM_DECKS * DECK_SIZE); i++) {
        (*deck)[i] = (i % DECK_SIZE) & 0xFF;
    }
}

/*
A test function to have deterministic deck.
Requires a 
*/
void engine_set_deck(struct GameState *game, uint8_t *deck, int size) {
    if (!game) return;

    if (game->cards) {
        free(game->cards);
    }

    game->cards = malloc(sizeof(uint8_t) * size);
    if (game->cards == NULL) {
        free(game->players);
        free(game);
        fprintf(stderr, "Test deck creation failed.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < size; i++) {
        game->cards[i] = deck[i];
    }

    game->deck_pos = 0;
    game->test_mode = true;
    game->test_size = size;
}

/* 
Shuffle deck into a random order of cards. 
Used at beginning of engine and whenever a threshold 
of the number of cards have been played. 
*/
static void shuffle(uint8_t *cards, int num_cards, uint64_t *rng_state) {
    // Fisher-Yates Algo for deck shuffling
    if (num_cards > 1) { // This should always be true
        for (int i = num_cards - 1; i > 0; i--) {
            int j = (int)(next_rand(rng_state) % (uint64_t)(i + 1));
            uint8_t temp = cards[i];
            cards[i] = cards[j];
            cards[j] = temp;
        }
    }
}

/*
Helper function for shuffling.
Generates a pseudo-random number for card swaps.
*/
static uint64_t next_rand(uint64_t *state) {
    uint64_t x = *state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    *state = x;
    return x * 0x2545F4914F6CDD1DULL;
}

/*
This should begin a new hand.
Player will input their initial bet for the hand.
For simplicity, all players have same initial bet in this current version.
*/
void deal(struct GameState *game, int initial_bet) {
    if (game == NULL) {
        return;
    }

    if (game->phase != PHASE_PAYOUT) {
        return;
    }

    game->curr_player = 0;

    // Set dealer hand count to zero
    game->dealer.hand.count = 0;

    // Set up player hand counts to zero and set their bets
    for (int i = 0; i < game->num_players; i++) {
        game->players[i].hand.count = 0;
        game->players[i].current_bet = initial_bet;
    }

    // Deal 2 cards to all to start blackjack round.
    for (int r = 0; r < 2; r++) {
        for (int i = 0; i < game->num_players; i++) {
            add_card(&game->players[i].hand, draw_card(game));
        }
        uint8_t card = draw_card(game);
        add_card(&game->dealer.hand, card);
        if (r == 0) {
            game->dealer.show_card = card;
        }
    }
    game->phase = PHASE_PLAYER_TURN;
}

/*
Helper function that draws a card from the deck and reshuffles if deck (shoe) is empty.
*/
uint8_t draw_card(struct GameState *game) {
    if (game->test_mode) { // If the game is in test mode we do not want to shuffle, just end game after test.
        if (game->deck_pos >= game->test_size) {
            fprintf(stderr, "Test deck out of cards!\n");
            free(game->players);
            free(game->cards);
            free(game);
            exit(EXIT_FAILURE);
        }
    } else if (game->deck_pos >= (DECK_SIZE * NUM_DECKS)) {
        shuffle(game->cards, DECK_SIZE * NUM_DECKS, &game->rng_state);
        game->deck_pos = 0;
    }
    return game->cards[game->deck_pos++]; // Return the current card and move pointer.
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
Allows for user actions to occur if correct game conditions are met.
Returns whether the action was applied successfully or not.
*/
bool apply_action(struct GameState *game, Action action) {
    if (game == NULL) {
        return false;
    }
    if (game->phase != PHASE_PLAYER_TURN) {
        return false;
    }

    PlayerData *curr = &game->players[game->curr_player];

    switch (action) {
        case ACTION_HIT: {
            add_card(&curr->hand, draw_card(game));
            if (get_hand_value(&curr->hand) > 21) {
                advance_turn(game);
            }
            return true;
        }
        case ACTION_STAND: {
            advance_turn(game);
            return true;
        }
        case ACTION_DOUBLE: {
            if (curr->money < curr->current_bet) {
                return false;
            }
            curr->current_bet *= 2;
            add_card(&curr->hand, draw_card(game));
            advance_turn(game);
            return true;
        }
        default:
            return false;
    }
}

void resolve_dealer(struct GameState *game) {
    if (game == NULL) {
        return;
    }

    if (game->phase != PHASE_DEALER_TURN) {
        return;
    }

    // Bounded loop at most MAX_HAND draws
    for (int i = 0; i < MAX_HAND; i++) {
        if (get_hand_value(&game->dealer.hand) >= 17) break;
        add_card(&game->dealer.hand, draw_card(game));
    }

    compute_win(game);
    game->phase = PHASE_PAYOUT;
}
