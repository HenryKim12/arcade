#include "input.h"

#include <cstdio>

static const char* keyName(Key k) {
    switch (k) {
        case Key::Up:    return "Up";
        case Key::Down:  return "Down";
        case Key::Left:  return "Left";
        case Key::Right: return "Right";
        case Key::Space: return "Space";
        case Key::Pause: return "Pause";
        case Key::Quit:  return "Quit";
        default:         return "None";
    }
}

int main() {
    initTerminal();
    printf("\033[2J\033[H"); // clear screen
    printf("\033[1;1HPress arrow keys, space, p, q (q to quit)\n");
    fflush(stdout);

    while (true) {
        Key k = readKey();
        if (k != Key::None) {
            printf("\033[3;1HLast key: %-10s\n", keyName(k));
            fflush(stdout);
        }
        if (k == Key::Quit) break;
    }

    restoreTerminal();
    return 0;
}
