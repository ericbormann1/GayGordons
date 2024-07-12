#pragma once

#include <array>
#include <algorithm>
#include <numeric>
#include <random>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>


/*This method game state storage is significantly more efficient than anything before it (I believe)
  Instead of using a structure of vectors or a long array, each card gets exactly 4 bits in a 32 bit int. 
  Each of these ints represents a pile. I waste 12 bits, yes, but unfortunately there's no 4bit*5*10= 200 bit int.
  This also lets me take advantage of super fast and efficient bitwise manipulation.
  Without multithreading, it can do 5000 games in about 7 seconds.
*/ 

struct GameState {
    uint32_t piles[10];
    uint8_t reserve;
    bool operator==(const GameState& other) const {
        return memcmp(this, &other, sizeof(*this)) == 0;
    }
};

// Function to initialize GameState with shuffled deck
void initializeGameState(GameState* state) {
    // Create a deck of cards (0-12 represent Ace to King)
    uint8_t deck[52];
    for (int i = 0; i < 52; ++i) {
        deck[i] = i % 13; // 0-12 (Ace to King)
    }

    // Shuffle the deck using std::shuffle and a random engine
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(deck, deck + 52, g);

    // Distribute cards into piles and reserve
    for (int i = 0; i < 10; ++i) {

        state->piles[i] = 0; // Initialize pile as empty
        for (int j = 0; j < 5; ++j) {
            state->piles[i] |= static_cast<uint32_t>(deck[i * 5 + j]) << (j * 4);
        }
        // fill the remaining slots with 0x0F
        state->piles[i] |= 0x0F << (5 * 4);
        state->piles[i] |= 0x0F << (6 * 4);
        state->piles[i] |= 0x0F << (7 * 4);

    }
    // set reserve cards
    state->reserve = (static_cast<uint8_t>(deck[50]) & 0x0F) | ((static_cast<uint8_t>(deck[51]) & 0x0F) << 4);
}

// Hasher for GameState
struct GameStateHasher {
    size_t operator()(const GameState& state) const {
        size_t hash = 0;
        for (int i = 0; i < 10; ++i) {
            hash ^= _rotl(state.piles[i], i % 32);
        }
        hash ^= _rotl(state.reserve, 10); // Rotate reserve by a fixed amount
        return hash;
    }
};

// a function which creates and returns a game state from a given shuffled array of 52 cards 
GameState createGameState(const int deck[52]) {
    GameState state;
    for (int i = 0; i < 10; ++i) {
        state.piles[i] = 0;
        for (int j = 0; j < 5; ++j) {
            // Reverse the order of cards in the pile
            state.piles[i] |= static_cast<uint32_t>(deck[i * 5 + (4 - j)]) << (j * 4);
        }
        // Fill the remaining slots with 0x0F
        state.piles[i] |= 0x0F << (5 * 4);
        state.piles[i] |= 0x0F << (6 * 4);
        state.piles[i] |= 0x0F << (7 * 4);
    }
    state.reserve = (static_cast<uint8_t>(deck[51]) & 0x0F) | ((static_cast<uint8_t>(deck[50]) & 0x0F) << 4);
    return state;
}

// a function that takes a pointer to an array of GameStates,
// and loads the game state array with states from a file of shuffled cards
// A function to load the 5000 decks  from a file, each number representing a card is separated by a space and each deck is separated by a new line
void loadDecksToStates(GameState* states, int size) {
    std::ifstream infile("../BenchmarkGen/benchmarkDecks.txt");
    std::string line;
    int lines = 0;
    while (getline(infile, line) && lines < size) {
        int deck[52];
        std::stringstream ss(line);
        std::string card;
        int index = 0;
        while (std::getline (ss, card, ' ')) {
            deck[index++] = std::stoi(card)%13;
        }
        states[lines] = createGameState(deck);
        lines++;
    }
    infile.close();
}