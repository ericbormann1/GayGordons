// This program is attempting to brute force the solvability of the game Gay Gordons, a card game.

#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <numeric>
#include <stack>
#include <functional>
#include <unordered_set>
#include <string>
#include <sstream>
#include <set>
#include <iomanip>

using namespace std;


void printPilesAndReserve(const vector<vector<int>>& piles, const vector<int>& reserve) {
    // Determine the maximum number of cards in a pile
    size_t max_cards = 0;
    for (const auto& pile : piles) {
        max_cards = max(max_cards, pile.size());
    }

    // Print column headers
    cout << "| ";
    for (size_t j = 0; j < piles.size(); ++j) {
        cout << "P" << j << " ";
    }
    cout << "| " << "Reserve" << endl;

    // Print each pile and reserve vertically
    for (size_t i = 0; i < max_cards; ++i) {
        cout << "| ";
        for (size_t j = 0; j < piles.size(); ++j) {
            if (i < piles[j].size()) {
                cout << setw(2) << setfill(' ') << piles[j][i] % 13<< " ";
            } else {
                cout << "   ";
            }
        }
        cout << "| ";
        if (i < reserve.size()) {
            cout << setw(2) << setfill(' ') << reserve[i] % 13;
        }
        cout << endl;
    }
    cout << endl;
}

bool isPair(int card1, int card2) {
    /*0-12: 0 (ace) + 9 (10) = 9. 
    1       1 (2) + 8 (9) = 9. 
    2       2 (3)+ 7(8) = 9
    3       3 (4) + 6 (7) = 9
    4       4 (5) + 5 (6) = 9
    5       5 (6) + 4 (5) = 9
    */
    int rank1 = card1 % 13; 
    int rank2 = card2 % 13;

    // Pairing logic: A + 10, K + Q, J + J
    if ((rank1 + rank2 == 9) || // Ace + 10 (equiv to 0 + 9) is 9, same as 3+8 (equiv to 2+7) is 9
        (rank1 == 12 && rank2 == 11) || // King + Queen
        (rank1 == 11 && rank2 == 12) || // Queen + King
        (rank1 == 10 && rank2 == 10)) { // Jack + Jack
        return true;
    }
    return false;
}

