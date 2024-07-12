/*This program determines the solvability of Gay Gordons, a card game where a deck is shuffled and 
distributed into ten piles of 5 cards, with the 2 remaining cards going into a reserve pile. 
You are trying to make pairs of 11, with ace going with 10, king going with queen, and jack going with jack. 
Color and suit do not matter. Only the top cards of each stack are in play.
 */

//// HOLY SHIT THIS CODE EVALUATED 5000 STATES to 992 in 2000ms!!

const bool BENCHMARKING = true;
const bool DEBUG = false;
const bool ADVANCEDEBUG = false;
// numSims is 5000 in conventional benchmarking
const int numSimulations = 100;
#pragma once
#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>
#include <numeric>
#include <unordered_set>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <thread>
#include <mutex>
#include <array>
#include <immintrin.h>

using namespace std;

inline bool isPair(int card1, int card2) {
    int rank1 = card1 % 13;
    int rank2 = card2 % 13;

    if ((rank1 + rank2 == 9) || (rank1 == 12 && rank2 == 11) || (rank1 == 11 && rank2 == 12) || (rank1 == 10 && rank2 == 10)) {
        return true;
    }
    return false;
}

inline bool hasThreeJacks(const int* piles) {
    for (int i = 0; i < 10; ++i) {
        int countJacks = 0;
        for (int j = 0; j < 5; ++j) {
            if (piles[i * 5 + j] % 13 == 10) {
                countJacks++;
                if (countJacks >= 3) {
                    return true;
                }
            } else {
                countJacks = 0;
            }
        }
    }
    return false;
}

const int NUM_PILES = 10;
const int PILE_SIZE = 5;
const int RESERVE_SIZE = 2;

struct CompactGameState {
    int state[NUM_PILES * PILE_SIZE + RESERVE_SIZE];

    bool operator==(const CompactGameState& other) const {
        return memcmp(state, other.state, sizeof(state)) == 0;
    }
};

struct CompactGameStateHasher {
    size_t operator()(const CompactGameState& state) const {
        size_t hash = 0;
        for (int i = 0; i < NUM_PILES * PILE_SIZE + RESERVE_SIZE; ++i) {
            hash ^= _rotl(state.state[i], i % 32);
        }
        return hash;
    }
};

inline int findTopCard(const int* pile) {
    for (int i = PILE_SIZE - 1; i >= 0; --i) {
        if (pile[i] != -1) {
            return i;
        }
    }
    return -1;  // Return -1 if no cards are found
}

inline int findTopReserve(const int* reserve) {
    for (int i = RESERVE_SIZE - 1; i >= 0; --i) {
        if (reserve[i] != -1) {
            return i;
        }
    }
    return -1;  // Return -1 if no cards are found
}

bool solve(CompactGameState& state, unordered_set<CompactGameState, CompactGameStateHasher>& visited) {
    bool allEmpty = true;
    for (int i = 0; i < NUM_PILES; ++i) {
        if (findTopCard(state.state + i * PILE_SIZE) != -1) {
            allEmpty = false;
            break;
        }
    }
    if (allEmpty && findTopReserve(state.state + NUM_PILES * PILE_SIZE) == -1) return true;

    if (visited.find(state) != visited.end()) return false; // state already visited
    visited.insert(state);

    // Try to find a valid pair from piles
    for (int i = 0; i < NUM_PILES; ++i) {
        int topCardIndex1 = findTopCard(state.state + i * PILE_SIZE);
        if (topCardIndex1 != -1) {
            for (int j = i + 1; j < NUM_PILES; ++j) {
                int topCardIndex2 = findTopCard(state.state + j * PILE_SIZE);
                if (topCardIndex2 != -1 && isPair(state.state[i * PILE_SIZE + topCardIndex1], state.state[j * PILE_SIZE + topCardIndex2])) {
                    int card1 = state.state[i * PILE_SIZE + topCardIndex1];
                    int card2 = state.state[j * PILE_SIZE + topCardIndex2];
                    state.state[i * PILE_SIZE + topCardIndex1] = -1;
                    state.state[j * PILE_SIZE + topCardIndex2] = -1;

                    if (solve(state, visited)) return true;

                    state.state[i * PILE_SIZE + topCardIndex1] = card1;
                    state.state[j * PILE_SIZE + topCardIndex2] = card2;
                }
            }
        }
    }

    // Try to find a valid pair between reserve and piles
    int topReserveIndex = findTopReserve(state.state + NUM_PILES * PILE_SIZE);
    if (topReserveIndex != -1) {
        for (int i = 0; i < NUM_PILES; ++i) {
            int topCardIndex = findTopCard(state.state + i * PILE_SIZE);
            if (topCardIndex != -1 && isPair(state.state[i * PILE_SIZE + topCardIndex], state.state[NUM_PILES * PILE_SIZE + topReserveIndex])) {
                int card1 = state.state[i * PILE_SIZE + topCardIndex];
                int card2 = state.state[NUM_PILES * PILE_SIZE + topReserveIndex];
                state.state[i * PILE_SIZE + topCardIndex] = -1;
                state.state[NUM_PILES * PILE_SIZE + topReserveIndex] = -1;

                if (solve(state, visited)) return true;

                state.state[i * PILE_SIZE + topCardIndex] = card1;
                state.state[NUM_PILES * PILE_SIZE + topReserveIndex] = card2;
            }
        }
    }

    return false;
}

