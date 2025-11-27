#!/usr/bin/env python3

# this uses the lichess data base and is due to its size not in the repo
# https://database.lichess.org/#puzzles

import csv
import subprocess
import argparse
import random
import time

seed = int(time.time_ns())
random.seed(seed)

RED = "\033[31m"
GREEN = "\033[32m"
RESET = "\033[0m"

total_load = 10
total_tests = 10
random_testing = False
movetime = 300

parser = argparse.ArgumentParser(description="Runs puzzles, optional flags are '--load', '--use', '--movetime'")
parser.add_argument('--load', type=int, help='How many puzzles should be loaded')
parser.add_argument('--use', type=int, help='How many puzzles should be testet')
parser.add_argument('--movetime', type=int, help='Engine\'s time per move in ms')
parser.add_argument('--engine_path', type=str, required=True, help='Path to the engine')
parser.add_argument('--table_path', type=str, required=True, help='Path to the puzzles table')

args = parser.parse_args()

engine_path = args.engine_path
file_path = args.table_path

if args.load:
    total_load = args.load
if args.use:
    total_tests = args.use
if args.movetime:
    movetime = args.movetime

if total_load < total_tests:
    total_load = total_tests
if total_tests != total_load:
    random_testing = True


def load_column_from_csv(file_path, column_number, limit):
    data = []

    try:
        with open(file_path, mode='r', newline='') as csv_file:
            csv_reader = csv.reader(csv_file)

            # Skip the very first row
            next(csv_reader, None)

            for row in csv_reader:
                if column_number < len(row):
                    data.append(row[column_number])
                
                if len(data) >= limit:
                    break

        return data

    except FileNotFoundError:
        print(f"Error: The file {file_path} was not found.")
        return None
    except Exception as e:
        print(f"An error occurred: {e}")
        return None


fen_data = load_column_from_csv(file_path, 1, total_load)
move_data = load_column_from_csv(file_path, 2, total_load)

if fen_data is not None:
    print("Loaded FEN Column:", fen_data)

if move_data is not None:
    print("Loaded Move Column:", move_data)

def start_engine():
    return subprocess.Popen(
        [engine_path],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        text=True,
        bufsize=1
    )

def send_cmd(engine, cmd):
    engine.stdin.write(cmd + "\n")

def read_bestmove(engine):
    """Read until the 'bestmove' line appears."""
    while True:
        line = engine.stdout.readline().strip()
        if line.startswith("bestmove"):
            return line.split()[1]  # returns the move only


if fen_data is not None and move_data is not None:

    engine = start_engine()

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
    
    send_command("uci")
    while True:
        line = engine.stdout.readline().strip()
        if line == "uciok":
            break
    
    send_command("isready")
    wait_for_output("readyok")

    if random_testing:
        print(f"Using seed {seed}\n")
    else:
        print(f"No random testing, testing the first {total_tests} tests\n")

    print("\nStarting evaluation...\n")

    correct_counter = 0

    i = 0

    start = time.perf_counter()

    for j in range(total_tests):
    # for fen, solution in zip(fen_data, move_data):
        if random_testing:
            i = random.randrange(0, total_tests)
        else:
            i = j

        fen = fen_data[i]
        solution = move_data[i]

        print("======================================")
        print("FEN:", fen)
        print("Solution:", solution)

        solution_moves = solution.split()
        play_history = [solution_moves[0]]

        puzzle_correct = True

        for i in range(1, len(solution_moves)):
            cmd = f"position fen {fen} moves {' '.join(play_history)}"
            send_cmd(engine, cmd)

            if i % 2 == 1:
                send_cmd(engine, f"go movetime {movetime}")
                bestmove = read_bestmove(engine)
                expected = solution_moves[i]

                print(f"Engine move: {bestmove}, expected: {expected}")

                if bestmove != expected:
                    print(f"{RED}✘ Incorrect move!{RESET}")
                    puzzle_correct = False
                    break

                play_history.append(bestmove)
            else:
                play_history.append(solution_moves[i])

        if puzzle_correct:
            print(f"{GREEN}✔ Puzzle solved completely!{RESET}")
            correct_counter += 1


end = time.perf_counter()
print(f"\nTook {end - start:.3f} seconds\n")

print(f"Solved {correct_counter} out of {total_tests}", end="")
if random_testing:
    print(f", randomly chosen from {total_load}")
else:
    print()

