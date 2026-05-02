#pragma once

enum class Key { None, Up, Down, Left, Right, Space, Pause, Quit };

void initTerminal();
void restoreTerminal();
Key readKey();
