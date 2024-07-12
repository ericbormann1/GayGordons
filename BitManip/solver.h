#pragma once

#include "gameState.h"

// helper function to check if two given cards are a pair
/*0-12: 0 (ace) + 9 (10) = 9. 
    1       1 (2) + 8 (9) = 9. 
    2       2 (3)+ 7(8) = 9
    3       3 (4) + 6 (7) = 9
    4       4 (5) + 5 (6) = 9
    5       5 (6) + 4 (5) = 9
    */
inline bool isPair(int card1, int card2) {
    if ((card1 + card2 == 9) || (card1 == 12 && card2 == 11) || (card1 == 11 && card2 == 12) || (card1 == 10 && card2 == 10)) {
        return true;
    }
    return false;
}

// helper function to print the in play (first) card from a given pile, return 15 if the column is empty
inline int getTopPileCard(GameState* state, int column) {
    if ((state->piles[column] & 0x0F) == 0x0F) {
        return 15;
    }
    return (state->piles[column] & 0x0F);
}

// helper function to print the in play (first) card from reserve, return 15 if the reserve is empty
inline int getTopReserveCard(GameState* state) {
    if ((state->reserve & 0x0F) == 0x0F) {
        return 15;
    }
    return (state->reserve & 0x0F);
}

/* helper function to remove the top card from a given pile, it returns the card it just removed, 15 if the column is empty. It shifts the remaining cards 
 to the left, and sets the new space to 0x0F.*/
inline int removeTopPileCard(GameState* state, int column) {
    if ((state->piles[column] & 0x0F) == 0x0F) {
        return 15;
    }
    int card = (state->piles[column] & 0x0F);
    state->piles[column] = (state->piles[column] >> 4);
    state->piles[column] |= 0x0F << (4 * 7);
    return card;
}

// helper function to remove the top card from reserve, it returns the card it just removed, 15 if the reserve is empty
// it shifts the remaining cards to the left, and sets the new space to 0x0F
inline int removeTopReserveCard(GameState* state) {
    if ((state->reserve & 0x0F) == 0x0F) {
        return 15;
    }
    int card = (state->reserve & 0x0F);
    state->reserve = (state->reserve >> 4);
    state->reserve |= 0x0F << (4 * 1);
    return card;
}

// helper function to add a card to a given pile
inline void addPileCard(GameState* state, int column, int card) {
    state->piles[column] = (state->piles[column] << 4);
    state->piles[column] |= card;
}

//helper function to add a card to reserve
inline void addReserveCard(GameState* state, int card) {
    state->reserve = (state->reserve << 4);
    state->reserve |= card;
}