// Helper function to convert deck to CompactGameState, expects a shuffled deck.
CompactGameState createInitialGameState(const int deck[52]) {
    CompactGameState state;
    memcpy(state.state, deck, sizeof(int) * 52);
    return state;
}

bool isSolvable(const CompactGameState& state) {
    unordered_set<CompactGameState, CompactGameStateHasher> visited;
    if (hasThreeJacks(state.state)) {
        cout << "Impossible to solve (3 Jacks in a stack)." << endl;
        return false;
    }

    return solve(const_cast<CompactGameState&>(state), visited);
}

// Generates a random game state.
CompactGameState generateState() {
    int deck[52];
    iota(deck, deck + 52, 0);

    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    default_random_engine randomNumberGenerator(seed);

    shuffle(deck, deck + 52, randomNumberGenerator);
    return createInitialGameState(deck);
}

// A function to load the 5000 decks  from a file, each number representing a card is separated by a space and each deck is separated by a new line
vector<CompactGameState> loadDecksToStates() {
    vector<CompactGameState> decks;
    ifstream file("../BenchmarkGen/benchmarkDecks.txt");
    string line;

    // Use a while loop together with the getline() function to read the file line by line
    while (getline (file, line)) {
        int deck[52];
        stringstream ss(line);
        string card;
        int index = 0;
        while (getline(ss, card, ' ')) {
            deck[index++]=(stoi(card));
        }
        decks.push_back(createInitialGameState(deck));
    }
    file.close();
    return decks;
}

void simulateGames(int numSimsThisThread, int offset, int& unsolvableCount, mutex& mtx, vector<CompactGameState>& decks) {
    if (BENCHMARKING) {
        for (int i = 0; i < numSimsThisThread; ++i) {
            if (!isSolvable(decks[offset + i])) {
                lock_guard<mutex> lock(mtx);
                unsolvableCount++;
            }
        }
    } else {
        for (int i = 0; i < numSimsThisThread; ++i) {
            if (!isSolvable(generateState())) {
                lock_guard<mutex> lock(mtx);
                unsolvableCount++;
            }
        }
    }
}


int main() {
    vector<CompactGameState> decks;
    if (BENCHMARKING)
    {
        cout << "Decks are being loaded..." << endl;
        decks = loadDecksToStates();
        cout << "Decks are loaded." << endl;
        cout << "Number of decks: " << decks.size() << endl;
    }
    auto totalStart = chrono::steady_clock::now();
    const int numThreads = thread::hardware_concurrency();
    vector<thread> threads;
    mutex mtx;
    int simulationsPerThread = numSimulations / numThreads;
    int remainderSimulations = numSimulations % numThreads;

    int unsolvableCount = 0;
    cout << numThreads << " threads will be used." << endl;
    cout << "Number of simulations per thread: " << simulationsPerThread << endl;
    cout << "Number of remainder simulations: " << remainderSimulations << endl;
    int offsetCounter = 0;
    for (int i = 0; i < numThreads; ++i) {
        int simulationsForThisThread = simulationsPerThread + (i < remainderSimulations ? 1 : 0);
        threads.emplace_back(simulateGames, simulationsForThisThread, offsetCounter, ref(unsolvableCount), ref(mtx), ref(decks));
        offsetCounter += simulationsForThisThread;
    }

    for (auto& t : threads) {
        t.join();
    }
    double unsolvablePercentage = (double)unsolvableCount / numSimulations * 100;
    cout << "Unsolvable percentage: " << unsolvablePercentage << "%" << endl;
    cout << "Unsolvable count: " << unsolvableCount << endl;
    auto totalEnd = chrono::steady_clock::now();
    auto totalDuration = chrono::duration_cast<chrono::milliseconds>(totalEnd - totalStart);
    cout << "Total time: " << totalDuration.count() << " milliseconds." << endl;
    return 0;
}