#include <cctype>
#include <iostream>

#include "game.h"

std::string ask(const std::string &prompt) {
    std::string line;
    std::cout << prompt;
    if (!std::getline(std::cin, line)) std::exit(0);
    return line;
}

int main() {
    Game game;
    int length = (int)game.word.size();
    std::cout << "Вгадай слово! Загадано слово з " << length << " літер, можна помилитися " << MAX_MISSES << " разів\n";
    while (!game.won() && !game.lost()) {
        std::cout << game.show() << "\n";
        std::string letter = ask("Літера: ");
        std::string position = ask("Позиція (0-" + std::to_string(length - 1) + "): ");
        int index = position.size() == 1 && std::isdigit(position[0]) ? position[0] - '0' : -1;
        if (letter.size() != 1 || !std::islower(letter[0]) || index < 0 || index >= length) {
            std::cout << "Введіть малу латинську літеру та позицію від 0 до " << length - 1 << "\n";
        } else if (game.guess(letter[0], index)) {
            std::cout << "Вгадали!\n";
        } else {
            std::cout << "Не вгадали. Залишилось спроб: " << MAX_MISSES - game.misses << "\n";
        }
    }
    std::cout << game.show() << "\n";
    std::cout << (game.won() ? "Перемога! Слово вгадано" : "Спроби вичерпано. Загадане слово: " + game.word) << "\n";
}
