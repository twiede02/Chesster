#pragma once

#include <search.h>
#include "move.h"
#include "zobrist.h"
#include "book.h"
#include "perft.h"

inline void handleSetOption(const std::string &optionName,
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

inline void handlePosition(const std::string &positionData) {
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
            if (token.size() == 5) {
                switch (token[4]) {
                    case 'q': {
                        Move m(Square(file1, rank1), Square(file2, rank2), MoveType::Promotion, Piece::Queen);
                        p.make_move(m);
                        break; }
                    case 'n': {
                        Move m(Square(file1, rank1), Square(file2, rank2), MoveType::Promotion, Piece::Knight);
                        p.make_move(m);
                        break; }
                    case 'r': {
                        Move m(Square(file1, rank1), Square(file2, rank2), MoveType::Promotion, Piece::Rook);
                        p.make_move(m);
                        break; }
                    case 'b': {
                        Move m(Square(file1, rank1), Square(file2, rank2), MoveType::Promotion, Piece::Bishop);
                        p.make_move(m);
                        break; }
                    default:
                        break;
                }
            } else {
                Move m(Square(file1, rank1), Square(file2, rank2));
                p.make_move(m);
            }
        }
    }
}

inline void handleGo(const std::string &goData) {
    // Implement move calculation logic
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

    Move m;
    auto deadline = std::chrono::high_resolution_clock::now() + time_limit;

    for (int i = 1;; i++) {
        auto search_res = search(p, i, deadline);
        if (!search_res)
            break;

        m = *search_res;
    }

    std::cout << "bestmove "
        << m.from().to_string()
        << m.to().to_string();
    if (m.type() == MoveType::Promotion)
        std::cout << to_string(m.promotedPiece());
    std::cout << "\n";
}

inline void uciloop() {
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
            initialize_zobrist();
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
        } else if (input.empty()) {
            break;
        } else {
            std::cerr << "[Unknown command: " << input << "]" << std::endl;
        }
    }
}

