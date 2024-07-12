/*This program determines the solvability of Gay Gordons, a card game where a deck is shuffled and 
distributed into ten piles of 5 cards, with the 2 remaining cards going into a reserve pile. 
You are trying to make pairs of 11, with ace going with 10, king going with queen, and jack going with jack. 
Color and suit do not matter. Only the top cards of each stack are in play.
 */

 // See game state to understand how this is different.
 // Without multithreading, it can do 5000 games in about 7 seconds.

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

#include "gameState.h"
#include "print.h"
#include "solver.h"

using namespace std;

const bool benchmarking = false;

const int numSimulations = 5000;

/* Recursive solve function which takes reference to a game state and reference to an
unordered_set of visited game states. It returns true if the game state is solvable.
It starts by checking if all the piles are empty. If that is the case, it returns true.
It then checks if the game state is already in the set. If it is, it returns false. 
If the game state is not in the set, it adds the game state to the set. It then checks
every possible pair of top cards in the piles. If it finds a pair, it removes the cards and 
calls solve on the new game state. If the called solve function returns true, it returns true./
If it does not return true, it adds the cards back to the pile.
It then checks all the pairs between the reserve card and the tops of each pile.
If it finds a pair, it removes the cards and calls solve on the new game state.
If the called solve function returns true, it returns true.
If it does not return true, it adds the cards back to the pile.
Once it has gone through every possible combination, it returns false.
 */
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

// function which check to see if a state is solvable
bool isSolvable(GameState* state) {
    unordered_set<GameState, GameStateHasher> visited;
    return solve(*state, visited);
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
int main() {
    // create an array of game states
    GameState* gameArray = new GameState[numSimulations];
    
    // randomly initialize each game state
    /*for (int i = 0; i < 100; ++i) {
        initializeGameState(&gameArray[i]);
    }*/
    // load the decks
    if (benchmarking) loadDecksToStates(gameArray, numSimulations);

    auto totalStart = chrono::steady_clock::now();
    // iterate through each game state, printing if is solvable. Also,
    // keep track of how many are unsolvable
    int unsolvableCount = 0;
    if (benchmarking) {
        for (int i = 0; i < numSimulations; ++i) {
            if (isSolvable(&gameArray[i])) {
                //std::cout << "State " << i << " is solvable." << std::endl;
            } else {
                //std::cout << "State " << i << " is unsolvable." << std::endl;
                unsolvableCount++;
            }
        }
    } else {
        for (int i = 0; i < numSimulations; ++i) {
            GameState active;
            initializeGameState(&active);
            if (isSolvable(&active)) {
                //std::cout << "State " << i << " is solvable." << std::endl;
            } else {
                //std::cout << "State " << i << " is unsolvable." << std::endl;
                unsolvableCount++;
            }
        }
    }
    auto totalEnd = chrono::steady_clock::now();
    auto totalDuration = chrono::duration_cast<chrono::milliseconds>(totalEnd - totalStart);
    std::cout << "Total time: " << totalDuration.count() << " milliseconds." << std::endl;

    std::cout << "There are " << unsolvableCount << " unsolvable states." << std::endl;

    return 0;
}

