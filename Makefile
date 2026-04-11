CC = gcc
CFLAGS = -Wall -Wextra -g

SRC = src/blackjack_engine.c

TEST_HAND = test/test_hand
TEST_ENGINE = test/test_engine

.PHONY: all test clean

all: test

test: $(TEST_HAND) $(TEST_ENGINE)
	./$(TEST_HAND)
	./$(TEST_ENGINE)

$(TEST_HAND): test/test_hand.c $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_ENGINE): test/test_engine.c $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f test/test_hand test/test_engine