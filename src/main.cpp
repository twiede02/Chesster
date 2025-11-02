#include <chrono>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <random>
#include <search.h>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "board.h"
#include "evaluate.h"
#include "movegen.h"
#include "perft.h"
#include "utils.h"

std::mt19937 gen(static_cast<unsigned int>(
    std::chrono::system_clock::now().time_since_epoch().count()));
std::uniform_int_distribution<> dis(1, 1000);

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
// Position p("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ");

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

    // std::cout << "IN" << std::endl;
  }
  // std::cout << "CHECK" << std::endl;

  file.close();
  return true;
}
void handleSetOption(const std::string &optionName,
                     const std::string &optionValue) {
  if (optionName == "Threads") {
    std::cout << "info string Threads option is fixed to 1" << std::endl;
  } else if (optionName == "Hash") {
    std::cout << "info string Hash option is not supported and will be ignored"
              << std::endl;
  } else if (optionName == "BookFile") {
    std::string book_file_path = optionValue;
    load_book(book_file_path);
  } else {
    std::cout << "info string Option " << optionName << " not recognized."
              << std::endl;
  }
}

void handlePosition(const std::string &positionData) {
  std::istringstream iss(positionData);
  std::string token;
  iss >> token; // position type

  bool startpos_flag = false;
  std::string fen = "";
  if (token == "startpos") {
    startpos_flag = true;
    iss >> token;
  } else {
    while (iss >> token && token != "moves") {
      fen += token + " ";
    }
  }
  if (startpos_flag) {
    fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  }
  p = Position(fen);
  if (token == "moves") {
    while (iss >> token) {
      char file1 = token[0];
      char file2 = token[2];
      int rank1 = token[1] - '0';
      int rank2 = token[3] - '0';
      Move m(get_index(file1, rank1), get_index(file2, rank2));
      if (token.size() == 5) {
        switch (token[4]) {
        case 'q':
          m.promotion = Piece::Queen;
          break;
        case 'n':
          m.promotion = Piece::Knight;
          break;
        case 'r':
          m.promotion = Piece::Rook;
          break;
        case 'b':
          m.promotion = Piece::Bishop;
          break;
        default:
          break;
        }
      }
      p.make_move(m);
    }
  }
}

void handleGo(const std::string &goData) {
  // Implement move calculation logic
  if (p.move_history.size() < 16) {
    std::string res = my_book.get_random_next_move(p.move_history);
    if (res != "") {
      std::cout << "bestmove " << res << std::endl;
      return;
    }
  }

  std::istringstream iss(goData);
  std::string token;
  TimeControl tc;

  while (iss >> token) {
    if (token == "wtime") {
      int value;
      iss >> value;
      tc.wtime = value;
    } else if (token == "btime") {
      int value;
      iss >> value;
      tc.btime = value;
    } else if (token == "movestogo") {
      int value;
      iss >> value;
      tc.movestogo = value;
    } else if (token == "movetime") {
      int value;
      iss >> value;
      tc.movetime = value;
    }
  }
  std::chrono::milliseconds time_limit;
  if (p.side_to_move == Color::White) {
    if (tc.wtime)
      time_limit = std::chrono::milliseconds(*tc.wtime / 20);
  } else {
    if (tc.btime)
      time_limit = std::chrono::milliseconds(*tc.btime / 20);
  }
  if (tc.movetime)
    time_limit += std::chrono::milliseconds(*tc.movetime / 2);

  Move m(0, 0);
  auto deadline = std::chrono::high_resolution_clock::now() + time_limit;

  for (int i = 1;; i++) {
    auto search_res = search(p, i, deadline);
    if (!search_res)
      break;

    m = *search_res;
  }

  std::cout << "bestmove " << get_coords_from_index(m.from)
            << get_coords_from_index(m.to);
  switch (m.promotion) {
  case Piece::Queen:
    std::cout << "q";
    break;
  case Piece::Knight:
    std::cout << "n";
    break;
  case Piece::Rook:
    std::cout << "r";
    break;
  case Piece::Bishop:
    std::cout << "b";
    break;
  default:
    break;
  }
  std::cout << "\n";
}

void uciloop() {
  std::string input;
  while (true) {
    std::getline(std::cin, input);
    if (input == "uci") {
      std::cout << "id name Chesster\n";
      std::cout << "id author MyName\n";
      std::cout << "option name Threads type spin default 1 min 1 max 1\n";
      std::cout << "option name Hash type spin default 16 min 1 max 2048\n";
      std::cout << "option name BookFile type string default" << std::endl;
      load_book("../books/8move_balanced.txt");
      std::cout << "uciok\n";
    } else if (input == "isready") {
      std::cout << "readyok\n";
    } else if (input.rfind("setoption", 0) == 0) {
      // Parse setoption command
      std::istringstream iss(input);
      std::string token;
      std::string optionName;
      std::string optionValue;

      // Split the input into words
      while (iss >> token) {
        if (token == "name") {
          iss >> optionName;
        } else if (token == "value") {
          iss >> optionValue;
        }
      }

      handleSetOption(optionName, optionValue);
    } else if (input.rfind("position", 0) == 0) {
      // Parse position command
      std::string positionData = input.substr(9); // Extract position data
      handlePosition(positionData);
    } else if (input.rfind("go perft", 0) == 0) {
      std::string depth = input.substr(9);
      Perft myp(p);
      uint64_t res = myp.run_fast(std::stoi(depth));
      std::cout << "Nodes: " << res << std::endl;
    } else if (input.rfind("go", 0) == 0) {
      // Parse go command
      std::string goData =
          input.size() > 2 ? input.substr(3) : ""; // Extract go parameters
      handleGo(goData);
    } else if (input == "quit") {
      break;
    } else {
      std::cerr << "Unknown command: " << input << std::endl;
    }
  }
}

void initialize_zobrist() {
  std::uniform_int_distribution<uint64_t> uniform_dist;

  // Populate the Zobrist table with random 64-bit values
  for (int piece = 0; piece < 12; ++piece) {
    for (int square = 0; square < 64; ++square) {
      zobrist_table[piece][square] = uniform_dist(gen);
    }
  }
}

int main() {
  initialize_zobrist();
  uciloop();
  return 0;
}
