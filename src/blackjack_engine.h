#define DECK_SIZE 52

// character values
struct Player {
    int money;
    int current_bet;
    int hand; // Hand just an int because suit does not really matter.
};

struct Dealer {
    int show_card;
    int hand;
};

enum Type {
    TYPE_P,
    TYPE_D
};

struct Character {
    enum Type type;
    union {
        struct Player p;
        struct Dealer d;
    } data;
};

/*
All changes to player states will be controlled through the game state.
*/
struct GameState {
    int curr_player;
    int num_players;
    struct Character *players;
};

// struct GameState {
//     int currentBet;
//     struct Player *player;
//     struct Dealer *dealer;
// };

// Global variables that should be accessible by any user interface.
extern struct Player *player;
extern struct Dealer *dealer;

int start_engine(int player_money, int num_players);
int stop_engine();