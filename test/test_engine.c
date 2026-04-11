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

int main() {
    test_engine_init();
    test_hit_increases_hand();
    test_double_down();

    printf("\nAll engine tests passed.\n");
    return 0;
}