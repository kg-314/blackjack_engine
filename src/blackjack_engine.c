#include "blackjack_engine.h"

static void build_shoe(struct GameState *game, int num_decks);
static void shuffle(uint8_t *cards, int num_cards, uint64_t *rng_state);
static void advance_turn(struct GameState *game);
static void compute_win(struct GameState *game);
static uint64_t next_rand(uint64_t *state);

/*
In-place initialization. Caller owns `game`. No allocation occurs here, so the
only failure modes are argument validation (reported via the return value).
*/
bool engine_init(struct GameState *game, int player_money, int num_players, int num_decks, uint64_t seed) {
    if (game == NULL) {
        return false;
    }
    if (num_players < 1 || num_players > MAX_PLAYERS) {
        return false;
    }
    if (num_decks < 1 || num_decks > MAX_DECKS) {
        return false;
    }
 
    game->curr_player = 0;
    game->num_players = num_players;
    game->test_mode = false;
    game->phase = PHASE_PAYOUT; // No round played yet; "between hands, ready to deal"
 
    // Build the shoe (sets deck_size and deck_pos) into the embedded array.
    build_shoe(game, num_decks);
 
    // Dealer starts empty.
    game->dealer.hand.count = 0;
 
    // Players start empty with their initial bankroll.
    for (int i = 0; i < num_players; i++) {
        game->players[i].hand.count = 0;
        game->players[i].money = player_money;
        game->players[i].current_bet = 0;
    }
 
    game->rng_state = (seed != 0) ? seed : 1; // xorshift can't start at 0
    shuffle(game->cards, game->deck_size, &game->rng_state); // initial deck shuffle
 
    return true;
}

/*
Fill the embedded shoe with `num_decks` ordered decks (card indices 0..51 per
deck) and reset the read position. The caller shuffles afterward. Operates on
the in-struct array, so there is no allocation. Bounded by MAX_SHOE because
num_decks is validated against MAX_DECKS before this is reached.
*/
static void build_shoe(struct GameState *game, int num_decks) {
    int n = 0;
    for (int d = 0; d < num_decks; d++) {
        for (int c = 0; c < DECK_SIZE; c++) {
            game->cards[n++] = (uint8_t)c;
        }
    }
    game->deck_size = n;   // num_decks * DECK_SIZE
    game->deck_pos = 0;
}

/*
Controls whose turn it is. Advances through players and hands off to the
dealer phase once every player has acted.
*/
static void advance_turn(struct GameState *game) {
    game->curr_player++;

    // Reaching num_players means it's the dealer's turn
    if (game->curr_player >= game->num_players) {
        game->phase = PHASE_DEALER_TURN;
    }
}

/*
Settle every player's bet against the dealer once the dealer's hand is final.
 
Natural blackjack (21 on the first two cards) pays 3:2 and is settled before
ordinary comparisons. A dealer natural beats any non-natural hand, including a
player's 3+ card 21; two naturals push.
 
NOTE: even-money settlement on a player blackjack vs. a dealer ace belongs here
once insurance/even-money is implemented. Until then there is no such option.
*/
static void compute_win(struct GameState *game) {
    int dealer_val = get_hand_value(&game->dealer.hand);
    bool dealer_blackjack = (dealer_val == 21 && game->dealer.hand.count == 2);
 
    for (int i = 0; i < game->num_players; i++) {
        PlayerData *p = &game->players[i];
 
        int player_val = get_hand_value(&p->hand);
        bool player_blackjack = (player_val == 21 && p->hand.count == 2);
 
        if (player_val > 21) {
            p->money -= p->current_bet;                       // player bust
        }
        else if (player_blackjack && !dealer_blackjack) {
            p->money += (p->current_bet * 3) / 2;             // natural pays 3:2
        }
        else if (dealer_blackjack && !player_blackjack) {
            p->money -= p->current_bet;                       // dealer natural beats non-natural
        }
        else if (dealer_val > 21 || player_val > dealer_val) {
            p->money += p->current_bet;                       // ordinary win
        }
        else if (player_val < dealer_val) {
            p->money -= p->current_bet;                       // ordinary loss
        }
        // push (equal totals, or two naturals) -> no change
    }
}

