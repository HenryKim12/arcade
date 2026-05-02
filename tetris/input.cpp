#include "input.h"

#include <csignal>
#include <cstdio>
#include <termios.h>
#include <unistd.h>

static struct termios original;
static bool rawModeActive = false;

static void handleSignal(int) {
    restoreTerminal();
    _exit(0);
}

void initTerminal() {
    tcgetattr(STDIN_FILENO, &original);

    struct termios raw = original;
    raw.c_iflag &= ~(IXON | ICRNL);
    raw.c_oflag &= ~(OPOST);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG);
    raw.c_cc[VMIN]  = 0;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    rawModeActive = true;

    signal(SIGINT,  handleSignal);
    signal(SIGTERM, handleSignal);

    printf("\033[?25l"); // hide cursor
    fflush(stdout);
}

void restoreTerminal() {
    if (!rawModeActive) return;
    tcsetattr(STDIN_FILENO, TCSANOW, &original);
    rawModeActive = false;
    printf("\033[?25h"); // show cursor
    fflush(stdout);
}

Key readKey() {
    char c;
    if (read(STDIN_FILENO, &c, 1) <= 0) return Key::None;

    if (c == ' ')              return Key::Space;
    if (c == 'p' || c == 'P') return Key::Pause;
    if (c == 'q' || c == 'Q') return Key::Quit;

    if (c == '\033') {
        char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) <= 0) return Key::None;
        if (read(STDIN_FILENO, &seq[1], 1) <= 0) return Key::None;
        if (seq[0] == '[') {
            if (seq[1] == 'A') return Key::Up;
            if (seq[1] == 'B') return Key::Down;
            if (seq[1] == 'C') return Key::Right;
            if (seq[1] == 'D') return Key::Left;
        }
    }

    return Key::None;
}
