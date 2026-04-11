#include "../src/blackjack_engine.h"
#include "test_utils.h"

void test_hand_simple() {
    Hand h = { .count = 0 };

    add_card(&h, 0);  // Ace
    add_card(&h, 9);  // 10

    ASSERT_EQ(get_hand_value(&h), 21);

    TEST_PASS();
}

void test_hand_ace_adjust() {
    Hand h = { .count = 0 };

    add_card(&h, 0);   // Ace
    add_card(&h, 12);  // King
    add_card(&h, 5);   // 6

    // Should be 17 (11+10+6 = 27 → adjust ace → 17)
    ASSERT_EQ(get_hand_value(&h), 17);

    TEST_PASS();
}

void test_hand_multiple_aces() {
    Hand h = { .count = 0 };

    add_card(&h, 0); // Ace
    add_card(&h, 13); // Ace
    add_card(&h, 26); // Ace

    ASSERT_EQ(get_hand_value(&h), 13);

    TEST_PASS();
}

int main() {
    test_hand_simple();
    test_hand_ace_adjust();
    test_hand_multiple_aces();

    printf("\nAll hand tests passed.\n");
    return 0;
}