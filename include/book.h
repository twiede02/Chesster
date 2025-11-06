#pragma once

#include <cstdlib>
#include <ctime> 
#include <fstream> 
#include <iostream> 
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "random.h"
#include "board.h"


class TrieNode {
    public:
        std::unordered_map<std::string, TrieNode *> children;
        std::vector<std::string> moves;

        TrieNode() {}

        ~TrieNode() {
            for (auto &child : children) {
                delete child.second;
            }
        }
};
class Trie {
    private:
        TrieNode *root;

    public:
        Trie() { root = new TrieNode(); }
        void insert(std::vector<std::string> insert_moves) {
            TrieNode *node = root;
            for (std::string move : insert_moves) {
                if (!node->children[move]) {
                    node->children[move] = new TrieNode();
                    node->moves.push_back(move);
                }
                node = node->children[move];
            }
        }

        std::string get_random_next_move(std::vector<Move> history) {
            TrieNode *node = root;
            for (Move move : history) {
                std::string move_string =
                    get_coords_from_index(move.from) + get_coords_from_index(move.to);
                std::cout << move_string << "\n";
                if (!node->children[move_string]) {
                    return "";
                }
                node = node->children[move_string];
            }
            if (node->moves.size() == 0)
                return "";

            return node->moves[dis(gen) % node->moves.size()];
        }
};

Trie my_book;
Position p("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

bool load_book(std::string filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::vector<std::string> moveSequence;
        std::string move;

        // Assuming the line format is "move1 move2 move3 ... moveN"
        while (iss >> move) {
            moveSequence.push_back(move);
        }
        my_book.insert(moveSequence);
    }

    file.close();
    return true;
}
