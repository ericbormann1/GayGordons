/*This program determines the solvability of Gay Gordons, a card game where a deck is shuffled and 
distributed into ten piles of 5 cards, with the 2 remaining cards going into a reserve pile. 
You are trying to make pairs of 11, with ace going with 10, king going with queen, and jack going with jack. 
Color and suit do not matter. Only the top cards of each stack are in play.
 */

// using command: cl /O2 main.cpp in VS Developer Terminal,
/* Yielded:
Enter number of simulations: 1000000
22 threads will be used.
Number of simulations per thread: 45454
Number of remainder simulations: 12
Unsolvable percentage: 19.3016%
Unsolvable count: 193016
Total time: 49292 milliseconds.
AKA: 1 million random games evaluated in 50 seconds.
*/

#pragma once

#include <iostream>
#include <iomanip>
#include <array>
#include <algorithm>
#include <numeric>
#include <random>
#include <cstring>
#include <unordered_set>
#include <chrono>
#include <mutex>
#include <thread>

#include "gameState.h"
#include "print.h"
#include "solver.h"

using namespace std;

const bool benchmarking = false;

int numSimulations = 100000;

bool solve (GameState& state, unordered_set<GameState, GameStateHasher>& visited) {
    bool allEmpty = true;
    for (int i = 0; i < NUM_PILES; ++i) {
        if (getTopPileCard(&state, i) != 15) {
            allEmpty = false;
            break;
        }
    }
    if (allEmpty && getTopReserveCard(&state) == 15) return true;

    if (visited.find(state) != visited.end()) return false;
    visited.insert(state);
    // try to find a valid pair from piles
    for (int i = 0; i < NUM_PILES; ++i) {
        int topCard1 = getTopPileCard(&state, i);
        if (topCard1 != 15) {
            for (int j = i + 1; j < NUM_PILES; ++j) {
                int topCard2 = getTopPileCard(&state, j);
                if (topCard2 != 15 && isPair(topCard1, topCard2)) {
                    int card1 = removeTopPileCard(&state, i);
                    int card2 = removeTopPileCard(&state, j);
                    if (solve(state, visited)) return true;
                    addPileCard(&state, i, card1);
                    addPileCard(&state, j, card2);
                }
            }
        }
    }
    // try to find a valid pair between reserve and piles
    int topReserve = getTopReserveCard(&state);
    if (topReserve != 15) {
        for (int i = 0; i < NUM_PILES; ++i) {
            int topCard = getTopPileCard(&state, i);
            if (topCard != 15 && isPair(topReserve, topCard)) {
                int card = removeTopPileCard(&state, i);
                int reserve = removeTopReserveCard(&state);
                if (solve(state, visited)) return true;
                addPileCard(&state, i, card);
                addReserveCard(&state, reserve);
            }
        }
    }
    return false;
}

// inline helper function to see if any given pile has three jacks
inline bool hasThreeJacks(GameState* state) {
    for (int i = 0; i < NUM_PILES; ++i) {;
        int jackCount = 0;
        uint32_t pile = state->piles[i];
        for (int j = 0; j < PILE_SIZE; ++j) {
            //int card = pile >> (4 * i) & 0x0F;
            int card = pile & 0x0F;
            pile >>= 4;
            if (card == 10) {
                jackCount++;
                if (jackCount >= 3) {
                    return true;
                }
            }
        }
    }
    return false;
}

// function which check to see if a state is solvable
bool isSolvable(GameState* state) {
    unordered_set<GameState, GameStateHasher> visited;
    if (hasThreeJacks(state)) { 
        //std::cout << "Impossible to solve (3 Jacks in a stack)." << std::endl; 
        return false; 
    }
    return solve(*state, visited);
}

/* function which runs on a thread, running simulations.
Input is a the number of simulations it should run, the offset if benchmarking
of where to access the array of game states, and a reference to the total unsolvable count (passed as mutex),
a reference to a mutex, and a reference to the game state array
*/
void simulateGames(int numSimsThisThread, int offset, int& unsolvableCount, mutex& mtx, GameState* gameArray) {
    if (benchmarking) {
        for (int i = 0; i < numSimsThisThread; ++i) {
            if (!isSolvable(&gameArray[offset + i])) {
                lock_guard<mutex> lock(mtx);
                unsolvableCount++;
            }
        }
    } else {
        for (int i = 0; i < numSimsThisThread; ++i) {
            GameState active;
            initializeGameState(&active);
            if (!isSolvable(&active)) {
                lock_guard<mutex> lock(mtx);
                unsolvableCount++;
            }
        }
    }
}

int main() {
    // create an array of game states
    GameState* gameArray = new GameState[numSimulations];
    
    if (benchmarking) {
        std::cout << "Loading decks..." << std::endl;
        auto start = chrono::steady_clock::now();
        loadDecksToStates(gameArray, numSimulations);
        auto end = chrono::steady_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
        std::cout << "Decks loaded in " << duration.count() << " milliseconds." << std::endl;
        std::cout << "Number of decks loaded: " << numSimulations << std::endl;
    }
    std::cout << "Enter number of simulations: ";
    std::cin >> numSimulations;
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
        threads.emplace_back(simulateGames, simulationsForThisThread, offsetCounter, ref(unsolvableCount), ref(mtx), ref(gameArray));
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
    std::cout << "Total time: " << totalDuration.count() << " milliseconds." << std::endl;

    return 0;
}

/* recursive function which takes a game state, removes the top card from a specified column, 
updates the game state, then calls the function with the new game state. When it has gotten all the cards (attempted removal results in 15),
it backtracks adding each card back to the column.
*/
// DEBUGGING
void removeCards(GameState* state, int column) {
    int card = removeTopPileCard(state, column);
    printGameState (*state);
    if (card == 15) {
        return;
    }
    removeCards(state, column);
    addPileCard(state, column, card);
    printGameState (*state);
}
// DEBUGGING
void removeReserve(GameState* state) {
    int card = removeTopReserveCard(state);
    printGameState (*state);
    if (card == 15) {
        return;
    }
    removeReserve(state);
    addReserveCard(state, card);
    printGameState (*state);
}