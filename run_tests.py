#! /usr/bin/env python3

import subprocess
import re
import time
import argparse

def run_perft_tests(engine_path):
    test_cases = [
        ("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 1, 20, "Start Pos Depth 1"),
        ("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 2, 400, "Start Pos Depth 2"),
        ("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 3, 8902, "Start Pos Depth 3"),
        ("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 4, 197281, "Start Pos Depth 4"),
        ("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 5, 4865609, "Start Pos Depth 5"),
        ("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 6, 119060324 , "Start Pos Depth 6"),
        ("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 7, 3195901860 , "Start Pos Depth 7"),
        ("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ", 1, 48 , "Pos 2 Depth 1"),
        ("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ", 2, 2039 , "Pos 2 Depth 2"),
        ("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ", 3, 97862 , "Pos 2 Depth 3"),
        ("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ", 4, 4085603 , "Pos 2 Depth 4"),
        ("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ", 5, 193690690 , "Pos 2 Depth 5"),
        ("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ", 6, 8031647685 , "Pos 2 Depth 6"),
        ("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ", 1, 14, "Pos 3 Depth 1"),
        ("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ", 2, 191, "Pos 3 Depth 2"),
        ("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ", 3, 2812, "Pos 3 Depth 3"),
        ("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ", 4, 43238, "Pos 3 Depth 4"),
        ("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ", 5, 674624, "Pos 3 Depth 5"),
        ("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ", 6, 11030083, "Pos 3 Depth 6"),
        ("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ", 7, 178633661, "Pos 3 Depth 7"),
        ("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ", 8, 3009794393, "Pos 3 Depth 8"),
        ("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 1, 6, "Pos 4 Depth 1"),
        ("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 2, 264, "Pos 4 Depth 2"),
        ("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 3, 9467, "Pos 4 Depth 3"),
        ("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 4, 422333, "Pos 4 Depth 4"),
        ("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 5, 15833292, "Pos 4 Depth 5"),
        ("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 6, 706045033, "Pos 4 Depth 6"),
        ("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 1, 44, "Pos 5 Depth 1"),
        ("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 2, 1486, "Pos 5 Depth 2"),
        ("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 3, 62379, "Pos 5 Depth 3"),
        ("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 4, 2103487, "Pos 5 Depth 4"),
        ("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 5, 89941194, "Pos 5 Depth 5"),
        ("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 1, 46, "Pos 6 Depth 1"),
        ("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 2, 2079, "Pos 6 Depth 2"),
        ("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 3, 89890, "Pos 6 Depth 3"),
        ("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 4, 3894594, "Pos 6 Depth 4"),
        ("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 5, 164075551, "Pos 6 Depth 5"),
        ("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 6, 6923051137, "Pos 6 Depth 6"),
        ("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 7, 287188994746, "Pos 6 Depth 7"),
        ("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 8, 11923589843526, "Pos 6 Depth 8"),
        ("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 9, 490154852788714, "Pos 6 Depth 9"),
    ]
    
    # Start the engine process
    engine = subprocess.Popen(
        engine_path,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL,
        text=True
    )
    
    def send_command(command):
        engine.stdin.write(command + "\n")
        engine.stdin.flush()
    
    def wait_for_output(expected):
        while True:
            line = engine.stdout.readline().strip()
            if line == expected:
                break
    
    def read_response():
        output = []
        while True:
            line = engine.stdout.readline().strip()
            if "Nodes" in line:
                output.append(line)
                break
            output.append(line)
        return "\n".join(output)
    
    # Send UCI initialization commands
    send_command("uci")
    while True:
        line = engine.stdout.readline().strip()
        if line == "uciok":
            break
    
    send_command("isready")
    wait_for_output("readyok")
    
    print("\n=== Perft Test Results ===")
    print("Test Name                      | Status  | Time (s)")
    print("---------------------------------------------")
    
    # Run test cases directly
    for fen, depth, expected_nodes, test_name in test_cases:
        if args.mode == "fast" and expected_nodes > 1000000:
            continue
        send_command(f"position fen {fen}")
        
        start_time = time.time()
        send_command(f"go perft {depth}")
        response = read_response()
        end_time = time.time()
        
        elapsed_time = end_time - start_time
        
        match = re.search(r"Nodes:\s*(\d+)", response)
        if match and int(match.group(1)) == expected_nodes:
            print(f"{test_name:<30} | Passed  | {elapsed_time:.3f}")
        else:
            print(f"{test_name:<30} | Failed  | {elapsed_time:.3f}")
            print(f"Expected: {expected_nodes}, Got: {match.group(1) if match else 'N/A'}")
            print(f"Response:\n{response}\n")
        
    print("=============================================")
    
    # Close the engine process
    send_command("quit")
    engine.terminate()

# Parse command-line arguments
parser = argparse.ArgumentParser(description="Run tests in fast or slow mode.")
parser.add_argument("mode", choices=["fast", "slow"], help="Choose mode: fast or slow")
args = parser.parse_args()

# Example usage, replace 'your_engine_path'
run_perft_tests("./build/chesster")