// Function to check if there are three Jacks stacked on top of each other in any pile
bool hasThreeJacks(const vector<vector<int>>& piles) {
    for (const auto& pile : piles) {
        int countJacks = 0;
        for (int card : pile) {
            if (card % 13 == 10) { // Check if the card is a Jack (10 % 13 == 10)
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

// Recursive function to solve the Gay Gordons game
bool solve(vector<vector<int>>& piles, vector<int>& reserve, unordered_set<string>& visited) {
    // Check if all piles and reserve are empty, indicating a solved game state
    bool allEmpty = true;
    for (const auto& pile : piles) {
        if (!pile.empty()) {
            allEmpty = false;
            break;
        }
    }
    if (allEmpty && reserve.empty()) return true; // Base case: game is solved

    // Generate a unique state representation
    stringstream ss;
    for (const auto& pile : piles) {
        for (int card : pile) {
            ss << card << ",";
        }
        ss << "/";
    }
    for (int card : reserve) {
        ss << card << ",";
    }
    string state = ss.str();

    // Check if state has been visited
    if (visited.find(state) != visited.end()) return false; // Already visited, no need to recompute
    visited.insert(state); // Mark current state as visited

    // Check pairs between top cards of different piles
    for (size_t i = 0; i < piles.size(); ++i) {
        if (!piles[i].empty()) {
            for (size_t j = i + 1; j < piles.size(); ++j) {
                if (!piles[j].empty() && isPair(piles[i].back(), piles[j].back())) {
                    // Record the cards being paired
                    int card1 = piles[i].back();
                    int card2 = piles[j].back();

                    // Remove the paired cards from piles
                    piles[i].pop_back();
                    piles[j].pop_back();

                    // Recursively solve the game with the updated piles and reserve
                    if (solve(piles, reserve, visited)) {
                        // If a solution is found, print the pair that was found and the piles they were
                        cout << "Pair found: " << card1%13 << " and " << card2%13 << "from piles " << i << " and " << j << endl;
                        // For debugging purposes, push back the cards and print
                        piles[i].push_back(card1);
                        piles[j].push_back(card2);
                        printPilesAndReserve(piles, reserve);
                        // If a solution is found, return true
                        return true;
                    }
                    // Restore the removed cards to piles for backtracking
                    piles[i].push_back(card1);
                    piles[j].push_back(card2);
                }
            }
        }
    }

    // Check pairs between reserve top card and piles top cards
    if (!reserve.empty()) {
        for (size_t i = 0; i < piles.size(); ++i) {
            if (!piles[i].empty() && isPair(piles[i].back(), reserve.back())) {
                // Record the cards being paired
                int card1 = piles[i].back();
                int card2 = reserve.back();

                // Remove the paired cards from piles and reserve
                reserve.pop_back();
                piles[i].pop_back();

                // Recursively solve the game with the updated piles and reserve
                if (solve(piles, reserve, visited)) {
                    // If a solution is found, print the pair that was found and the piles they were
                    cout << "Pair found: " << card1%13 << " and " << card2%13 << "from piles " << i << " and reserve" << endl;
                    // For debugging purposes, print the updated piles and reserve
                    reserve.push_back(card2);
                    piles[i].push_back(card1);
                    printPilesAndReserve(piles, reserve);
                    // If a solution is found, return true
                    return true;
                }
                // Restore the removed cards to piles and reserve for backtracking
                reserve.push_back(card2);
                piles[i].push_back(card1);
            }
        }
    }

    // No solution found for this state, return false
    return false;
}

// Function to check if a given deck is solvable
bool isSolvable(const vector<int>& deck) {
    // Initialize piles (10 piles of cards) and reserve (2 cards)
    vector<vector<int>> piles(10);
    vector<int> reserve(deck.end() - 2, deck.end()); // Last two cards to reserve

    // Distribute cards to piles
    int deckIndex = 0;
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 5; ++j) {
            piles[i].push_back(deck[deckIndex++]);
        }
    }
    printPilesAndReserve(piles, reserve);
    
    // Check if there are three Jacks stacked on top of each other in any pile
    if (hasThreeJacks(piles)) {
        // print that the game state was impossible because of jacks
        cout << "Impossible to solve (3 Jacks in a stack)." << endl;
        return false;
    }

    // Solve the game recursively
    unordered_set<string> visited;
    return solve(piles, reserve, visited);
}


// Function to generate a shuffled deck.
vector<int> generateDeck() {
    // Create a vector with 52 elements
    vector<int> deck(52);
    
    // Fill the vector with values from 0 to 51
    iota(deck.begin(), deck.end(), 0);
    
    // Get the current time as a seed value for the random number generator
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    
    // Create a default random number generator with the seed value
    default_random_engine randomNumberGenerator(seed);
    
    // Shuffle the vector using the random number generator
    shuffle(deck.begin(), deck.end(), randomNumberGenerator);
    
    // Return the shuffled deck
    return deck;
}

// Make a function to print the deck
void printDeck(vector<int> deck) {
    for (int card : deck) {
        cout << card << " ";
    }
    cout << endl;
}

int main() {
    const int numSimulations = 1; // Number of games to simulate
    int unsolvableCount = 0;

    for (int i = 0; i < numSimulations; ++i) {
        cout << "Simulation " << i + 1 << ":" << endl;
        vector<int> deck = generateDeck();

        // Record the start time
        auto start = chrono::steady_clock::now();
        if (!isSolvable(deck)) {
            unsolvableCount++;
            // Print that the game is unsolvable
            cout << "Game " << i + 1 << " is unsolvable." << endl;
        } 
        // Record the end time
        auto end = chrono::steady_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
        cout << "Simulation " << i + 1 << " took " << duration.count() << " milliseconds." << endl;
        cout << "Unsolvable count: " << unsolvableCount << endl;
    }

    double unsolvablePercentage = (double)unsolvableCount / numSimulations * 100;
    cout << "Unsolvable percentage: " << unsolvablePercentage << "%" << endl;

    return 0;
}




/*
// Function to generate a unique state key for memoization
string generateStateKey(const vector<stack<int>>& piles, const stack<int>& reserve) {
    stringstream key;
    for (const auto& pile : piles) {
        if (!pile.empty()) key << pile.top() << ",";
        else key << "X,";
    }
    if (!reserve.empty()) key << reserve.top();
    else key << "X";
    return key.str();
}

// Function to check if the game is solvable using a brute-force method with optimization
bool isSolvable(vector<int> deck) {
    // Memoization set to store visited states
    unordered_set<string> visitedStates;

    // Helper function to perform a move and backtrack if needed
    function<bool()> solve = [&]() -> bool {
        // Check for winning condition: all piles and reserve empty
        bool allEmpty = true;
        for (const auto& pile : piles) {
            if (!pile.empty()) {
                allEmpty = false;
                break;
            }
        }
        if (allEmpty && reserve.empty()) return true;

        // Check for unwinnable condition
        if (hasUnwinnableCondition(piles, reserve)) return false;

        // Generate current state key
        string stateKey = generateStateKey(piles, reserve);
        if (visitedStates.find(stateKey) != visitedStates.end()) return false;
        visitedStates.insert(stateKey);

        // Collect all available cards (top cards of piles and reserve)
        vector<int> availableCards;
        for (int i = 0; i < 10; ++i) {
            if (!piles[i].empty()) availableCards.push_back(piles[i].top());
        }
        if (!reserve.empty()) availableCards.push_back(reserve.top());

        // Try all pairs of available cards
        for (size_t i = 0; i < availableCards.size(); ++i) {
            for (size_t j = i + 1; j < availableCards.size(); ++j) {
                if (isPair(availableCards[i], availableCards[j])) {
                    // Make the pair and continue solving
                    vector<pair<int, int>> toRemove;

                    for (int k = 0; k < 10; ++k) {
                        if (!piles[k].empty() && piles[k].top() == availableCards[i]) {
                            piles[k].pop();
                            toRemove.push_back({k, availableCards[i]});
                            break; // Only remove one card per pile for this pair
                        }
                    }
                    for (int k = 0; k < 10; ++k) {
                        if (!piles[k].empty() && piles[k].top() == availableCards[j]) {
                            piles[k].pop();
                            toRemove.push_back({k, availableCards[j]});
                            break; // Only remove one card per pile for this pair
                        }
                    }
                    if (!reserve.empty() && reserve.top() == availableCards[i]) {
                        reserve.pop();
                        toRemove.push_back({-1, availableCards[i]});
                    } else if (!reserve.empty() && reserve.top() == availableCards[j]) {
                        reserve.pop();
                        toRemove.push_back({-1, availableCards[j]});
                    }

                    if (solve()) return true;

                    // Backtrack: restore reserve first, then piles
                    for (auto it = toRemove.rbegin(); it != toRemove.rend(); ++it) {
                        if (it->first == -1) reserve.push(it->second);
                        else piles[it->first].push(it->second);
                    }
                }
            }
        }

        return false;
    };

    return solve();
}*/