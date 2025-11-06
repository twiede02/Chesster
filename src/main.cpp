#include "zobrist.h"
#include "uci.h"

int main() {
    initialize_zobrist();
    uciloop();
    return 0;
}
