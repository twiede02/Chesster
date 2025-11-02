# Define the file name
# file_name = "rook_masks.txt"
file_name = "bishop_masks.txt"


def rook_blocker_mask(square):
    rank = square // 8
    file = square % 8
    mask = 0

    # North (up)
    for r in range(rank + 1, 7):  # stop before rank 7 (edge)
        mask |= 1 << (r * 8 + file)

    # South (down)
    for r in range(rank - 1, 0, -1):  # stop after rank 0 (edge)
        mask |= 1 << (r * 8 + file)

    # East (right)
    for f in range(file + 1, 7):  # stop before file 7 (edge)
        mask |= 1 << (rank * 8 + f)

    # West (left)
    for f in range(file - 1, 0, -1):  # stop after file 0 (edge)
        mask |= 1 << (rank * 8 + f)

    return mask


def bishop_blocker_mask(square):
    rank = square // 8
    file = square % 8
    mask = 0

    # NE (up-right)
    r, f = rank + 1, file + 1
    while r <= 6 and f <= 6:  # stop before edge
        mask |= 1 << (r * 8 + f)
        r += 1
        f += 1

    # NW (up-left)
    r, f = rank + 1, file - 1
    while r <= 6 and f >= 1:
        mask |= 1 << (r * 8 + f)
        r += 1
        f -= 1

    # SE (down-right)
    r, f = rank - 1, file + 1
    while r >= 1 and f <= 6:
        mask |= 1 << (r * 8 + f)
        r -= 1
        f += 1

    # SW (down-left)
    r, f = rank - 1, file - 1
    while r >= 1 and f >= 1:
        mask |= 1 << (r * 8 + f)
        r -= 1
        f -= 1

    return mask

with open(file_name, "w") as file:

    for i in range(64):
        # number = rook_blocker_mask(i)
        number = bishop_blocker_mask(i)
        file.write(f"0x{number:016X},\n")  # 16-digit uppercase hex with 0x prefix

print(f"64-bit integers written to {file_name}")

