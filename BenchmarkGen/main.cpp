#include <iostream>
#include <fstream>
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

using namespace std;

void printDeck(vector<int> deck) {
    for (int card : deck) {
        cout << card << " ";
    }
    cout << endl;
}

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

vector<vector<int>> loadDecks() {
    vector<vector<int>> decks;
    ifstream file("benchmarkDecks.txt");
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

int main() {
    // Create and open a text file
    ofstream MyFile("benchmarkDecks.txt");

    for (int i = 0; i < 5000; i++) {
        vector<int> deck = generateDeck();
        for (int i = 0 ; i < 52; i++) {
            MyFile << deck[i] << " ";
        }

        MyFile << endl;
    }

    // Close the file
    MyFile.close();

    vector<vector<int>> decks = loadDecks();
    printDeck (decks[0]);
    printDeck (decks[2000]);

    return 0;
}