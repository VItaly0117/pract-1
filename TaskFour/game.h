#pragma once
#include <random>
#include <string>

const int MAX_MISSES = 5;
const int MAX_LENGTH = 7;
const char HIDDEN = '*';

const char WORDS[][MAX_LENGTH] = {
    {'t', 'h', 'i', 's'},      {'i', 's'},
    {'a'},                     {'w', 'o', 'r', 'd'},
    {'a', 'r', 'r', 'a', 'y'}, {'m', 'a', 't', 'r', 'i', 'x'},
    {'l', 'e', 't', 't', 'e', 'r'},
};

struct Game {
    std::string word, mask;
    int misses = 0;

    Game() {
        word = WORDS[std::random_device{}() % std::size(WORDS)];
        mask = std::string(word.size(), HIDDEN);
    }

    bool guess(char letter, int position) {
        if (word[position] == letter) {
            mask[position] = letter;
            return true;
        }
        misses++;
        return false;
    }

    bool won() const { return mask == word; }
    bool lost() const { return misses >= MAX_MISSES; }

    std::string show() const {
        std::string text = "{ ";
        for (size_t i = 0; i < mask.size(); i++) text += (i ? ", '" : "'") + std::string(1, mask[i]) + "'";
        return text + " }";
    }
};
