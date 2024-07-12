#pragma once

#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <numeric>
#include <random>
#include <cstring>

#include "gameState.h"

using namespace std;

const int NUM_PILES = 10;
const int PILE_SIZE = 5;
const int RESERVE_SIZE = 2;

// Helper function to convert a given game state's piles into a vector of integer vectors (2D vector)
vector<vector<int>> convertPilesToVector(const GameState& state) {
    vector<vector<int>> piles(NUM_PILES);
    for (int i = 0; i < NUM_PILES; ++i) {
        for (int j = 0; j < PILE_SIZE; ++j) {
            int card = (state.piles[i] >> (j * 4)) & 0x0F;
            if (card != 0x0F) {
                piles[i].push_back(card);
            }
        }
    }
    for (int i = 0; i < NUM_PILES; ++i) {
        std::reverse(piles[i].begin(), piles[i].end());
    }
    return piles;
}

// Helper function to convert a given game state's reserve into a vector of integers
vector<int> convertReserveToVector (const GameState& state) {
    vector<int> reserve;
    for (int i = 0; i < RESERVE_SIZE; ++i) {
        int card = (state.reserve >> (i * 4)) & 0x0F;
        if (card != 0x0F) {
            reserve.push_back(card);
        }
    }
    std::reverse(reserve.begin(), reserve.end());
    return reserve;
}

// Function to print the entire game state
void printGameState(const GameState& state) {
    // Convert the state into a 2D vector
    vector<vector<int>> piles = convertPilesToVector(state);
    vector<int> reserve = convertReserveToVector(state);

    size_t max_cards = 0;
    for (const auto& pile : piles) {
        max_cards = max(max_cards, pile.size());
    }

    // Print header for piles
    std::cout << "| ";
    for (size_t j = 0; j < piles.size(); ++j) {
        std::cout << "P" << j << " ";
    }
    std::cout << "| " << "Reserve" << endl;

    for (size_t i = 0; i < max_cards; ++i) {
        std::cout << "| ";
        for (size_t j = 0; j < piles.size(); ++j) {
            if (i < piles[j].size()) {
                std::cout << setw(2) << setfill(' ') << piles[j][i] % 13 << " ";
            } else {
                std::cout << "   ";
            }
        }
        std::cout << "| ";
        if (i < reserve.size()) {
            std::cout << setw(2) << setfill(' ') << reserve[i] % 13;
        }
        std::cout << endl;
    }
    std::cout << endl;
}