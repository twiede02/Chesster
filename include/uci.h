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

#include "book.h"
#include "perft.h"

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
