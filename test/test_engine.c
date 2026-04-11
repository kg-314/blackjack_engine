#include "../src/blackjack_engine.h"
#include "test_utils.h"

void test_engine_init() {
    struct GameState *game = engine_create(1000, 1);

    ASSERT_TRUE(game != NULL);
    ASSERT_EQ(game->num_players, 2); // player + dealer

    engine_destroy(game);

    TEST_PASS();
}

void test_hit_increases_hand() {
    struct GameState *game = engine_create(1000, 1);

    deal(game, 100);

    int before = game->players[0].data.p.hand.count;

    hit(game);

    int after = game->players[0].data.p.hand.count;

    ASSERT_TRUE(after == before + 1);

    engine_destroy(game);

    TEST_PASS();
}

void test_double_down() {
    struct GameState *game = engine_create(1000, 1);

    deal(game, 100);

    int before = game->players[0].data.p.current_bet;

    double_down(game);

    int after = game->players[0].data.p.current_bet;

    ASSERT_EQ(after, before * 2);

    engine_destroy(game);

    TEST_PASS();
}

/* Deterministic engine tests. */
void test_player_bust() {
    struct GameState *game = engine_create(1000, 1);

    // Player: 10, 9; Dealer: 6, 7; Player draws 5 and busts.
    uint8_t deck[] = {
        9, 5,
        8, 6,
        4        // hit: 5
    };

    engine_set_deck(game, deck, 5);

    deal(game, 100);
    //printf("Made it past deal.\n");
    // This test was failing here because changeTurn(),
    // which is called when a player busts, calls draw_card(),
    // which was making the engine draw more cards than the test deck had.
    hit(game); // should bust
    //printf("Made it past hit.\n");

    int val = get_hand_value(&game->players[0].data.p.hand);
    //printf("Player hand value: %d\n", val);

    ASSERT_TRUE(val > 21);

    engine_destroy(game);
    TEST_PASS();
}

void test_dealer_hits_until_17() {
    struct GameState *game = engine_create(1000, 1);

    // Player: 19; Dealer: 9; Dealer should hit twice and end with 20.
    uint8_t deck[] = {
        9, 5,
        8, 2,
        3, 6,   // dealer hits these 2 cards
        0       // dealer should stand (not take this ace)
        // next hit would be needed if <17
    };

    engine_set_deck(game, deck, 7);

    deal(game, 100);

    stand(game); // player done

    test_dealer_draw(game); // Dealer should draw until 17

    int dealer_val = get_hand_value(&game->players[1].data.d.hand);

    ASSERT_TRUE(dealer_val == 20);

    engine_destroy(game);
    TEST_PASS();
}

int main() {
    test_engine_init();
    test_hit_increases_hand();
    test_double_down();
    test_player_bust();
    test_dealer_hits_until_17();

    printf("\nAll engine tests passed.\n");
    return 0;
}