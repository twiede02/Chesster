#pragma once

#include <search.h>
#include "attack_masks.h"
#include "board.h"
#include "move.h"
#include "movegen.h"
#include "square.h"
#include "utils.h"
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
        // fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ";
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
                        int i1 = file1 + 8 * rank1;
                        int i2 = file2 + 8 * rank2;
                        assert(i1 >= 0 && i1 < 64);
                        assert(i2 >= 0 && i2 < 64);
                        Move m(Square(i1), Square(i2), MoveType::Promotion, Piece::Queen);
                        p.make_move(m);
                        break; }
                    case 'n': {
                        int i1 = file1 + 8 * rank1;
                        int i2 = file2 + 8 * rank2;
                        assert(i1 >= 0 && i1 < 64);
                        assert(i2 >= 0 && i2 < 64);
                        Move m(Square(i1), Square(i2), MoveType::Promotion, Piece::Knight);
                        p.make_move(m);
                        break; }
                    case 'r': {
                        int i1 = file1 + 8 * rank1;
                        int i2 = file2 + 8 * rank2;
                        assert(i1 >= 0 && i1 < 64);
                        assert(i2 >= 0 && i2 < 64);
                        Move m(Square(i1), Square(i2), MoveType::Promotion, Piece::Rook);
                        p.make_move(m);
                        break; }
                    case 'b': {
                        int i1 = file1 + 8 * rank1;
                        int i2 = file2 + 8 * rank2;
                        assert(i1 >= 0 && i1 < 64);
                        assert(i2 >= 0 && i2 < 64);
                        Move m(Square(i1), Square(i2), MoveType::Promotion, Piece::Bishop);
                        p.make_move(m);
                        break; }
                    default:
                        break;
                }
            } else {
                int i1 = file1 + 8 * rank1;
                int i2 = file2 + 8 * rank2;
                assert(i1 >= 0 && i1 < 64);
                assert(i2 >= 0 && i2 < 64);
                Move m(Square(i1), Square(i2), MoveType::Normal);
                // TODO: detect castling here
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
        } else if (input == "test") {
            p = Position("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ");
            std::cout << "\n";
            print_position(p);

            Move m(Square(Square::Value::G2), Square(Square::Value::G4), MoveType::Normal, Piece::Knight);
            Movelog l = p.make_move(m);
            std::cout << "confirm " << to_string(l);
            std::cout << "just made move" << std::endl;
            print_position(p);
            std::cout << "move valid" << std::endl;
            p.unmake_move(l);
            std::cout << "unmade move" << std::endl;
            // Perft myp(p);
            // uint64_t res = myp.run_fast(4);
            // std::cout << "Nodes: " << res << std::endl;

            print_position(p);
            std::cout << "unmove valid" << std::endl;
            std::cout << "\n";
        } else if (input.empty()) {
            break;
        } else {
            std::cerr << "[Unknown command: " << input << "]" << std::endl;
        }
    }
}