/*
Calculates the value of a hand, treating aces as 11 then demoting to 1 as needed.
Non-static for test cases.
*/
int get_hand_value(const Hand *h) {
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
Install a deterministic deck for testing. Copies into the embedded shoe; no
allocation, no process teardown. Returns false on invalid arguments.
*/
bool engine_set_deck(struct GameState *game, const uint8_t *deck, int size) {
    if (game == NULL || deck == NULL) {
        return false;
    }
    if (size <= 0 || size > MAX_SHOE) {
        return false;
    }
 
    for (int i = 0; i < size; i++) {
        game->cards[i] = deck[i];
    }
 
    game->deck_size = size;
    game->deck_pos = 0;
    game->test_mode = true;
    return true;
}

/*
Fisher-Yates shuffle. Used at init and whenever the live shoe is exhausted.
*/
static void shuffle(uint8_t *cards, int num_cards, uint64_t *rng_state) {
    if (num_cards > 1) { 
        for (int i = num_cards - 1; i > 0; i--) {
            int j = (int)(next_rand(rng_state) % (uint64_t)(i + 1));
            uint8_t temp = cards[i];
            cards[i] = cards[j];
            cards[j] = temp;
        }
    }
}

/*
xorshift64* PRNG. Per-instance state keeps games independent and reproducible.
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
Draw a card. In a live game the shoe reshuffles when exhausted, so this never
fails. With a deterministic test deck, exhaustion is reported via the return
value instead of tearing down the process. Returns false if no card is
available (or on NULL arguments).
*/
bool draw_card(struct GameState *game, uint8_t *out_card) {
    if (game == NULL || out_card == NULL) {
        return false;
    }
 
    if (game->deck_pos >= game->deck_size) {
        if (game->test_mode) {
            return false; // deterministic deck exhausted; caller decides what to do
        }
        shuffle(game->cards, game->deck_size, &game->rng_state);
        game->deck_pos = 0;
    }
 
    *out_card = game->cards[game->deck_pos++];
    return true;
}

/*
Begin a new hand. All players share one initial bet in this version.
Returns false if the game is not ready to deal, or if the (test) deck cannot
supply the opening cards; on failure no hand is started.
*/
bool deal(struct GameState *game, int initial_bet) {
    if (game == NULL) {
        return false;
    }
    if (game->phase != PHASE_PAYOUT) {
        return false;
    }
 
    game->curr_player = 0;
    game->dealer.hand.count = 0;
 
    for (int i = 0; i < game->num_players; i++) {
        game->players[i].hand.count = 0;
        game->players[i].current_bet = initial_bet;
    }
 
    // Two opening cards to each player and the dealer
    for (int r = 0; r < 2; r++) {
        for (int i = 0; i < game->num_players; i++) {
            uint8_t card;
            if (!draw_card(game, &card)) {
                game->phase = PHASE_PAYOUT; // abort cleanly; no hand in progress
                return false;
            }
            add_card(&game->players[i].hand, card);
        }
        uint8_t dealer_card;
        if (!draw_card(game, &dealer_card)) {
            game->phase = PHASE_PAYOUT;
            return false;
        }
        add_card(&game->dealer.hand, dealer_card);
        if (r == 0) {
            game->dealer.show_card = dealer_card;
        }
    }
 
    game->phase = PHASE_PLAYER_TURN;
    return true;
}

/*
Add a card to a hand, bounded by MAX_HAND.
*/
void add_card(Hand *h, uint8_t card) {
    if (h->count < MAX_HAND) {
        h->cards[h->count++] = card;
    }
}

/*
Apply a player action for the current player. Returns whether it was applied.
No partial state mutation occurs on failure.
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
            uint8_t card;
            if (!draw_card(game, &card)) {
                return false;
            }
            add_card(&curr->hand, card);
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
            uint8_t card;
            if (!draw_card(game, &card)) {
                return false; // no bet change unless the draw succeeds
            }
            curr->current_bet *= 2;
            add_card(&curr->hand, card);
            advance_turn(game);
            return true;
        }
        default:
            return false;
    }
}

/*
Enact the dealer's deterministic turn (hit to 17), then settle bets.
Bounded by MAX_HAND draws; stops early if a (test) deck runs out.
*/
void resolve_dealer(struct GameState *game) {
    if (game == NULL) {
        return;
    }
    if (game->phase != PHASE_DEALER_TURN) {
        return;
    }
 
    for (int i = 0; i < MAX_HAND; i++) {
        if (get_hand_value(&game->dealer.hand) >= 17) {
            break;
        }
        uint8_t card;
        if (!draw_card(game, &card)) {
            break; // deterministic deck exhausted; stop drawing
        }
        add_card(&game->dealer.hand, card);
    }
 
    compute_win(game);
    game->phase = PHASE_PAYOUT;
}
