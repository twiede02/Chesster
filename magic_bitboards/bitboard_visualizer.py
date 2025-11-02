
import os
import sys

def visualize_bitboard(bitboard: int):
    print("  +-----------------+")
    for rank in range(7, -1, -1):
        line = f"{rank + 1} |"
        for file in range(8):
            square_index = rank * 8 + file
            bit = (bitboard >> square_index) & 1
            line += " X" if bit else " ."
        line += " |"
        print(line)
    print("  +-----------------+")
    print("    a b c d e f g h")

def wait_for_keypress():
    print("\nPress any key to continue, or 'q' to quit...")

    try:
        import msvcrt
        while True:
            key = msvcrt.getch()
            if key in (b'q', b'Q'):
                return 'q'
            if key:
                return key
    except ImportError:
        import termios
        import tty
        fd = sys.stdin.fileno()
        old_settings = termios.tcgetattr(fd)
        try:
            tty.setraw(fd)
            while True:
                key = sys.stdin.read(1)
                if key in ('q', 'Q'):
                    return 'q'
                if key:
                    return key
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)

def read_bitboards_from_file(filename: str):
    try:
        with open(filename, "r") as file:
            for line_num, line in enumerate(file, start=1):
                line = line.strip().rstrip(",")
                if not line:
                    continue
                try:
                    bitboard = int(line, 16 if line.startswith("0x") else 10)
                except ValueError:
                    print(f"Warning: Skipping invalid number on line {line_num}: {line}")
                    continue

                os.system("cls" if os.name == "nt" else "clear")
                print(f"Bitboard #{line_num}: {line}")
                visualize_bitboard(bitboard)

                key = wait_for_keypress()
                if key == 'q':
                    print("Quitting...")
                    break

    except FileNotFoundError:
        print(f"Error: File '{filename}' not found.")
    except Exception as e:
        print(f"Unexpected error: {e}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(f"Usage: python {sys.argv[0]} <filename>")
        sys.exit(1)

    filename = sys.argv[1]
    read_bitboards_from_file(filename)

