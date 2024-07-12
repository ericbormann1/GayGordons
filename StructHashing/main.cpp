/*
This program determines the solvability of Gay Gordons, a card game where a deck is shuffled and 
distributed into ten piles of 5 cards, with the 2 remaining cards going into a reserve pile. 
You are trying to make pairs of 11, with ace going with 10, king going with queen, and jack going with jack. 
Color and suit do not matter. Only the top cards of each stack are in play.
 */
const bool BENCHMARKING = true;
const bool DEBUG = false;
const bool ADVANCEDEBUG = false;
// numSims is 5000 in conventional benchmarking
const int numSimulations = 1;

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
#include <set>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <thread>
#include <mutex>

using namespace std;

void printPilesAndReserve(const vector<vector<int>>& piles, const vector<int>& reserve) {
    size_t max_cards = 0;
    for (const auto& pile : piles) {
        max_cards = max(max_cards, pile.size());
    }

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

bool isPair(int card1, int card2) {
    int rank1 = card1 % 13;
    int rank2 = card2 % 13;

    if ((rank1 + rank2 == 9) || (rank1 == 12 && rank2 == 11) || (rank1 == 11 && rank2 == 12) || (rank1 == 10 && rank2 == 10)) {
        return true;
    }
    return false;
}

bool hasThreeJacks(const vector<vector<int>>& piles) {
    for (const auto& pile : piles) {
        int countJacks = 0;
        for (int card : pile) {
            if (card % 13 == 10) {
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

struct GameState {
    vector<vector<int>> piles;
    vector<int> reserve;

    bool operator==(const GameState& other) const {
        return piles == other.piles && reserve == other.reserve;
    }
};

struct GameStateHasher {
    size_t operator()(const GameState& state) const {
        size_t hash = 0;
        for (const auto& pile : state.piles) {
            for (int card : pile) {
                hash ^= hash * 31 + card;
            }
        }
        for (int card : state.reserve) {
            hash ^= hash * 31 + card;
        }
        return hash;
    }
};

bool solve(vector<vector<int>>& piles, vector<int>& reserve, unordered_set<GameState, GameStateHasher>& visited) {
    bool allEmpty = all_of(piles.begin(), piles.end(), [](const vector<int>& pile) { return pile.empty(); }) && reserve.empty();
    if (allEmpty) return true;

    GameState currentState = {piles, reserve};

    if (visited.find(currentState) != visited.end()) return false;
    visited.insert(currentState);

    for (size_t i = 0; i < piles.size(); ++i) {
        if (!piles[i].empty()) {
            for (size_t j = i + 1; j < piles.size(); ++j) {
                if (!piles[j].empty() && isPair(piles[i].back(), piles[j].back())) {
                    int card1 = piles[i].back();
                    int card2 = piles[j].back();

                    piles[i].pop_back();
                    piles[j].pop_back();

                    if (solve(piles, reserve, visited)) {
                        if (ADVANCEDEBUG) {
                            // If a solution is found, print the pair that was found and the piles they were
                            std::cout << "Pair found: " << card1%13 << " and " << card2%13 << "from piles " << i << " and" << j << endl;
                            // For debugging purposes, print the updated piles and reserve
                            piles[i].push_back(card1);
                            piles[j].push_back(card2);
                            printPilesAndReserve(piles, reserve);
                            // If a solution is found, return true
                        }
                        return true;
                    }

                    piles[i].push_back(card1);
                    piles[j].push_back(card2);
                }
            }
        }
    }

    if (!reserve.empty()) {
        for (size_t i = 0; i < piles.size(); ++i) {
            if (!piles[i].empty() && isPair(piles[i].back(), reserve.back())) {
                int card1 = piles[i].back();
                int card2 = reserve.back();

                reserve.pop_back();
                piles[i].pop_back();

                if (solve(piles, reserve, visited)) {
                    if (ADVANCEDEBUG) {
                        // If a solution is found, print the pair that was found and the piles they were
                        std::cout << "Pair found: " << card1%13 << " and " << card2%13 << "from piles " << i << " and reserve" << endl;
                        // For debugging purposes, print the updated piles and reserve
                        reserve.push_back(card2);
                        piles[i].push_back(card1);
                        printPilesAndReserve(piles, reserve);
                    }
                    // If a solution is found, return true
                    return true;
                }

                reserve.push_back(card2);
                piles[i].push_back(card1);
            }
        }
    }

    return false;
}

bool isSolvable(const vector<int>& deck) {
    vector<vector<int>> piles(10);
    vector<int> reserve(deck.end() - 2, deck.end());

    int deckIndex = 0;
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 5; ++j) {
            piles[i].push_back(deck[deckIndex++]);
        }
    }

    if (DEBUG) { printPilesAndReserve(piles, reserve); }

    if (hasThreeJacks(piles)) {
        cout << "Impossible to solve (3 Jacks in a stack)." << endl;
        return false;
    }

    unordered_set<GameState, GameStateHasher> visited;


    return solve(piles, reserve, visited);
}

vector<int> generateDeck() {
    vector<int> deck(52);
    iota(deck.begin(), deck.end(), 0);

    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    default_random_engine randomNumberGenerator(seed);

    shuffle(deck.begin(), deck.end(), randomNumberGenerator);

    return deck;
}

void printDeck(vector<int> deck) {
    for (int card : deck) {
        std::cout << card << " ";
    }
    std::cout << endl;
}
// A function to load the 5000 decks  from a file, each number representing a card is separated by a space and each deck is separated by a new line
vector<vector<int>> loadDecks() {
    vector<vector<int>> decks;
    ifstream file("../BenchmarkGen/benchmarkDecks.txt");
    string line;

    // Use a while loop together with the getline() function to read the file line by line
    while (getline (file, line)) {
        vector <int> deck;
        stringstream ss(line);
        string card;
        while (getline(ss, card, ' ')) {
            deck.push_back(stoi(card));
        }
        decks.push_back(deck);
    }
    file.close();
    return decks;
}

void simulateGames(int numSimsThisThread, int offset, int& unsolvableCount, mutex& mtx, vector<vector<int>>& decks) {
    
    if (BENCHMARKING) {
        for (int i = 0; i < numSimsThisThread; ++i) {
            if (DEBUG) {cout << "Simulation " << offset + i << "begins (On thread: " << this_thread::get_id() << ")" << endl;
            auto start = chrono::steady_clock::now(); }

            if (!isSolvable(decks[offset + i])) {
                lock_guard<mutex> lock(mtx);
                unsolvableCount++;
                if (DEBUG) cout << "Game " << offset + i << " is unsolvable. On thread" << this_thread::get_id() << endl;
            }

            if (DEBUG) {
                auto end = chrono::steady_clock::now();
                //auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
                //cout << "Simulation " << offset + i << " took " << duration.count() << " milliseconds. On thread " << this_thread::get_id() << endl;
                cout << "Unsolvable count: " << unsolvableCount << endl;
            }
        }
    } else {
        for (int i = 0; i < numSimsThisThread; ++i) {
            if (DEBUG) {cout << "Simulation " << i + 1 << "begins (On thread: " << this_thread::get_id() << ")" << endl;
                auto start = chrono::steady_clock::now();
                vector<int> deck = generateDeck();

                if (!isSolvable(deck)) {
                    lock_guard<mutex> lock(mtx);
                    unsolvableCount++;
                    cout << "Game " << i + 1 << " is unsolvable. On thread" << this_thread::get_id() << endl;
                }
                auto end = chrono::steady_clock::now();
                auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
                cout << "Simulation " << i + 1 << " took " << duration.count() << " milliseconds. On thread " << this_thread::get_id() << endl;
                cout << "Unsolvable count: " << unsolvableCount << endl;
            } else {
                vector<int> deck = generateDeck();
                if (!isSolvable(deck)) {
                    lock_guard<mutex> lock(mtx);
                    unsolvableCount++;
                }
            }
        }
    }
}


int main() {
    vector<vector<int>> decks;
    if (BENCHMARKING)
    {
        cout << "Decks are being loaded..." << endl;
        decks = loadDecks();
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