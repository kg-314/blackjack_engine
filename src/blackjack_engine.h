
// character values
struct Player {
    int money;
    int current_bet;
    int hand; // Hand just an int because suit does not really matter.
};

struct Dealer {
    int showing;
    int hand;
};

// struct GameState {
//     int currentBet;
//     struct Player *player;
//     struct Dealer *dealer;
// };

// Global variables that should be accessible by any user interface.
extern struct Player *player;
extern struct Dealer *dealer;

int start_engine(int player_money);
int stop_engine();