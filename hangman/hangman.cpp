#include <cctype>
#include <iostream>
#include <set>
#include <tuple>
#include <vector>

void clearTerminal() { std::cout << "\033[2J\033[H"; }

bool isAlphabet(const char &c) { return std::isalpha(c); }

void endScreen(const int lives, const std::string word) {
  std::cout << std::endl;
  std::cout << "============================" << std::endl;
  if (lives == 0) {
    std::cout << "======== \033[1;31mYOU LOSE!\033[0m =========" << std::endl;
  } else {
    std::cout << "========= \033[1;32mYOU WIN!\033[0m =========" << std::endl;
  }
  std::cout << "============================" << std::endl;
  std::cout << std::endl;
  std::cout << "The answer was " << "`" << word << "`!"
            << " Thanks for playing!" << std::endl;
}

char toUpper(char &c) {
  if (c != '_') {
    c = std::toupper(c);
  }
  return c;
}

void printBoard(std::string &slots, const int lives, const bool prevGoodGuess) {
  std::cout << "========== HANGMAN ==========" << std::endl;
  if (prevGoodGuess) {
    std::cout << "\033[1;32mLives: \033[0m" << lives << std::endl;
  } else {
    std::cout << "\033[1;31mLives: \033[0m" << lives << std::endl;
  }
  std::cout << "Hangman: ";
  for (int i{}; i < slots.size(); i++) {
    if (i == slots.size() - 1) {
      std::cout << toUpper(slots[i]) << std::endl;
    } else {
      std::cout << toUpper(slots[i]) << " ";
    }
  }
}

int main() {
  int guessIdx = 0;

  std::cout << "Enter word: ";
  std::string word;
  while (true) {
    std::cin >> word;
    bool validWord = true;
    for (int i{}; i < word.size(); i++) {
      if (!isAlphabet(word[i])) {
        std::cout << "Please enter a word with only characters in the alphabet "
                     "(A-Z)! Try Again."
                  << std::endl;
        validWord = false;
        break;
      }
    }
    if (validWord) {
      break;
    }
  }
  std::transform(word.begin(), word.end(), word.begin(), ::toupper);
  clearTerminal();

  int lives = 6;
  std::string slots(word.size(), '_');
  std::set<std::tuple<int, char>> guessedChars;
  bool prevGoodGuess = true;
  while (lives > 0) {
    if (slots.find('_') == std::string::npos) {
      break;
    }
    printBoard(slots, lives, prevGoodGuess);

    std::cout << "Guessed characters: ";
    if (guessedChars.empty()) {
      std::cout << "None" << std::endl;
    } else {
      for (auto it = guessedChars.begin(); it != guessedChars.end(); it++) {
        char c = std::get<1>(*it);
        if (std::next(it) == guessedChars.end()) {
          std::cout << toUpper(c) << std::endl;
        } else {
          std::cout << toUpper(c) << ", ";
        }
      }
    }

    std::cout << "Choose a character to guess!" << std::endl;
    char guess;
    while (true) {
      std::cin >> guess;
      const bool alreadyGuessed =
          (guessedChars.find(std::make_tuple(guessIdx, guess)) !=
           guessedChars.end());
      const bool nonAlphabet = !isAlphabet(guess);
      const bool moreThanSingleChar =
          std::cin.peek() != '\n' && std::cin.peek() != EOF;

      if (!alreadyGuessed && !nonAlphabet && !moreThanSingleChar) {
        break;
      }

      if (alreadyGuessed) {
        std::cout << "You already guessed that character! Try again."
                  << std::endl;
      } else if (nonAlphabet) {
        std::cout
            << "Please enter a character in the alphabet (A-Z)! Try Again."
            << std::endl;
      } else {
        std::cout << "Please enter a single character! Try Again." << std::endl;
      }

      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    const std::tuple<int, char> guessTuple = std::make_tuple(guessIdx, guess);
    guessedChars.insert(guessTuple);
    guessIdx++;
    std::vector<int> goodGuessIndices;
    for (int i{}; i < word.size(); i++) {
      if (toUpper(guess) == word[i]) {
        goodGuessIndices.push_back(i);
      }
    }
    if (goodGuessIndices.empty()) {
      prevGoodGuess = false;
      lives--;
    } else {
      prevGoodGuess = true;
      for (int i{}; i < goodGuessIndices.size(); i++) {
        slots[goodGuessIndices[i]] = guess;
      }
    }
  }

  endScreen(lives, word);

  return 0;
}
