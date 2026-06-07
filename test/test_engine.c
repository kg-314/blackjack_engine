#include "../src/blackjack_engine.h"
#include "test_utils.h"

/* Caller-owned, in-place initialization on the stack. */
void test_engine_init_inplace() {
    struct GameState game;
    bool ok = engine_init(&game, 1000, 3, 1, 42);
 
    ASSERT_TRUE(ok);
    ASSERT_EQ(game.num_players, 3);
    ASSERT_EQ(game.phase, PHASE_PAYOUT);
    ASSERT_EQ(game.players[0].money, 1000);
    ASSERT_EQ(game.deck_size, DECK_SIZE);          // single deck
    ASSERT_TRUE(game.rng_state != 0);
 
    // Multi-deck shoe sizing.
    ASSERT_TRUE(engine_init(&game, 1000, 1, 6, 42));
    ASSERT_EQ(game.deck_size, 6 * DECK_SIZE);
 
    // Invalid argument paths report failure instead of crashing.
    ASSERT_TRUE(engine_init(&game, 1000, 0, 1, 1) == false);                 // num_players too low
    ASSERT_TRUE(engine_init(&game, 1000, MAX_PLAYERS + 1, 1, 1) == false);   // num_players too high
    ASSERT_TRUE(engine_init(&game, 1000, 1, 0, 1) == false);                 // num_decks too low
    ASSERT_TRUE(engine_init(&game, 1000, 1, MAX_DECKS + 1, 1) == false);     // num_decks too high
    ASSERT_TRUE(engine_init(NULL, 1000, 1, 1, 1) == false);                  // NULL game
 
    TEST_PASS();
}

void test_hit_increases_hand() {
    struct GameState game;
    ASSERT_TRUE(engine_init(&game, 1000, 1, 1, 1));
 
    deal(&game, 100);
 
    uint8_t before = game.players[0].hand.count;
 
    apply_action(&game, ACTION_HIT);
 
    uint8_t after = game.players[0].hand.count;
 
    ASSERT_TRUE(after == before + 1);
 
    TEST_PASS();
}

void test_double_down() {
    struct GameState game;
    ASSERT_TRUE(engine_init(&game, 1000, 1, 1, 1));
 
    deal(&game, 100);
 
    int before = game.players[0].current_bet;
 
    apply_action(&game, ACTION_DOUBLE);
 
    int after = game.players[0].current_bet;
 
    ASSERT_EQ(after, before * 2);
 
    TEST_PASS();
}

/* Deterministic engine tests. */
/* Deterministic engine tests. */
void test_player_bust() {
    struct GameState game;
    ASSERT_TRUE(engine_init(&game, 1000, 1, 1, 1));
 
    // Player: 10, 9; Dealer: 6, 7; Player draws 5 and busts.
    uint8_t deck[] = {
        9, 5,
        8, 6,
        4        // hit: 5
    };
 
    ASSERT_TRUE(engine_set_deck(&game, deck, 5));
 
    ASSERT_TRUE(deal(&game, 100));
    apply_action(&game, ACTION_HIT);  // bust -> phase advances to dealer
 
    int val = get_hand_value(&game.players[0].hand);
 
    ASSERT_TRUE(val > 21);
    ASSERT_TRUE(game.phase == PHASE_DEALER_TURN); // Make sure it goes to dealer's turn
 
    TEST_PASS();
}

void test_dealer_hits_until_17() {
    struct GameState game;
    ASSERT_TRUE(engine_init(&game, 1000, 1, 1, 1));
 
    // Player: 19; Dealer: 9; Dealer should hit twice and end with 20.
    uint8_t deck[] = {
        9, 5,
        8, 2,
        3, 6,   // dealer hits these 2 cards
        0       // dealer should stand (not take this ace)
    };
 
    ASSERT_TRUE(engine_set_deck(&game, deck, 7));
 
    ASSERT_TRUE(deal(&game, 100));
 
    apply_action(&game, ACTION_STAND); // player done
 
    resolve_dealer(&game); // Dealer should draw until 17
 
    int dealer_val = get_hand_value(&game.dealer.hand);
 
    ASSERT_TRUE(dealer_val == 20);
 
    TEST_PASS();
}

/* Natural blackjack pays 3:2 against a non-blackjack dealer. */
void test_blackjack_pays_3to2() {
    struct GameState game;
    ASSERT_TRUE(engine_init(&game, 1000, 1, 1, 1));
 
    // Player: Ace(0) + Ten(9) = natural 21.
    // Dealer: 9 + 9 = 18 (stands at 18, not a natural).
    uint8_t deck[] = {
        0, 8,    // player Ace, dealer 9
        9, 8     // player Ten, dealer 9
    };
 
    ASSERT_TRUE(engine_set_deck(&game, deck, 4));
    ASSERT_TRUE(deal(&game, 100));
 
    apply_action(&game, ACTION_STAND);
    resolve_dealer(&game);
 
    // 1000 + (100 * 3 / 2) = 1150
    ASSERT_EQ(game.players[0].money, 1150);
 
    TEST_PASS();
}

int main() {
    test_engine_init_inplace();
    test_hit_increases_hand();
    test_double_down();
    test_player_bust();
    test_dealer_hits_until_17();
    test_blackjack_pays_3to2();
 
    printf("\nAll engine tests passed.\n");
    return 0;
}